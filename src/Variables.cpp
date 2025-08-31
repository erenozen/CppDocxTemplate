#include "QtDocxTemplate/Variables.hpp"
#include "QtDocxTemplate/TextVariable.hpp"
#include "QtDocxTemplate/ImageVariable.hpp"
#include "QtDocxTemplate/BulletListVariable.hpp"
#include <QImage>

using namespace QtDocxTemplate;

namespace QtDocxTemplate {

void Variables::add(const VariablePtr &v) { m_vars.push_back(v); }

VariablePtr Variables::addText(const QString &key, const QString &value) {
    auto v = std::make_shared<TextVariable>(key, value);
    add(v); return v;
}

VariablePtr Variables::addImage(const QString &key, const QImage &image, int wPx, int hPx) {
    auto v = std::make_shared<ImageVariable>(key, image, wPx, hPx);
    add(v); return v;
}

VariablePtr Variables::addBulletList(const QString &key, const QStringList &items) {
    auto bl = std::make_shared<BulletListVariable>(key);
    for(const auto &t : items) bl->addItem(std::make_shared<TextVariable>(QStringLiteral("ignored"), t));
    add(bl); return bl;
}

} // namespace QtDocxTemplate
