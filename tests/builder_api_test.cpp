// Tests for free builder helper API
#include "QtDocxTemplate/Docx.hpp"
#include "QtDocxTemplate/Builder.hpp"
#include "QtDocxTemplate/Variables.hpp"
#include "opc/Package.hpp"
#include <QTemporaryDir>
#include <QFile>
#include <cassert>
#include <iostream>

using namespace QtDocxTemplate; using QtDocxTemplate::opc::Package;

static QByteArray minimalDocxWithBody(const QString &bodyXml){
    Package pkg; pkg.writePart("[Content_Types].xml", QByteArray(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" \
        "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">" \
        "<Default Extension=\"xml\" ContentType=\"application/xml\"/>" \
        "<Override PartName=\"/word/document.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml\"/>" \
        "</Types>"));
    QByteArray docXml = QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>" \
        "<w:document xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"><w:body>" + bodyXml.toUtf8() + "</w:body></w:document>");
    pkg.writePart("word/document.xml", docXml); QTemporaryDir dir; QString outPath = dir.path()+"/doc.docx"; pkg.saveAs(outPath); QFile f(outPath); assert(f.open(QIODevice::ReadOnly)); return f.readAll();
}

int main(){
    // Text builder auto-wrap
    {
        QString para = "<w:p><w:r><w:t>${NAME}</w:t></w:r></w:p>";
        QByteArray bytes = minimalDocxWithBody(para);
        QTemporaryDir dir; QString in = dir.path()+"/b.docx"; QFile f(in); assert(f.open(QIODevice::WriteOnly)); f.write(bytes); f.close();
        Docx d(in); Variables vars; vars.add(makeTextVar("NAME", "Alice")); d.fillTemplate(vars); d.save(in+".o");
        Docx d2(in+".o"); auto text = d2.readTextContent(); assert(text.contains("Alice")); assert(!text.contains("${NAME}"));
    }
    // Table builder with raw names
    {
        QString body = "<w:tbl><w:tr><w:tc><w:p><w:r><w:t>${name}</w:t></w:r></w:p></w:tc><w:tc><w:p><w:r><w:t>${age}</w:t></w:r></w:p></w:tc></w:tr></w:tbl>";
        QByteArray bytes = minimalDocxWithBody(body);
        QTemporaryDir dir; QString in = dir.path()+"/t.docx"; QFile f(in); assert(f.open(QIODevice::WriteOnly)); f.write(bytes); f.close();
        Docx d(in); Variables vars; vars.add(makeTableVar({ {"name", {"Alice","Bob"}}, {"age", {"30","25"}} })); d.fillTemplate(vars); d.save(in+".o");
        Package pkg; assert(pkg.open(in+".o")); auto xml = pkg.readPart("word/document.xml"); QString doc = *xml; assert(doc.contains("Alice")); assert(doc.contains("Bob")); assert(!doc.contains("${name}"));
    }
    // Bullet list builder
    {
        QString body = "<w:p><w:r><w:t>${items}</w:t></w:r></w:p>";
        QByteArray bytes = minimalDocxWithBody(body);
        QTemporaryDir dir; QString in = dir.path()+"/l.docx"; QFile f(in); assert(f.open(QIODevice::WriteOnly)); f.write(bytes); f.close();
        Docx d(in); Variables vars; vars.add(makeBulletListVar("items", {"First","Second"})); d.fillTemplate(vars); d.save(in+".o");
        Docx d2(in+".o"); QString text = d2.readTextContent(); assert(text.contains("First")); assert(text.contains("Second"));
    }
    std::cout << "builder_api_test passed" << std::endl; return 0;
}
