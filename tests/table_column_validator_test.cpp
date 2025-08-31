#include "QtDocxTemplate/Docx.hpp"
#include "QtDocxTemplate/Variables.hpp"
#include "QtDocxTemplate/Builder.hpp"
#include "QtDocxTemplate/TableVariable.hpp"
#include "QtDocxTemplate/TextVariable.hpp"
#include "opc/Package.hpp"
#include <QTemporaryDir>
#include <QFile>
#include <cassert>
#include <iostream>

using namespace QtDocxTemplate; using QtDocxTemplate::opc::Package;

static QString writeMinimalDocx(const QString &path,const std::string &body){
    Package pkg; pkg.writePart("[Content_Types].xml", QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\"?><Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\"><Default Extension=\"xml\" ContentType=\"application/xml\"/><Override PartName=\"/word/document.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml\"/></Types>"));
    QByteArray docXml = QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><w:document xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"><w:body>") + QByteArray::fromStdString(body) + QByteArray("</w:body></w:document>");
    pkg.writePart("word/document.xml", docXml); bool ok=pkg.saveAs(path); assert(ok); return path; }

int main(){
    QTemporaryDir tmp; assert(tmp.isValid());
    // Template only includes two placeholders of three to simulate missing column token
    std::string body = "<w:p><w:r><w:t>${A}</w:t></w:r></w:p><w:p><w:r><w:t>${B}</w:t></w:r></w:p>"; // ${C} missing
    QString in = tmp.path()+"/col.docx"; writeMinimalDocx(in, body);
    Docx doc(in);
    auto table = std::make_shared<TableVariable>();
    // Build columns A,B,C with two rows each
    std::vector<VariablePtr> colA { std::make_shared<TextVariable>("${A}", "a1"), std::make_shared<TextVariable>("${A}", "a2") };
    std::vector<VariablePtr> colB { std::make_shared<TextVariable>("${B}", "b1"), std::make_shared<TextVariable>("${B}", "b2") };
    std::vector<VariablePtr> colC { std::make_shared<TextVariable>("${C}", "c1"), std::make_shared<TextVariable>("${C}", "c2") };
    table->addColumn(colA); table->addColumn(colB); table->addColumn(colC);
    Variables vars; vars.addTable(table);
    QStringList missing = doc.validateTableColumnPlaceholders(vars);
    assert(missing.size()==1 && missing.contains("${C}"));
    std::cout << "table_column_validator_test passed" << std::endl; return 0;
}
