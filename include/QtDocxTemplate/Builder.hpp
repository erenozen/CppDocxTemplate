/** \file Builder.hpp
 *  Free helper functions to concisely create variable shared_ptr objects.
 *  These wrap raw keys with the current variable pattern if needed.
 */
#pragma once
#include "QtDocxTemplate/Export.hpp"
#include "QtDocxTemplate/VariablePattern.hpp"
#include "QtDocxTemplate/TextVariable.hpp"
#include "QtDocxTemplate/ImageVariable.hpp"
#include "QtDocxTemplate/BulletListVariable.hpp"
#include "QtDocxTemplate/TableVariable.hpp"
#include "QtDocxTemplate/Docx.hpp"
#include <memory>
#include <initializer_list>

namespace QtDocxTemplate {

/** Returns true if key already appears wrapped with pattern prefix+suffix. */
inline bool keyLooksWrapped(const QString &key, const VariablePattern &pat){
    return key.startsWith(pat.prefix) && key.endsWith(pat.suffix);
}

/** Ensure a key is wrapped with pattern delimiters. */
inline QString ensureWrapped(const QString &rawOrWrapped, const VariablePattern &pat){
    if(keyLooksWrapped(rawOrWrapped, pat)) return rawOrWrapped;
    return pat.prefix + rawOrWrapped + pat.suffix;
}

/** Create a TextVariable (wrapping key if necessary). */
QTDOCTXTEMPLATE_EXPORT std::shared_ptr<TextVariable> makeTextVar(const QString &keyOrName, const QString &value, const VariablePattern &pat = {});
/** Overload: derive pattern from a Docx instance. */
inline std::shared_ptr<TextVariable> makeTextVar(const Docx &d, const QString &keyOrName, const QString &value){ return makeTextVar(keyOrName, value, d.variablePattern()); }

/** Create an ImageVariable (wrapping key if necessary). */
QTDOCTXTEMPLATE_EXPORT std::shared_ptr<ImageVariable> makeImageVar(const QString &keyOrName, const QImage &img, int wPx, int hPx, const VariablePattern &pat = {});
inline std::shared_ptr<ImageVariable> makeImageVar(const Docx &d, const QString &keyOrName, const QImage &img, int wPx, int hPx){ return makeImageVar(keyOrName, img, wPx, hPx, d.variablePattern()); }

/** Create a BulletListVariable from item texts (key wrapped if necessary). */
QTDOCTXTEMPLATE_EXPORT std::shared_ptr<BulletListVariable> makeBulletListVar(const QString &keyOrName, const QStringList &items, const VariablePattern &pat = {});
inline std::shared_ptr<BulletListVariable> makeBulletListVar(const Docx &d, const QString &keyOrName, const QStringList &items){ return makeBulletListVar(keyOrName, items, d.variablePattern()); }

/** Convenience structure for table column specification. */
struct TableColumnSpec { QString keyOrName; QStringList values; };

/** Create a TableVariable from initializer_list of column specs. Each item list becomes a column; lengths may differ (shortest governs). */
QTDOCTXTEMPLATE_EXPORT std::shared_ptr<TableVariable> makeTableVar(std::initializer_list<TableColumnSpec> cols, const VariablePattern &pat = {});
inline std::shared_ptr<TableVariable> makeTableVar(const Docx &d, std::initializer_list<TableColumnSpec> cols){ return makeTableVar(cols, d.variablePattern()); }

} // namespace QtDocxTemplate
