// Test builder overloads with custom variable pattern
#include "QtDocxTemplate/Docx.hpp"
#include "QtDocxTemplate/Builder.hpp"
#include "QtDocxTemplate/Variables.hpp"
#include "opc/Package.hpp"
#include <QTemporaryDir>
#include <QFile>
#include <cassert>
#include <iostream>

using namespace QtDocxTemplate; using QtDocxTemplate::opc::Package;

static QByteArray minimalDocx(const QString &body){
    Package pkg; pkg.writePart("[Content_Types].xml", QByteArray(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" \
        "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">" \
        "<Default Extension=\"xml\" ContentType=\"application/xml\"/>" \
        "<Override PartName=\"/word/document.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml\"/>" \
        "</Types>"));
    QByteArray docXml = QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>" \
        "<w:document xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"><w:body>" + body.toUtf8() + "</w:body></w:document>");
    pkg.writePart("word/document.xml", docXml); QTemporaryDir dir; QString out = dir.path()+"/p.docx"; pkg.saveAs(out); QFile f(out); assert(f.open(QIODevice::ReadOnly)); return f.readAll();
}

int main(){
    // Use [[ ]] pattern and ensure builder auto-wraps when deriving from Docx
    QString para = "<w:p><w:r><w:t>[[NAME]]</w:t></w:r></w:p>"; QByteArray bytes = minimalDocx(para);
    QTemporaryDir dir; QString in = dir.path()+"/pat.docx"; QFile f(in); assert(f.open(QIODevice::WriteOnly)); f.write(bytes); f.close();
    Docx d(in); d.setVariablePattern({QString("[["), QString("]]")});
    Variables vars; vars.add(makeTextVar(d, "NAME", "Zed"));
    d.fillTemplate(vars); d.save(in+".o");
    Docx d2(in+".o"); auto txt = d2.readTextContent(); assert(txt.contains("Zed")); assert(!txt.contains("[[NAME]]"));
    std::cout << "builder_pattern_test passed" << std::endl; return 0;
}
