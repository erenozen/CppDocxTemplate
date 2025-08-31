/** \file TableVariable.hpp
 *  Table placeholder replicating rows based on column-wise variable lists.
 *  Redesigned for templ4docx parity: each column corresponds to a distinct
 *  placeholder token (e.g. ${deviceId}, ${cpm}, ...). A template table row
 *  contains one occurrence of each column's placeholder (typically one per cell).
 *  During expansion, the template row is cloned N times (N = column list length)
 *  and each placeholder token is replaced with the i-th element of its column.
 */
#pragma once
#include "QtDocxTemplate/Variable.hpp"
#include <vector>

namespace QtDocxTemplate {

/** Represents a table data set: each column is a vector of variables sharing a single placeholder key. */
class QTDOCTXTEMPLATE_EXPORT TableVariable : public Variable {
public:
    /** Construct with optional identifying key (not used for matching in templ4docx style). */
    explicit TableVariable(QString key = QString()) : Variable(std::move(key), VariableType::Table) {}

    /** Add a column. All variables within the column must be Text variables and share the same placeholder token. */
    void addColumn(const std::vector<VariablePtr> &column);

    /** Access raw column storage. */
    const std::vector<std::vector<VariablePtr>> & columns() const { return m_columns; }

    /** Placeholder token for each column (e.g. ${deviceId}). */
    const std::vector<QString> & placeholderKeys() const { return m_columnKeys; }

    /** Number of rows (length of first column) or 0 if empty. */
    size_t rowCount() const { return m_columns.empty() ? 0 : m_columns.front().size(); }

    /** Validate that all columns are of equal length; returns min length if mismatch. */
    size_t validatedRowCount(bool &lengthsMismatch) const;

private:
    std::vector<std::vector<VariablePtr>> m_columns; // column-wise data
    std::vector<QString> m_columnKeys;               // per-column placeholder token
};

} // namespace QtDocxTemplate
