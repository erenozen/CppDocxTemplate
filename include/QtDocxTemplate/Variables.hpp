/** \file Variables.hpp
 *  Container of variable objects supplied to Docx::fillTemplate.
 */
#pragma once
#include "QtDocxTemplate/Variable.hpp"
#include <vector>
#include <QImage>
#include "QtDocxTemplate/TableVariable.hpp"

namespace QtDocxTemplate {

/** Simple aggregate of shared_ptr<Variable>. Order preserved. */
class QTDOCTXTEMPLATE_EXPORT Variables {
public:
    /** Append a variable (no deduplication performed). */
    void add(const VariablePtr &v);
    /** Convenience: create and add a TextVariable. Returns added variable. */
    VariablePtr addText(const QString &key, const QString &value);
    /** Convenience: create and add an ImageVariable. */
    VariablePtr addImage(const QString &key, const QImage &image, int wPx, int hPx);
    /** Convenience: create and add a BulletListVariable with given item texts (each becomes a TextVariable). */
    VariablePtr addBulletList(const QString &key, const QStringList &items);
    /** Convenience: add an existing TableVariable (shared_ptr). */
    void addTable(const std::shared_ptr<class TableVariable> &tableVar) { add(std::static_pointer_cast<Variable>(tableVar)); }
    /** Access underlying ordered collection. */
    const std::vector<VariablePtr> & all() const { return m_vars; }
private:
    std::vector<VariablePtr> m_vars;
};

} // namespace QtDocxTemplate
