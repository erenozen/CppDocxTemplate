/** \file Docx.hpp
 *  Public façade for loading a DOCX template and performing variable replacement.
 *  Processing scope: main document part (word/document.xml) only. Unknown placeholders are left untouched.
 */
#pragma once
#include "QtDocxTemplate/Export.hpp"
#include "QtDocxTemplate/VariablePattern.hpp"
#include "QtDocxTemplate/Variables.hpp"
#include <QString>
#include <QStringList>
#include <memory>
#include <optional>

namespace QtDocxTemplate {

// Forward declarations & internal includes
namespace opc { class Package; }
namespace xml { class XmlPart; }

/** Main API entry. Load a template, configure pattern, replace variables, and save.
 *  Thread-safety: instances are not thread-safe. One instance per document.
 */
class QTDOCTXTEMPLATE_EXPORT Docx {
public:
    /** Error codes for last operation (non-fatal except OpenFailed / XmlParseFailed). */
    enum class ErrorCode {
        OpenFailed,
        DocumentPartMissing,
        XmlParseFailed,
        TableColumnLengthMismatch
    };
    /** Construct with path to an existing .docx template. No I/O until first operation. */
    explicit Docx(QString templatePath);
    ~Docx();

    /** Override variable pattern (default ${ .. }). */
    void setVariablePattern(const VariablePattern &pattern);
    /** Current variable pattern in effect. */
    const VariablePattern & variablePattern() const { return m_pattern; }
    /** Return paragraph-joined plain text of the main document (paragraphs separated by \n). */
    QString readTextContent() const; // paragraphs joined by '\n'
    /** Non-greedy scan for placeholders matching prefix+suffix. Spans across run boundaries. Deduplicated, order of first appearance. */
    QStringList findVariables() const;
    /** Perform in-place substitution for provided variables (text, image, bullet list, table). Unknown placeholders remain. */
    void fillTemplate(const Variables &variables);
    /** Validate that each TableVariable column placeholder appears somewhere in the template (main doc + headers/footers).
     *  Returns list of missing placeholder tokens (wrapped form). Emits qWarning for each missing token.
     */
    QStringList validateTableColumnPlaceholders(const Variables &variables) const;
    /** Write resulting package to disk (zip). */
    void save(const QString &outputPath) const;

    /** Last error code set during an operation; std::nullopt if none since construction or after successful clear. */
    std::optional<ErrorCode> lastError() const { return m_lastError; }
    /** Clear stored error. */
    void clearError() { m_lastError.reset(); }

private:
    QString m_templatePath;
    VariablePattern m_pattern;
    mutable std::shared_ptr<opc::Package> m_package; // OPC container (shared_ptr works with incomplete type)
    mutable bool m_openAttempted{false};
    mutable bool m_documentLoaded{false};
    bool ensureOpened() const; // lazy open helper
    bool ensureDocumentLoaded() const; // load word/document.xml once
    void setError(ErrorCode ec) const { m_lastError = ec; }

    // Build paragraph-joined text (implementation detail shared by readTextContent & findVariables)
    QString readFullTextCache() const;
    mutable std::optional<ErrorCode> m_lastError;
};

} // namespace QtDocxTemplate
