#include <QtDocxTemplate/Docx.hpp>
#include <QtDocxTemplate/Variables.hpp>
#include <QtDocxTemplate/TextVariable.hpp>
#include <QtDocxTemplate/TableVariable.hpp>
#include <QtDocxTemplate/ImageVariable.hpp>
#include <QtDocxTemplate/BulletListVariable.hpp>
#include <QtDocxTemplate/Builder.hpp>
#include <QImage>
#include <iostream>

// Demonstrates a Java templ4docx-like workflow using new alias methods.
// Template expectation: a table row with placeholders ${plate} ${company} ${speed} ${maxCps} ${when}

using namespace QtDocxTemplate;

int main(){
    Docx doc("report_template.docx");
    Variables vars;
    vars.addTextVariable("${title}", "RADGUS Report Parity");
    // Build table with columns (plate, company, speed, maxCps, when) two rows
    auto table = std::make_shared<TableVariable>();
    table->addVariable({ std::make_shared<TextVariable>("${plate}", "06ABC123"), std::make_shared<TextVariable>("${plate}", "06XYZ789") });
    table->addVariable({ std::make_shared<TextVariable>("${company}", "Alpha"), std::make_shared<TextVariable>("${company}", "Beta") });
    table->addVariable({ std::make_shared<TextVariable>("${speed}", "3.40"), std::make_shared<TextVariable>("${speed}", "5.10") });
    table->addVariable({ std::make_shared<TextVariable>("${maxCps}", "0"), std::make_shared<TextVariable>("${maxCps}", "0") });
    table->addVariable({ std::make_shared<TextVariable>("${when}", "02-01-2024 08:08:00"), std::make_shared<TextVariable>("${when}", "02-01-2024 08:09:00") });
    vars.addTableVariable(table);

    doc.fillTemplate(vars);
    doc.save("report_parity_output.docx");
    std::cout << "parity_workflow example complete" << std::endl;
    return 0;
}
