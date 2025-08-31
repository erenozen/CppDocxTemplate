#include "QtDocxTemplate/Docx.hpp"
#include "QtDocxTemplate/Variables.hpp"
#include "QtDocxTemplate/TextVariable.hpp"
#include "QtDocxTemplate/TableVariable.hpp"
#include "opc/Package.hpp"
#include <QTemporaryDir>
#include <cassert>
#include <iostream>

using namespace QtDocxTemplate; using QtDocxTemplate::opc::Package;

static std::string contentTypesMinimalWithDocument() {return R"(<?xml version="1.0" encoding="UTF-8"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
  <Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
  <Default Extension="xml"  ContentType="application/xml"/>
  <Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
</Types>)"; }
static std::string packageRelsMinimal(){return R"(<?xml version="1.0" encoding="UTF-8"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
  <Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
</Relationships>)"; }
static std::string makeDocXml(const std::string &bodyXml){return std::string("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n")+"<w:document xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"><w:body>"+bodyXml+"</w:body></w:document>"; }
static QString writeMinimalDocx(const QString &path,const std::string &docXml){Package pkg;pkg.writePart("[Content_Types].xml", QByteArray::fromStdString(contentTypesMinimalWithDocument()));pkg.writePart("_rels/.rels", QByteArray::fromStdString(packageRelsMinimal()));pkg.writePart("word/document.xml", QByteArray::fromStdString(docXml));bool ok=pkg.saveAs(path);assert(ok);return path;}

int main(){
    QTemporaryDir tmp; assert(tmp.isValid());
    QString in = tmp.path()+"/tbl_mismatch.docx";
    std::string body = "<w:tbl><w:tr><w:tc><w:p><w:r><w:t>${colA}</w:t></w:r></w:p></w:tc><w:tc><w:p><w:r><w:t>${colB}</w:t></w:r></w:p></w:tc></w:tr></w:tbl>";
    writeMinimalDocx(in, makeDocXml(body));

    auto tv = std::make_shared<TableVariable>();
    std::vector<VariablePtr> colA; colA.push_back(std::make_shared<TextVariable>("${colA}", "A1")); colA.push_back(std::make_shared<TextVariable>("${colA}", "A2")); colA.push_back(std::make_shared<TextVariable>("${colA}", "A3"));
    std::vector<VariablePtr> colB; colB.push_back(std::make_shared<TextVariable>("${colB}", "B1")); colB.push_back(std::make_shared<TextVariable>("${colB}", "B2")); // shorter column
    tv->addColumn(colA); tv->addColumn(colB);
    Variables vars; vars.add(tv);
    Docx d(in); d.fillTemplate(vars); QString out = in+".out.docx"; d.save(out);
    Package pkg; assert(pkg.open(out)); auto xml = pkg.readPart("word/document.xml"); QString doc = *xml;
    // Expect only two rows (min length = 2) => A1,A2,B1,B2 present; A3 absent
    assert(doc.contains("A1")); assert(doc.contains("A2")); assert(!doc.contains("A3"));
    assert(doc.contains("B1")); assert(doc.contains("B2"));
    std::cout << "table_replace_mismatch_test passed" << std::endl; return 0;
}
