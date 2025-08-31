# QtDocxTemplate (C++ / Qt 6)

[![License](https://img.shields.io/badge/license-Apache--2.0-blue.svg)](LICENSE)

A comprehensive C++ library for DOCX template processing and variable replacement. Supports text, image, bullet list, and table variable expansion in document templates.

📖 **[View Documentation](https://erenozen.github.io/QtDocxTemplate/)**

## Quick Start
```bash
cmake -S . -B build -DQTDOCTXTEMPLATE_BUILD_TESTS=ON
cmake --build build -j
ctest --test-dir build -V
```

Install & consume:
```bash
cmake --install build --prefix /tmp/qdt-prefix
```
```cmake
find_package(QtDocxTemplate CONFIG REQUIRED)
add_executable(app main.cpp)
target_link_libraries(app PRIVATE QtDocxTemplate::core Qt6::Core Qt6::Gui)
```

## Minimal Usage
```cpp
using namespace QtDocxTemplate;
Docx doc("template.docx");
Variables vars;
vars.add(std::make_shared<TextVariable>("${firstname}", "Eren"));
vars.add(std::make_shared<TextVariable>("${lastname}",  "Ozen"));
doc.fillTemplate(vars);
doc.save("output.docx");
```

## Variable Types
- TextVariable (style preserved, run fragmentation neutral)
- ImageVariable (pixels → EMU @ 96 DPI)
- BulletListVariable (list of text items)
- TableVariable (column-wise lists; row replication across document, headers & footers)

Utility helpers: `Docx::readTextContent()`, `Docx::findVariables()`.

### templ4docx Parity Aliases & Helpers
For easier migration from Java templ4docx naming, the following aliases are provided (identical behavior):
`Variables::addTextVariable`, `addImageVariable`, `addBulletListVariable`, `addTableVariable` and `TableVariable::addVariable` (adds a column). Existing concise methods remain.

Additional helpers for parity ergonomics:
- `makeTableVarFromRows(keys, rows)` build column-wise table variable from row maps
- `validateTableColumnPlaceholders(vars)` warns/returns any table column placeholders absent from the template (document + headers/footers)

Parity table example (Java-like style):
```cpp
Variables vars;
auto table = std::make_shared<TableVariable>();
table->addVariable({ std::make_shared<TextVariable>("${plate}", "06ABC123"), std::make_shared<TextVariable>("${plate}", "06XYZ789") });
table->addVariable({ std::make_shared<TextVariable>("${company}", "Alpha"), std::make_shared<TextVariable>("${company}", "Beta") });
vars.addTableVariable(table);
doc.fillTemplate(vars);
```
Row-based helper example:
```cpp
std::vector<QMap<QString,QString>> rows;
QMap<QString,QString> r1; r1["plate"]="06ABC123"; r1["company"]="Alpha"; rows.push_back(r1);
QMap<QString,QString> r2; r2["plate"]="06XYZ789"; r2["company"]="Beta"; rows.push_back(r2);
auto table = makeTableVarFromRows(doc, {"plate","company"}, rows);
vars.addTableVariable(table);
QStringList missing = doc.validateTableColumnPlaceholders(vars); // optional validation
```
See example target `parity_workflow` and test `parity_aliases_test`.

## Extended Examples

Custom pattern:
```cpp
doc.setVariablePattern({"#{","}"});
vars.add(std::make_shared<TextVariable>("#{city}", "Berlin"));
```

Image insertion:
```cpp
QImage logo(80,80,QImage::Format_ARGB32); logo.fill(Qt::red);
vars.add(std::make_shared<ImageVariable>("${logo}", logo, 80, 80));
```

Bullet list:
```cpp
auto bl = std::make_shared<BulletListVariable>("${skills}");
bl->addItem(std::make_shared<TextVariable>("${i1}", "C++"));
bl->addItem(std::make_shared<TextVariable>("${i2}", "Qt"));
vars.add(bl);
```

Table (4 columns: name, age, avatar image, skills bullet list):
```cpp
TableVariable table("${students}");
// columns are parallel vectors of VariablePtr (same length)
std::vector<VariablePtr> colName { TV("${n1}", "Alice"), TV("${n2}", "Bob") };
std::vector<VariablePtr> colAge  { TV("${a1}", "30"),    TV("${a2}", "25") };
QImage avatar(32,32,QImage::Format_ARGB32); avatar.fill(Qt::blue);
std::vector<VariablePtr> colImg  { IV("${i1}", avatar,32,32), IV("${i2}", avatar,32,32) };
auto mkSkills = [](QString a, QString b){
	auto bl = std::make_shared<BulletListVariable>("${skills}");
	bl->addItem(std::make_shared<TextVariable>("${k1}", a));
	bl->addItem(std::make_shared<TextVariable>("${k2}", b));
	return std::static_pointer_cast<Variable>(bl);
};
std::vector<VariablePtr> colSkills { mkSkills("C++","Qt"), mkSkills("Python","Docs") };
table.addColumn(colName); table.addColumn(colAge); table.addColumn(colImg); table.addColumn(colSkills);
vars.add(std::make_shared<TableVariable>(table));
```
Helper aliases for brevity (user-defined):
```cpp
auto TV = [](QString key, QString v){ return std::make_shared<TextVariable>(key, v); };
auto IV = [&](QString key, const QImage &img, int w, int h){ return std::make_shared<ImageVariable>(key, img, w, h); };
```

## Dependencies

QtDocxTemplate has minimal external dependencies with smart fallbacks:

| Component | License | Purpose | Acquisition |
|-----------|---------|---------|-------------|
| **Qt 6** (Core, Gui) | LGPL-3.0 | Core framework, XML/image processing | System package (required) |
| **pugixml** | MIT | Fast XML parsing and DOM manipulation | System package or auto-downloaded |
| **libzip** | BSD-3-Clause | ZIP archive handling (preferred) | System package or falls back to minizip-ng |
| **minizip-ng** | Zlib | ZIP archive handling (fallback) | Auto-downloaded if libzip unavailable |

**Smart Dependency Resolution:**
- Prefers system-installed libraries when available
- Automatically downloads missing dependencies via CMake FetchContent
- Configurable via build options for enterprise environments

## Build Options
| Option | Default | Description |
|--------|---------|-------------|
| QDT_FORCE_SYSTEM_LIBZIP | OFF | Require system libzip or fail |
| QDT_FORCE_FETCH_MINIZIP | OFF | Force FetchContent minizip-ng |
| QDT_BUILD_DOCS | OFF | Build Doxygen docs target `docs` |
| QTDOCTXTEMPLATE_BUILD_TESTS | ON | Build tests (ctest) |
| QTDOCTXTEMPLATE_BUILD_EXAMPLES | ON | Build examples |

Feature flags exported in `QtDocxTemplateConfig.cmake`:
`QtDocxTemplate_WITH_LIBZIP`, `QtDocxTemplate_WITH_MINIZIP`.

## Style & Replacement Semantics (Parity Rules)
- Variable detection spans run boundaries; duplicates deduped in `findVariables()`.
- Replacement preserves leading/trailing whitespace and first run styling.
- Non-text (images) inserted structurally (not aggregated into surrounding text runs).
- Table replication uses the first row under the table variable cell as a template.

## Roadmap / Non-Goals
Feature-complete for primary templ4docx parity (text, images, bullet lists with auto-numbering, tables, headers/footers, validator). Out of scope (for now): hyperlinks, advanced drawing objects, nested/complex table scenarios beyond simple row replication, tracked changes.

## License & Notices
Apache-2.0. See LICENSE and THIRD_PARTY_NOTICES.md. This project is an independent implementation.

**Third-Party Licenses:** All dependencies are compatible with commercial use. See [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md) for complete license information.
