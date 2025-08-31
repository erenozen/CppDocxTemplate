#pragma once
#include <QString>
#include <pugixml.hpp>
#include "QtDocxTemplate/Variables.hpp"
#include "opc/Package.hpp"

namespace QtDocxTemplate { namespace engine {

// Variable replacement engine for text, image, bullet list, and table processing
struct Replacers {
    static void replaceText(pugi::xml_document &doc,
                            const QString &prefix,
                            const QString &suffix,
                            const ::QtDocxTemplate::Variables &vars);
    static void replaceImages(pugi::xml_document &doc, opc::Package &pkg,
                              const QString &prefix, const QString &suffix,
                              const ::QtDocxTemplate::Variables &vars);
    static void replaceBulletLists(pugi::xml_document &doc, opc::Package &pkg,
                                   const QString &prefix, const QString &suffix,
                                   const ::QtDocxTemplate::Variables &vars);
    static void replaceTables(pugi::xml_document &doc, opc::Package &pkg,
                              const QString &prefix, const QString &suffix,
                              const ::QtDocxTemplate::Variables &vars);
};

}} // namespace QtDocxTemplate::engine
