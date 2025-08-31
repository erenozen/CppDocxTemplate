#include "QtDocxTemplate/Builder.hpp"
#include <utility>

namespace QtDocxTemplate {

std::shared_ptr<TextVariable> makeTextVar(const QString &keyOrName, const QString &value, const VariablePattern &pat){
    return std::make_shared<TextVariable>(ensureWrapped(keyOrName, pat), value);
}

std::shared_ptr<ImageVariable> makeImageVar(const QString &keyOrName, const QImage &img, int wPx, int hPx, const VariablePattern &pat){
    return std::make_shared<ImageVariable>(ensureWrapped(keyOrName, pat), img, wPx, hPx);
}

std::shared_ptr<BulletListVariable> makeBulletListVar(const QString &keyOrName, const QStringList &items, const VariablePattern &pat){
    auto bl = std::make_shared<BulletListVariable>(ensureWrapped(keyOrName, pat));
    for(const auto &t : items){
        bl->addItem(std::make_shared<TextVariable>(QStringLiteral("ignored"), t));
    }
    return bl;
}

std::shared_ptr<TableVariable> makeTableVar(std::initializer_list<TableColumnSpec> cols, const VariablePattern &pat){
    auto tv = std::make_shared<TableVariable>();
    for(const auto &c : cols){
        std::vector<VariablePtr> col; col.reserve(c.values.size());
        QString wrapped = ensureWrapped(c.keyOrName, pat);
        for(const auto &val : c.values){
            col.push_back(std::make_shared<TextVariable>(wrapped, val));
        }
        tv->addColumn(col);
    }
    return tv;
}

std::shared_ptr<TableVariable> makeTableVarFromRows(const QStringList &orderedKeys,
                                                    const std::vector<QMap<QString, QString>> &rows,
                                                    const VariablePattern &pat){
    auto tv = std::make_shared<TableVariable>();
    if(orderedKeys.isEmpty() || rows.empty()) return tv;
    // Initialize columns
    std::vector<std::vector<VariablePtr>> cols; cols.resize(orderedKeys.size());
    for(const auto &row : rows){
        for(int i=0;i<orderedKeys.size();++i){
            QString key = orderedKeys[i];
            QString wrapped = ensureWrapped(key, pat);
            QString value = row.value(key, QString());
            cols[i].push_back(std::make_shared<TextVariable>(wrapped, value));
        }
    }
    for(auto &c : cols) tv->addColumn(c);
    return tv;
}

} // namespace QtDocxTemplate
