## CppDocxTemplate – Utility library for filling templates in DOCX files (Qt 6 / C++)

Lightweight C++ library for reading DOCX text, finding placeholders, and filling templates with text, images, bullet lists and tables (including headers & footers). Inspired by templ4docx. Repository trimmed to only essential library sources (see layout below).

Core features:
- Read full document text (`Docx::readTextContent()`)
- Discover unique variables (`Docx::findVariables()`)
- Custom variable pattern (default `${` `}`)
- Fill template with provided variables (`Docx::fillTemplate()`)
- Table expansion (column-wise), image insertion, bullet list expansion with numbering, header/footer replacement

### Get It (CMake)
```bash
cmake -S . -B build
cmake --build build -j
cmake --install build --prefix /your/prefix
```
```cmake
find_package(QtDocxTemplate CONFIG REQUIRED)
target_link_libraries(your_app PRIVATE QtDocxTemplate::core Qt6::Core Qt6::Gui)
```

### Minimal Example
```cpp
using namespace QtDocxTemplate;
Docx doc("template.docx");
doc.setVariablePattern({"#{","}"}); // optional
Variables vars;
vars.addTextVariable(std::make_shared<TextVariable>("#{firstname}", "Eren"));
vars.addTextVariable(std::make_shared<TextVariable>("#{lastname}",  "Ozen"));
doc.fillTemplate(vars);
doc.save("filled.docx");
```

### Variable Types
- TextVariable
- ImageVariable
- BulletListVariable
- TableVariable

Aliases (templ4docx style): `addTextVariable`, `addImageVariable`, `addBulletListVariable`, `addTableVariable`, `TableVariable::addVariable`.

Helper utilities: row-to-column table builder (`makeTableVarFromRows`), column placeholder validator (`validateTableColumnPlaceholders`).

### License
Apache-2.0 (see `LICENSE`).

### Minimal Table Snippet
```cpp
auto table = std::make_shared<TableVariable>();
table->addVariable({ std::make_shared<TextVariable>("${col1}", "A"), std::make_shared<TextVariable>("${col1}", "B") });
table->addVariable({ std::make_shared<TextVariable>("${col2}", "1"), std::make_shared<TextVariable>("${col2}", "2") });
Variables vars; vars.addTableVariable(table);
Docx doc("t.docx"); doc.fillTemplate(vars); doc.save("t_out.docx");
```

### Images & Bullets (quick)
```cpp
QImage img(64,64,QImage::Format_ARGB32); img.fill(Qt::red);
vars.addImageVariable(std::make_shared<ImageVariable>("${logo}", img, 64, 64));
auto bullets = std::make_shared<BulletListVariable>("${skills}");
bullets->addItem(std::make_shared<TextVariable>("${s1}", "C++"));
bullets->addItem(std::make_shared<TextVariable>("${s2}", "Qt"));
vars.addBulletListVariable(bullets);
```

### Finding Variables
```cpp
Docx doc("template.docx");
QStringList varsFound = doc.findVariables();
```

### Optional Build Flags
- `QDT_FORCE_SYSTEM_LIBZIP` – require system libzip (fail if missing)
- `QDT_FORCE_FETCH_MINIZIP` – force minizip-ng FetchContent even if libzip present

### Dependencies
Qt6 (Core, Gui), pugixml, libzip or minizip-ng (auto fallback). All bundled or resolved automatically when not present system-wide.

### Repository Layout (minimal distribution)
```
include/QtDocxTemplate/   Public headers
src/                      Implementation
cmake/                    CMake package config helpers
CMakeLists.txt            Build script
LICENSE                   Apache-2.0 license
README.md                 This file
```
Generated / local-only (ignored): `build/`, `install/`, `tmp_*`, editor configs.
