#include "QtDocxTemplate/Docx.hpp"
#include <QFile>
#include "opc/Package.hpp"
#include "xml/XmlPart.hpp"
#include "engine/Replacers.hpp"
#include "QtDocxTemplate/Variables.hpp"
#include <QRegularExpression>

namespace QtDocxTemplate {

bool Docx::ensureOpened() const {
    if(m_openAttempted) return m_package != nullptr;
    m_openAttempted = true;
    auto pkg = std::make_shared<opc::Package>();
    if(!pkg->open(m_templatePath)) {
    setError(ErrorCode::OpenFailed);
        return false;
    }
    m_package = std::move(pkg);
    return true;
}

bool Docx::ensureDocumentLoaded() const {
    if(!ensureOpened()) return false;
    if(m_documentLoaded) return true;
    // Lazy include of XmlPart header structure: we can't forward declare internal storage, so we store in a lambda-local static.
    // For now we parse on demand each call to readTextContent; simple implementation approach.
    return true;
}

QString Docx::readFullTextCache() const {
    if(!ensureOpened()) return {};
    // Load document.xml
    auto dataOpt = m_package->readPart("word/document.xml");
    if(!dataOpt) { setError(ErrorCode::DocumentPartMissing); return {}; }
    xml::XmlPart part;
    if(!part.load(*dataOpt)) { setError(ErrorCode::XmlParseFailed); return {}; }
    // Structural validation: require w:body
    if(part.selectAll("//w:body").empty()) { setError(ErrorCode::XmlParseFailed); return {}; }
    QStringList paragraphLines;
    auto paragraphs = part.selectAll("//w:p");
    for(const auto &p : paragraphs) {
        QString paraText;
        // Collect all descendant w:t in order
        pugi::xpath_query textQuery(".//w:t");
        auto ts = textQuery.evaluate_node_set(p);
        for(const auto &tn : ts) {
            pugi::xml_node node = tn.node();
            // Respect xml:space="preserve" (we just take text as-is)
            QString t = QString::fromUtf8(node.text().get());
            paraText += t;
        }
        paragraphLines << paraText;
    }
    return paragraphLines.join('\n');
}

Docx::Docx(QString templatePath)
    : m_templatePath(std::move(templatePath)) {}

Docx::~Docx() = default;

void Docx::setVariablePattern(const VariablePattern &pattern) {
    m_pattern = pattern;
}

QString Docx::readTextContent() const {
    return readFullTextCache();
}

QStringList Docx::findVariables() const {
    QString text = readFullTextCache();
    if(text.isEmpty()) return {};
    // Escape prefix/suffix for regex
    auto esc = [](const QString &s){ return QRegularExpression::escape(s); };
    QString pattern = esc(m_pattern.prefix) + "(.*?)" + esc(m_pattern.suffix); // non-greedy capture
    QRegularExpression re(pattern);
    QRegularExpressionMatchIterator it = re.globalMatch(text);
    QStringList found;
    QSet<QString> seen;
    while(it.hasNext()) {
        auto m = it.next();
        QString full = m.captured(0);
        if(!seen.contains(full)) {
            seen.insert(full);
            found << full;
        }
    }
    return found;
}

void Docx::fillTemplate(const Variables &variables) {
    if(!ensureOpened()) return;
    clearError();
    // Build list of document parts to process: main doc + headers + footers
    QStringList targets;
    targets << "word/document.xml";
    for(const auto &name : m_package->partNames()) {
        if(name.startsWith("word/header") && name.endsWith(".xml")) targets << name;
        else if(name.startsWith("word/footer") && name.endsWith(".xml")) targets << name;
    }
    targets.removeDuplicates();
    for(const auto &partName : targets) {
        auto dataOpt = m_package->readPart(partName);
        if(!dataOpt) continue;
        xml::XmlPart part;
    if(!part.load(*dataOpt)) { setError(ErrorCode::XmlParseFailed); continue; }
    if(part.selectAll("//w:body").empty()) { setError(ErrorCode::XmlParseFailed); continue; }
        engine::Replacers::replaceText(part.doc(), m_pattern.prefix, m_pattern.suffix, variables);
        engine::Replacers::replaceImages(part.doc(), *m_package, m_pattern.prefix, m_pattern.suffix, variables);
        engine::Replacers::replaceBulletLists(part.doc(), *m_package, m_pattern.prefix, m_pattern.suffix, variables);
    bool mismatch = engine::Replacers::replaceTables(part.doc(), *m_package, m_pattern.prefix, m_pattern.suffix, variables);
    if(mismatch && !m_lastError.has_value()) setError(ErrorCode::TableColumnLengthMismatch);
        QByteArray out = part.save();
        m_package->writePart(partName, out);
    }
}

void Docx::save(const QString &outputPath) const {
    Q_UNUSED(outputPath);
    if(!ensureOpened()) return;
    if(m_package) {
        m_package->saveAs(outputPath);
    }
}

QStringList Docx::validateTableColumnPlaceholders(const Variables &variables) const {
    QStringList missing;
    if(!ensureOpened()) return missing;
    // Collect all wrapped placeholders for table columns
    QSet<QString> required; QSet<QString> present;
    for(const auto &v : variables.all()) {
        if(v->type() == VariableType::Table) {
            auto *tv = static_cast<const TableVariable*>(v.get());
            for(const auto &k : tv->placeholderKeys()) {
                QString token = k;
                if(!token.startsWith(m_pattern.prefix) || !token.endsWith(m_pattern.suffix)) {
                    token = m_pattern.prefix + token + m_pattern.suffix;
                }
                required.insert(token);
            }
        }
    }
    if(required.isEmpty()) return missing;
    // Gather text from all relevant parts
    QStringList partNames; partNames << "word/document.xml";
    for(const auto &name : m_package->partNames()) {
        if(name.startsWith("word/header") && name.endsWith(".xml")) partNames << name;
        else if(name.startsWith("word/footer") && name.endsWith(".xml")) partNames << name;
    }
    partNames.removeDuplicates();
    for(const auto &pn : partNames) {
        auto dataOpt = m_package->readPart(pn); if(!dataOpt) continue;
        xml::XmlPart part; if(!part.load(*dataOpt)) continue;
        auto paragraphs = part.selectAll("//w:p");
        for(const auto &p : paragraphs) {
            QString paraText; pugi::xpath_query textQuery(".//w:t"); auto ts = textQuery.evaluate_node_set(p);
            for(const auto &tn : ts) { pugi::xml_node node = tn.node(); paraText += QString::fromUtf8(node.text().get()); }
            for(const auto &req : required) if(paraText.contains(req)) present.insert(req);
        }
    }
    for(const auto &req : required) if(!present.contains(req)) { missing << req; qWarning("Template missing table column placeholder: %s", req.toUtf8().constData()); }
    return missing;
}

} // namespace QtDocxTemplate
