// Diagnostics test: open failure, missing part, XML parse failure, table mismatch
#include "QtDocxTemplate/Docx.hpp"
#include "QtDocxTemplate/Builder.hpp"
#include "QtDocxTemplate/Variables.hpp"
#include "opc/Package.hpp"
#include <QTemporaryDir>
#include <QFile>
#include <cassert>
#include <iostream>

using namespace QtDocxTemplate; using QtDocxTemplate::opc::Package;

static QString writeDocx(const QByteArray &docXml){
    Package pkg; pkg.writePart("[Content_Types].xml", QByteArray(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" \
        "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">" \
        "<Default Extension=\"xml\" ContentType=\"application/xml\"/>" \
        "<Override PartName=\"/word/document.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml\"/>" \
        "</Types>"));
    pkg.writePart("word/document.xml", docXml); QTemporaryDir dir; QString p = dir.path()+"/d.docx"; pkg.saveAs(p); QFile f(p); assert(f.exists()); return p;
}

int main(){
    // Open failure (path does not exist)
    {
        Docx d("/nonexistent/path/does_not_exist.docx");
        d.fillTemplate(Variables{});
        assert(d.lastError().has_value());
        assert(d.lastError().value() == Docx::ErrorCode::OpenFailed);
    }
    // Missing document part
    {
        // Create a package without word/document.xml
        Package pkg; pkg.writePart("[Content_Types].xml", QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\"?><Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\"></Types>"));
        QTemporaryDir dir; QString path = dir.path()+"/nodoc.docx"; pkg.saveAs(path);
        Docx d(path); auto text = d.readTextContent(); (void)text;
        assert(d.lastError().has_value());
        assert(d.lastError().value() == Docx::ErrorCode::DocumentPartMissing);
    }
    // Table mismatch sets error after fillTemplate
    {
        QTemporaryDir dir; assert(dir.isValid());
        QString path = dir.path()+"/mismatch.docx";
        Package pkg; pkg.writePart("[Content_Types].xml", QByteArray(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" \
            "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">" \
            "<Default Extension=\"xml\" ContentType=\"application/xml\"/>" \
            "<Override PartName=\"/word/document.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml\"/>" \
            "</Types>"));
        QByteArray xml("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
                        "<w:document xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"><w:body>"
                        "<w:tbl><w:tr><w:tc><w:p><w:r><w:t>${c1}</w:t></w:r></w:p></w:tc><w:tc><w:p><w:r><w:t>${c2}</w:t></w:r></w:p></w:tc></w:tr></w:tbl>"
                        "</w:body></w:document>");
        pkg.writePart("word/document.xml", xml); pkg.saveAs(path);
        Docx d(path);
        auto tv = makeTableVar({{"c1", {"A","B"}}, {"c2", {"1"}}}); // mismatch lengths 2 vs 1
        Variables vars; vars.add(tv);
        d.fillTemplate(vars);
        assert(d.lastError().has_value());
        if(d.lastError().value() != Docx::ErrorCode::TableColumnLengthMismatch) {
            std::cerr << "Unexpected error code (numeric)=" << (int)(*d.lastError()) << std::endl;
        }
        assert(d.lastError().value() == Docx::ErrorCode::TableColumnLengthMismatch);
    }
    std::cout << "diagnostics_test passed" << std::endl; return 0;
}
