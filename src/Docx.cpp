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
    if(!dataOpt) return {};
    xml::XmlPart part;
    if(!part.load(*dataOpt)) return {};
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
    // Load document.xml
    auto dataOpt = m_package->readPart("word/document.xml");
    if(!dataOpt) return; // nothing to do
    xml::XmlPart part;
    if(!part.load(*dataOpt)) return;
    // Perform replacements (text first, then images, bullet lists, tables)
    engine::Replacers::replaceText(part.doc(), m_pattern.prefix, m_pattern.suffix, variables);
    engine::Replacers::replaceImages(part.doc(), *m_package, m_pattern.prefix, m_pattern.suffix, variables);
    engine::Replacers::replaceBulletLists(part.doc(), *m_package, m_pattern.prefix, m_pattern.suffix, variables);
    engine::Replacers::replaceTables(part.doc(), *m_package, m_pattern.prefix, m_pattern.suffix, variables);
    // Save back
    QByteArray out = part.save();
    m_package->writePart("word/document.xml", out);
}

void Docx::save(const QString &outputPath) const {
    Q_UNUSED(outputPath);
    if(!ensureOpened()) return;
    if(m_package) {
        m_package->saveAs(outputPath);
    }
}

} // namespace QtDocxTemplate
