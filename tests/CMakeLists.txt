add_executable(qtdocxtemplate_tests test_main.cpp)
target_link_libraries(qtdocxtemplate_tests PRIVATE QtDocxTemplate::core)
add_test(NAME qtdocxtemplate_smoke COMMAND qtdocxtemplate_tests)

add_executable(package_test package_test.cpp)
target_include_directories(package_test PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_BINARY_DIR}/_deps/pugixml-src/src)
target_link_libraries(package_test PRIVATE QtDocxTemplate::core Qt6::Core)
add_test(NAME qtdocxtemplate_package COMMAND package_test)

add_executable(xml_test xml_test.cpp)
target_include_directories(xml_test PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_BINARY_DIR}/_deps/pugixml-src/src)
target_link_libraries(xml_test PRIVATE QtDocxTemplate::core Qt6::Core)
add_test(NAME qtdocxtemplate_xml COMMAND xml_test)

add_executable(text_replace_test text_replace_test.cpp)
target_include_directories(text_replace_test PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_BINARY_DIR}/_deps/pugixml-src/src)
target_link_libraries(text_replace_test PRIVATE QtDocxTemplate::core Qt6::Core)
add_test(NAME qtdocxtemplate_text_replace COMMAND text_replace_test)

add_executable(text_replace_edge_test text_replace_edge_test.cpp)
target_include_directories(text_replace_edge_test PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_BINARY_DIR}/_deps/pugixml-src/src)
target_link_libraries(text_replace_edge_test PRIVATE QtDocxTemplate::core Qt6::Core Qt6::Gui)
add_test(NAME qtdocxtemplate_text_replace_edges COMMAND text_replace_edge_test)

add_executable(image_replace_test image_replace_test.cpp)
target_include_directories(image_replace_test PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_BINARY_DIR}/_deps/pugixml-src/src)
target_link_libraries(image_replace_test QtDocxTemplate::core)
add_test(NAME qtdocxtemplate_image_replace COMMAND image_replace_test)

add_executable(bullet_replace_test bullet_replace_test.cpp)
target_include_directories(bullet_replace_test PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_BINARY_DIR}/_deps/pugixml-src/src)
target_link_libraries(bullet_replace_test QtDocxTemplate::core)
add_test(NAME qtdocxtemplate_bullet_replace COMMAND bullet_replace_test)

add_executable(bullet_numbering_test bullet_numbering_test.cpp)
target_include_directories(bullet_numbering_test PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_BINARY_DIR}/_deps/pugixml-src/src)
target_link_libraries(bullet_numbering_test QtDocxTemplate::core)
add_test(NAME qtdocxtemplate_bullet_numbering COMMAND bullet_numbering_test)

add_executable(table_replace_test table_replace_test.cpp)
target_include_directories(table_replace_test PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_BINARY_DIR}/_deps/pugixml-src/src)
target_link_libraries(table_replace_test QtDocxTemplate::core)
add_test(NAME qtdocxtemplate_table_replace COMMAND table_replace_test)

add_executable(table_replace_mismatch_test table_replace_mismatch_test.cpp)
target_include_directories(table_replace_mismatch_test PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_BINARY_DIR}/_deps/pugixml-src/src)
target_link_libraries(table_replace_mismatch_test QtDocxTemplate::core)
add_test(NAME qtdocxtemplate_table_replace_mismatch COMMAND table_replace_mismatch_test)

add_executable(table_image_replace_test table_image_replace_test.cpp)
target_include_directories(table_image_replace_test PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_BINARY_DIR}/_deps/pugixml-src/src)
target_link_libraries(table_image_replace_test QtDocxTemplate::core Qt6::Gui)
add_test(NAME qtdocxtemplate_table_image_replace COMMAND table_image_replace_test)

add_executable(multipart_replace_test multipart_replace_test.cpp)
target_include_directories(multipart_replace_test PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_BINARY_DIR}/_deps/pugixml-src/src)
target_link_libraries(multipart_replace_test QtDocxTemplate::core)
add_test(NAME qtdocxtemplate_multipart_replace COMMAND multipart_replace_test)

add_executable(qtdocxtemplate_builder_api_test builder_api_test.cpp)
target_include_directories(qtdocxtemplate_builder_api_test PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_BINARY_DIR}/_deps/pugixml-src/src)
target_link_libraries(qtdocxtemplate_builder_api_test QtDocxTemplate::core Qt6::Core Qt6::Gui)
add_test(NAME qtdocxtemplate_builder_api COMMAND qtdocxtemplate_builder_api_test)

add_executable(qtdocxtemplate_builder_pattern_test builder_pattern_test.cpp)
target_include_directories(qtdocxtemplate_builder_pattern_test PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_BINARY_DIR}/_deps/pugixml-src/src)
target_link_libraries(qtdocxtemplate_builder_pattern_test QtDocxTemplate::core Qt6::Core)
add_test(NAME qtdocxtemplate_builder_pattern COMMAND qtdocxtemplate_builder_pattern_test)

add_executable(diagnostics_test diagnostics_test.cpp)
target_include_directories(diagnostics_test PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_BINARY_DIR}/_deps/pugixml-src/src)
target_link_libraries(diagnostics_test QtDocxTemplate::core Qt6::Core Qt6::Gui)
add_test(NAME qtdocxtemplate_diagnostics COMMAND diagnostics_test)

add_executable(parity_aliases_test parity_aliases_test.cpp)
target_include_directories(parity_aliases_test PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_BINARY_DIR}/_deps/pugixml-src/src)
target_link_libraries(parity_aliases_test QtDocxTemplate::core Qt6::Core Qt6::Gui)
add_test(NAME qtdocxtemplate_parity_aliases COMMAND parity_aliases_test)
