#include "QtDocxTemplate/Docx.hpp"
#include "QtDocxTemplate/Variables.hpp"
#include "QtDocxTemplate/TextVariable.hpp"
#include "QtDocxTemplate/TableVariable.hpp"
#include "QtDocxTemplate/ImageVariable.hpp"
#include "opc/Package.hpp"
#include <QTemporaryDir>
#include <QImage>
#include <cassert>
#include <iostream>

using namespace QtDocxTemplate; using QtDocxTemplate::opc::Package;

static std::string contentTypesMinimalWithDocument() {return R"(<?xml version="1.0" encoding="UTF-8"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
  <Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
  <Default Extension="xml"  ContentType="application/xml"/>
  <Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
  <Default Extension="png" ContentType="image/png"/>
</Types>)"; }
static std::string packageRelsMinimal(){return R"(<?xml version="1.0" encoding="UTF-8"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
  <Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
</Relationships>)"; }
static std::string makeDocXml(const std::string &bodyXml){return std::string("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n")+"<w:document xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"><w:body>"+bodyXml+"</w:body></w:document>"; }
static QString writeMinimalDocx(const QString &path,const std::string &docXml){Package pkg;pkg.writePart("[Content_Types].xml", QByteArray::fromStdString(contentTypesMinimalWithDocument()));pkg.writePart("_rels/.rels", QByteArray::fromStdString(packageRelsMinimal()));pkg.writePart("word/document.xml", QByteArray::fromStdString(docXml));bool ok=pkg.saveAs(path);assert(ok);return path;}

int main(){
    QTemporaryDir tmp; assert(tmp.isValid());
    QString in = tmp.path()+"/tbl_img.docx";
    std::string body = "<w:tbl><w:tr><w:tc><w:p><w:r><w:t>${label}</w:t></w:r></w:p></w:tc><w:tc><w:p><w:r><w:t>${icon}</w:t></w:r></w:p></w:tc></w:tr></w:tbl>";
    writeMinimalDocx(in, makeDocXml(body));

    auto tv = std::make_shared<TableVariable>();
    std::vector<VariablePtr> labelCol; labelCol.push_back(std::make_shared<TextVariable>("${label}", "Row1")); labelCol.push_back(std::make_shared<TextVariable>("${label}", "Row2"));
    // Create small in-memory images
    QImage img1(10,10,QImage::Format_ARGB32); img1.fill(Qt::red);
    QImage img2(10,10,QImage::Format_ARGB32); img2.fill(Qt::green);
    std::vector<VariablePtr> iconCol; iconCol.push_back(std::make_shared<ImageVariable>("${icon}", img1, 10, 10)); iconCol.push_back(std::make_shared<ImageVariable>("${icon}", img2, 10, 10));
    tv->addColumn(labelCol); tv->addColumn(iconCol);
    Variables vars; vars.add(tv);
    Docx d(in); d.fillTemplate(vars); QString out = in+".out.docx"; d.save(out);
    Package pkg; assert(pkg.open(out));
    // Basic assertions: text present, placeholder gone, media parts created
    auto xml = pkg.readPart("word/document.xml"); QString doc = *xml;
    assert(doc.contains("Row1")); assert(doc.contains("Row2"));
    assert(!doc.contains("${label}")); assert(!doc.contains("${icon}"));
    // Relationship file should reference images
    auto rels = pkg.readPart("word/_rels/document.xml.rels"); assert(rels.has_value()); QString relsXml = *rels; assert(relsXml.contains("image"));
    std::cout << "table_image_replace_test passed" << std::endl; return 0;
}
