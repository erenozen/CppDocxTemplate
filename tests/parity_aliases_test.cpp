// Verifies templ4docx parity alias methods and makeTableVarFromRows helper.
#include "QtDocxTemplate/Docx.hpp"
#include "QtDocxTemplate/Variables.hpp"
#include "QtDocxTemplate/Builder.hpp"
#include "opc/Package.hpp"
#include <QTemporaryDir>
#include <QFile>
#include <cassert>
#include <iostream>

using namespace QtDocxTemplate; using QtDocxTemplate::opc::Package;

static QByteArray docWithTableRow(const QStringList &placeholders){
    QString row;
    for(const auto &ph : placeholders){
        row += QString("<w:tc><w:p><w:r><w:t>%1</w:t></w:r></w:p></w:tc>").arg(ph);
    }
    QString body = QString("<w:tbl><w:tr>%1</w:tr></w:tbl>").arg(row);
    Package pkg; pkg.writePart("[Content_Types].xml", QByteArray(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">"
        "<Default Extension=\"xml\" ContentType=\"application/xml\"/>"
        "<Override PartName=\"/word/document.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml\"/>"
        "</Types>"));
    QByteArray docXml = QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
        "<w:document xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"><w:body>" + body.toUtf8() + "</w:body></w:document>");
    pkg.writePart("word/document.xml", docXml);
    QTemporaryDir dir; QString out = dir.path()+"/d.docx"; pkg.saveAs(out); QFile f(out); assert(f.open(QIODevice::ReadOnly)); return f.readAll();
}

int main(){
    // Build a minimal doc with five placeholders
    QStringList placeholders = {"${plate}","${company}","${speed}","${maxCps}","${when}"};
    QByteArray bytes = docWithTableRow(placeholders);
    QTemporaryDir dir; QString in = dir.path()+"/p.docx"; QFile f(in); assert(f.open(QIODevice::WriteOnly)); f.write(bytes); f.close();
    Docx d(in);
    Variables vars; vars.addTextVariable("${title}", "Parity Test");
    // Use makeTableVarFromRows
    std::vector<QMap<QString, QString>> rows;
    QMap<QString, QString> r1; r1["plate"]="06ABC123"; r1["company"]="Alpha"; r1["speed"]="3.40"; r1["maxCps"]="0"; r1["when"]="02-01-2024 08:08:00"; rows.push_back(r1);
    QMap<QString, QString> r2; r2["plate"]="06XYZ789"; r2["company"]="Beta";  r2["speed"]="5.10"; r2["maxCps"]="0"; r2["when"]="02-01-2024 08:09:00"; rows.push_back(r2);
    auto table = makeTableVarFromRows({"plate","company","speed","maxCps","when"}, rows);
    vars.addTableVariable(table);
    d.fillTemplate(vars); d.save(in+".o");
    Package pkg; assert(pkg.open(in+".o")); auto xml = pkg.readPart("word/document.xml"); QString doc = *xml;
    assert(!doc.contains("${plate}"));
    assert(doc.contains("06ABC123"));
    assert(doc.contains("06XYZ789"));
    std::cout << "parity_aliases_test passed" << std::endl; return 0;
}
