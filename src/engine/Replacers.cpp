#include "engine/Replacers.hpp"
#include "engine/RunModel.hpp"
#include "QtDocxTemplate/Variables.hpp"
#include "QtDocxTemplate/TextVariable.hpp"
#include "QtDocxTemplate/ImageVariable.hpp"
#include "QtDocxTemplate/BulletListVariable.hpp"
#include "QtDocxTemplate/TableVariable.hpp"
#include "util/Emu.hpp"
#include "opc/Package.hpp"
#include <QRegularExpression>
#include <unordered_map>
#include <QBuffer>
#include <QFileInfo>
#include <sstream>
#include <QDebug>

using QtDocxTemplate::opc::Package;
using namespace QtDocxTemplate::util;

namespace QtDocxTemplate { 

// Implementation for TableVariable new methods (kept in this TU for compactness)
size_t TableVariable::validatedRowCount(bool &lengthsMismatch) const {
	lengthsMismatch = false;
	if(m_columns.empty()) return 0;
	size_t expected = m_columns.front().size();
	for(const auto &c : m_columns) {
		if(c.size() != expected) { lengthsMismatch = true; expected = std::min(expected, c.size()); }
	}
	return expected;
}

void TableVariable::addColumn(const std::vector<VariablePtr> &column) {
	if(column.empty()) return; // ignore empty column silently
	QString key = column.front()->key();
	bool consistent = true;
	for(const auto &v : column) {
		if(v->key() != key) { consistent = false; break; }
	}
	if(!consistent) {
		qWarning() << "TableVariable: column skipped due to mixed placeholder keys";
		return;
	}
	m_columns.push_back(column);
	m_columnKeys.push_back(key);
}

namespace engine {

namespace { QString wrapKey(const QString &k,const QString &pre,const QString &suf){ return pre + k + suf; } }

void Replacers::replaceText(pugi::xml_document &doc,
							const QString &prefix,
							const QString &suffix,
							const ::QtDocxTemplate::Variables &vars) {
	// Build lookup of wrapped placeholders => replacement
	std::unordered_map<QString, QString> map; map.reserve(vars.all().size()*2);
	for(const auto &vp : vars.all()) {
		if(vp->type() != VariableType::Text) continue;
		auto *tv = static_cast<TextVariable*>(vp.get());
		QString key = vp->key();
		if(key.startsWith(prefix) && key.endsWith(suffix)) {
			// Stored key already wrapped; accept both wrapped literal and inner form
			QString inner = key.mid(prefix.size(), key.size()-prefix.size()-suffix.size());
			map[key] = tv->value();
			map[wrapKey(inner,prefix,suffix)] = tv->value();
		} else {
			map[wrapKey(key,prefix,suffix)] = tv->value();
		}
	}
	if(map.empty()) return;

	pugi::xpath_query pq("//w:p");
	auto pnodes = pq.evaluate_node_set(doc);
	for(auto &n : pnodes) {
		pugi::xml_node p = n.node();
		RunModel rm; rm.build(p);
		QString paraText = rm.text();
		if(paraText.isEmpty()) continue;
		QRegularExpression re(QRegularExpression::escape(prefix)+"(.*?)"+QRegularExpression::escape(suffix));
		auto it = re.globalMatch(paraText);
		struct Match { int s; int e; QString token; };
		std::vector<Match> matches;
		while(it.hasNext()) {
			auto m = it.next();
			QString token = m.captured(0);
			if(!map.count(token)) continue; // skip unknown
			matches.push_back({(int)m.capturedStart(0), (int)m.capturedEnd(0), token});
		}
		if(matches.empty()) continue;
		std::sort(matches.begin(), matches.end(), [](const Match &a,const Match &b){ return a.s > b.s; });
		for(const auto &mm : matches) {
			QString replacement = map[mm.token];
			rm.replaceRange(mm.s, mm.e, [&](pugi::xml_node w_p, pugi::xml_node styleR){
				auto newRun = RunModel::makeTextRun(w_p, styleR, replacement, true);
				return std::vector<pugi::xml_node>{ newRun };
			});
			rm.build(p); // rebuild after each replacement
		}
	}
}

// Helper: ensure document relationships part loaded/created
static pugi::xml_document loadOrCreateDocRels(Package &pkg) {
	pugi::xml_document rels;
	auto data = pkg.readPart("word/_rels/document.xml.rels");
	if(data) {
		rels.load_buffer(data->constData(), data->size());
	}
	if(!rels.child("Relationships")) {
		auto root = rels.append_child("Relationships");
		root.append_attribute("xmlns") = "http://schemas.openxmlformats.org/package/2006/relationships";
	}
	return rels;
}

static QString nextImageRelId(const pugi::xml_document &rels) {
	int maxId = 0;
	for(auto r : rels.child("Relationships").children("Relationship")) {
		QString id = r.attribute("Id").value();
		if(id.startsWith("rId")) {
			bool ok=false; int num = id.mid(3).toInt(&ok); if(ok && num>maxId) maxId=num; }
	}
	return QString("rId%1").arg(maxId+1);
}

static pugi::xml_node buildDrawingRun(pugi::xml_node w_p, pugi::xml_node styleR, const QString &rId, int wPx, int hPx) {
	auto cx = pixelsToEmu(wPx); auto cy = pixelsToEmu(hPx);
	pugi::xml_node r = RunModel::makeTextRun(w_p, styleR, QString(), true); // create run with style; we'll replace w:t
	// remove w:t
	for(auto t = r.child("w:t"); t; ) { auto nxt = t.next_sibling(); r.remove_child(t); t=nxt; }
	pugi::xml_node drawing = r.append_child("w:drawing");
	pugi::xml_node inlineNode = drawing.append_child("wp:inline");
	inlineNode.append_attribute("xmlns:wp") = "http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing";
	inlineNode.append_attribute("xmlns:a") = "http://schemas.openxmlformats.org/drawingml/2006/main";
	inlineNode.append_attribute("xmlns:pic") = "http://schemas.openxmlformats.org/drawingml/2006/picture";
	inlineNode.append_attribute("xmlns:r") = "http://schemas.openxmlformats.org/officeDocument/2006/relationships";
	auto extent = inlineNode.append_child("wp:extent"); extent.append_attribute("cx") = std::to_string(cx).c_str(); extent.append_attribute("cy") = std::to_string(cy).c_str();
	auto graphic = inlineNode.append_child("a:graphic");
	auto gData = graphic.append_child("a:graphicData"); gData.append_attribute("uri") = "http://schemas.openxmlformats.org/drawingml/2006/picture";
	auto pic = gData.append_child("pic:pic");
	auto blipFill = pic.append_child("pic:blipFill");
	auto blip = blipFill.append_child("a:blip"); blip.append_attribute("r:embed") = rId.toUtf8().constData();
	blipFill.append_child("a:stretch").append_child("a:fillRect");
	auto spPr = pic.append_child("pic:spPr");
	auto xfrm = spPr.append_child("a:xfrm"); xfrm.append_child("a:off").append_attribute("x") = "0"; xfrm.append_child("a:off").append_attribute("y") = "0";
	auto prstGeom = spPr.append_child("a:prstGeom"); prstGeom.append_attribute("prst") = "rect"; prstGeom.append_child("a:avLst");
	return r;
}

void Replacers::replaceImages(pugi::xml_document &doc, Package &pkg,
							  const QString &prefix, const QString &suffix,
							  const ::QtDocxTemplate::Variables &vars) {
	// Build map of image variables
	struct Img { QImage img; int w; int h; QString key; };
	std::unordered_map<QString, Img> imap;
	for(const auto &v : vars.all()) if(v->type()==VariableType::Image) {
		auto *iv = static_cast<ImageVariable*>(v.get());
		QString k = v->key(); imap[k] = {iv->image(), iv->widthPx(), iv->heightPx(), k};
	}
	if(imap.empty()) return;
	pugi::xpath_query pq("//w:p");
	auto pnodes = pq.evaluate_node_set(doc);
	for(auto &nn : pnodes) {
		pugi::xml_node p = nn.node();
		RunModel rm; rm.build(p); QString paraText = rm.text(); if(paraText.isEmpty()) continue;
		QRegularExpression re(QRegularExpression::escape(prefix)+"(.*?)"+QRegularExpression::escape(suffix));
		auto it = re.globalMatch(paraText);
		struct M { int s; int e; QString tok; };
		std::vector<M> matches; while(it.hasNext()){ auto m=it.next(); QString token=m.captured(0); if(!imap.count(token)) continue; matches.push_back({m.capturedStart(0), m.capturedEnd(0), token}); }
		if(matches.empty()) continue; std::sort(matches.begin(), matches.end(), [](auto &a, auto &b){ return a.s > b.s; });
		for(auto &mm : matches) {
			auto info = imap[mm.tok];
			// Add media part
			QByteArray png; QBuffer buf(&png); buf.open(QIODevice::WriteOnly); info.img.save(&buf, "PNG");
			QString mediaPath = pkg.addMedia(png, "png");
			// Update rels
			auto rels = loadOrCreateDocRels(pkg);
			QString rId = nextImageRelId(rels);
			auto relRoot = rels.child("Relationships");
			auto rel = relRoot.append_child("Relationship");
			rel.append_attribute("Id") = rId.toUtf8().constData();
			rel.append_attribute("Type") = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/image";
			rel.append_attribute("Target") = QString("media/%1").arg(QFileInfo(mediaPath).fileName()).toUtf8().constData();
			// Persist rels
			std::stringstream ss; rels.save(ss, "  "); QByteArray relBytes(ss.str().c_str(), ss.str().size());
			pkg.writePart("word/_rels/document.xml.rels", relBytes);
			rm.replaceRangeStructural(mm.s, mm.e, [&](pugi::xml_node w_p, pugi::xml_node styleR){ auto r = buildDrawingRun(w_p, styleR, rId, info.w, info.h); return std::vector<pugi::xml_node>{ r }; });
			rm.build(p);
		}
	}
}

void Replacers::replaceBulletLists(pugi::xml_document &doc, const QString &prefix, const QString &suffix, const ::QtDocxTemplate::Variables &vars) {
	// Map bullet list variables
	std::unordered_map<QString, const BulletListVariable*> bmap;
	for(const auto &v : vars.all()) if(v->type()==VariableType::BulletList) bmap[v->key()] = static_cast<const BulletListVariable*>(v.get());
	if(bmap.empty()) return;
	pugi::xpath_query pq("//w:p");
	auto pnodes = pq.evaluate_node_set(doc);
	for(auto &nn : pnodes) {
		pugi::xml_node p = nn.node();
		RunModel rm; rm.build(p); QString paraText = rm.text(); if(paraText.isEmpty()) continue;
		QRegularExpression re(QRegularExpression::escape(prefix)+"(.*?)"+QRegularExpression::escape(suffix));
		auto it = re.globalMatch(paraText); struct M { int s; int e; QString tok; }; std::vector<M> matches; while(it.hasNext()){ auto m=it.next(); QString token=m.captured(0); if(!bmap.count(token)) continue; matches.push_back({m.capturedStart(0), m.capturedEnd(0), token}); }
		if(matches.empty()) continue; // only first bullet placeholder processed per paragraph for simplicity
		// Assume one placeholder per bullet paragraph template
		auto mm = matches.front(); auto bl = bmap[mm.tok];
		// Clone paragraph N times before original
		std::vector<pugi::xml_node> newParas;
		for(const auto &itemVar : bl->items()) {
			pugi::xml_node clone = p.parent().insert_copy_before(p, p);
			// Remove all existing runs in clone
			std::vector<pugi::xml_node> runsToRemove; for(pugi::xml_node r = clone.child("w:r"); r; r = r.next_sibling("w:r")) runsToRemove.push_back(r);
			for(auto r : runsToRemove) clone.remove_child(r);
			if(itemVar->type()==VariableType::Text) {
				auto *tv = static_cast<const TextVariable*>(itemVar.get());
				RunModel::makeTextRun(clone, pugi::xml_node(), tv->value(), true);
			}
			newParas.push_back(clone);
		}
		// Remove original template paragraph
		p.parent().remove_child(p);
	}
}

void Replacers::replaceTables(pugi::xml_document &doc, Package &pkg, const QString &prefix, const QString &suffix, const ::QtDocxTemplate::Variables &vars) {
	Q_UNUSED(pkg);
	// Collect all TableVariables
	std::vector<const TableVariable*> tables;
	tables.reserve(vars.all().size());
	for(const auto &v : vars.all()) if(v->type()==VariableType::Table) tables.push_back(static_cast<const TableVariable*>(v.get()));
	if(tables.empty()) return;

	// Build placeholder -> (tableIdx, columnIdx) map
	struct ColRef { size_t tableIdx; size_t colIdx; };
	std::unordered_map<QString, ColRef> placeholderMap;
	for(size_t ti=0; ti<tables.size(); ++ti) {
		const auto *tv = tables[ti];
		for(size_t ci=0; ci<tv->placeholderKeys().size(); ++ci) {
			placeholderMap[tv->placeholderKeys()[ci]] = {ti, ci};
		}
	}
	if(placeholderMap.empty()) return;

	// Iterate tables in document
	pugi::xpath_query tq("//w:tbl"); auto tblNodes = tq.evaluate_node_set(doc);
	for(auto &tn : tblNodes) {
		pugi::xml_node tblNode = tn.node();
		bool expanded = false;
		// Examine each row to find a template row (containing one or more known placeholders)
		for(pugi::xml_node tr = tblNode.child("w:tr"); tr; tr = tr.next_sibling("w:tr")) {
			// Scan row cells for placeholders
			struct CellToken { pugi::xml_node cell; pugi::xml_node para; QString placeholder; };
			std::vector<CellToken> cellTokens;
			for(pugi::xml_node tc = tr.child("w:tc"); tc; tc = tc.next_sibling("w:tc")) {
				for(pugi::xml_node p = tc.child("w:p"); p; p = p.next_sibling("w:p")) {
					RunModel rm; rm.build(p); QString txt = rm.text(); if(txt.isEmpty()) continue;
					QRegularExpression re(QRegularExpression::escape(prefix)+"(.*?)"+QRegularExpression::escape(suffix));
					auto it = re.globalMatch(txt);
					while(it.hasNext()) {
						auto m = it.next(); QString token = m.captured(0);
						if(placeholderMap.count(token)) {
							cellTokens.push_back({tc, p, token});
							break; // one token per cell is typical
						}
					}
				}
			}
			if(cellTokens.empty()) continue; // not a template row

			// Determine which TableVariable(s) are represented by collected placeholders (choose first fully matched table)
			const TableVariable *matched = nullptr; size_t matchedIdx = 0;
			for(size_t ti=0; ti<tables.size() && !matched; ++ti) {
				const auto *tv = tables[ti];
				// Count how many of its column keys appear in this row
				size_t hits = 0; for(const auto &k : tv->placeholderKeys()) {
					for(const auto &ct : cellTokens) if(ct.placeholder == k) { ++hits; break; }
				}
				if(hits == tv->placeholderKeys().size() && hits>0) { matched = tv; matchedIdx = ti; }
			}
			if(!matched) continue; // row doesn't fully represent a declared table variable

			bool lenMismatch=false; size_t rowCount = matched->validatedRowCount(lenMismatch);
			if(rowCount==0) { tblNode.remove_child(tr); expanded=true; break; }
			if(lenMismatch) qWarning("TableVariable: column length mismatch; truncating to minimum length %zu", rowCount);

			// Map placeholder -> columnIdx for the matched table
			std::unordered_map<QString, size_t> colIndexByToken;
			for(size_t ci=0; ci<matched->placeholderKeys().size(); ++ci) {
				colIndexByToken[matched->placeholderKeys()[ci]] = ci;
			}

			// Clone template row 'rowCount' times inserting AFTER original
			pugi::xml_node insertionPoint = tr;
			for(size_t r=0; r<rowCount; ++r) {
				pugi::xml_node newRow = tblNode.insert_copy_after(tr, insertionPoint);
				insertionPoint = newRow;
				// For each cell with a placeholder
				size_t cellIdx = 0;
				for(pugi::xml_node tc = newRow.child("w:tc"); tc; tc = tc.next_sibling("w:tc"), ++cellIdx) {
					// Find placeholder token in first paragraph
					for(pugi::xml_node p = tc.child("w:p"); p; p = p.next_sibling("w:p")) {
						RunModel rm; rm.build(p); QString txt = rm.text(); if(txt.isEmpty()) continue;
						for(const auto &kv : colIndexByToken) {
							const QString &token = kv.first; size_t colIdx = kv.second;
							int pos = txt.indexOf(token); if(pos<0) continue;
							int endPos = pos + token.size();
							// Retrieve variable for this row/column
							const auto &col = matched->columns()[colIdx]; if(r >= col.size()) break;
							auto cellVar = col[r];
							if(cellVar->type()==VariableType::Text) {
								auto *tv = static_cast<TextVariable*>(cellVar.get());
								rm.replaceRange(pos, endPos, [&](pugi::xml_node w_p, pugi::xml_node styleR){ auto run = RunModel::makeTextRun(w_p, styleR, tv->value(), true); return std::vector<pugi::xml_node>{ run }; });
								rm.build(p);
							} else if(cellVar->type()==VariableType::Image) {
								// TODO: Implement image insertion inside table cells (future step)
							}
							break; // one placeholder per cell
						}
					}
				}
			}
			// Remove original template row
			tblNode.remove_child(tr);
			expanded = true;
			break; // only one template row consumed per declared TableVariable per table
		}
		if(!expanded) {
			// Provide diagnostics if table variable placeholders were expected but not found
			// (Optional – suppressed for brevity)
		}
	}
}

}} // namespace QtDocxTemplate::engine
