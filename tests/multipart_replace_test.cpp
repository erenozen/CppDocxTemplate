#include "QtDocxTemplate/Docx.hpp"
#include "QtDocxTemplate/Variables.hpp"
#include "QtDocxTemplate/TextVariable.hpp"
#include "opc/Package.hpp"
#include <QTemporaryDir>
#include <cassert>
#include <iostream>

using namespace QtDocxTemplate; using QtDocxTemplate::opc::Package;

static std::string contentTypesWithHeaderFooter() {return R"(<?xml version="1.0" encoding="UTF-8"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
  <Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
  <Default Extension="xml"  ContentType="application/xml"/>
  <Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
  <Override PartName="/word/header1.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.header+xml"/>
  <Override PartName="/word/footer1.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.footer+xml"/>
</Types>)"; }
static std::string packageRelsWithDoc() {return R"(<?xml version="1.0" encoding="UTF-8"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
  <Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
</Relationships>)"; }
static std::string docXmlRelatingHF() {return R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
<w:body>
<w:p><w:r><w:t>${bodyVar}</w:t></w:r></w:p>
</w:body>
</w:document>)"; }
static std::string headerXml() {return R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:hdr xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"><w:p><w:r><w:t>${headerVar}</w:t></w:r></w:p></w:hdr>)"; }
static std::string footerXml() {return R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:ftr xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"><w:p><w:r><w:t>${footerVar}</w:t></w:r></w:p></w:ftr>)"; }

static QString writeDocx(const QString &path) {
    Package pkg; pkg.writePart("[Content_Types].xml", QByteArray::fromStdString(contentTypesWithHeaderFooter()));
    pkg.writePart("_rels/.rels", QByteArray::fromStdString(packageRelsWithDoc()));
    pkg.writePart("word/document.xml", QByteArray::fromStdString(docXmlRelatingHF()));
    pkg.writePart("word/header1.xml", QByteArray::fromStdString(headerXml()));
    pkg.writePart("word/footer1.xml", QByteArray::fromStdString(footerXml()));
    bool ok = pkg.saveAs(path); assert(ok); return path;
}

int main(){
    QTemporaryDir tmp; assert(tmp.isValid());
    QString in = tmp.path()+"/mp.docx"; writeDocx(in);
    Variables vars; vars.addText("${bodyVar}", "BODY"); vars.addText("${headerVar}", "HEADER"); vars.addText("${footerVar}", "FOOTER");
    Docx d(in); d.fillTemplate(vars); QString out = in+".out.docx"; d.save(out);
    Package pkg; assert(pkg.open(out));
    auto body = pkg.readPart("word/document.xml"); assert(body.has_value()); assert(QString(*body).contains("BODY"));
    auto hdr = pkg.readPart("word/header1.xml"); assert(hdr.has_value()); assert(QString(*hdr).contains("HEADER"));
    auto ftr = pkg.readPart("word/footer1.xml"); assert(ftr.has_value()); assert(QString(*ftr).contains("FOOTER"));
    std::cout << "multipart_replace_test passed" << std::endl; return 0;
}
