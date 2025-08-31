cmake_minimum_required(VERSION 3.16)
project(QtDocxTemplate VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(GNUInstallDirs)

find_package(Qt6 REQUIRED COMPONENTS Core Gui)
option(QDT_FORCE_SYSTEM_LIBZIP "Require system libzip; fail if missing" OFF)
option(QDT_FORCE_FETCH_MINIZIP "Force FetchContent minizip-ng fallback" OFF)

find_package(libzip QUIET)
find_package(pugixml QUIET)

if(QDT_FORCE_SYSTEM_LIBZIP)
    if(NOT libzip_FOUND)
        message(FATAL_ERROR "QDT_FORCE_SYSTEM_LIBZIP=ON but libzip not found")
    endif()
endif()

if(QDT_FORCE_FETCH_MINIZIP AND QDT_FORCE_SYSTEM_LIBZIP)
    message(FATAL_ERROR "Can't force both system libzip and fetch minizip")
endif()

if(libzip_FOUND AND NOT QDT_FORCE_FETCH_MINIZIP)
    message(STATUS "Using system libzip")
    set(QTDOCTEMPLATE_USE_LIBZIP TRUE)
else()
    include(FetchContent)
    # Minizip-ng fallback
    if(NOT DEFINED MINIZIP_NG_TAG)
        set(MINIZIP_NG_TAG 4.0.4) # reasonable stable tag
    endif()
    set(MZ_OPENSSL OFF CACHE INTERNAL "")
    FetchContent_Declare(minizip_ng
        GIT_REPOSITORY https://github.com/nmoinvaz/minizip.git
        GIT_TAG ${MINIZIP_NG_TAG}
    )
    set(MZ_BUILD_TESTS OFF CACHE INTERNAL "")
    set(MZ_BUILD_EXAMPLES OFF CACHE INTERNAL "")
    set(MZ_BUILD_UNIT_TESTS OFF CACHE INTERNAL "")
    set(MZ_ZLIB ON CACHE INTERNAL "")
    FetchContent_MakeAvailable(minizip_ng)
endif()

if(pugixml_FOUND)
    message(STATUS "Using system pugixml")
else()
    include(FetchContent)
    if(NOT pugixml_POPULATED)
        FetchContent_Declare(pugixml
            GIT_REPOSITORY https://github.com/zeux/pugixml.git
            GIT_TAG v1.14
        )
        FetchContent_MakeAvailable(pugixml)
    endif()
    if(NOT TARGET pugixml::pugixml AND TARGET pugixml)
        add_library(pugixml::pugixml ALIAS pugixml)
    endif()
endif()

add_library(QtDocxTemplate
    src/Docx.cpp
    src/Variables.cpp
    src/Variable.cpp
    src/TextVariable.cpp
    src/ImageVariable.cpp
    src/BulletListVariable.cpp
    src/TableVariable.cpp
    src/Builder.cpp
    src/opc/Package.cpp
    src/xml/XmlPart.cpp
    src/engine/RunModel.cpp
    src/engine/Replacers.cpp
    src/util/Emu.hpp
)

add_library(QtDocxTemplate::core ALIAS QtDocxTemplate)

target_include_directories(QtDocxTemplate
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(QtDocxTemplate PUBLIC cxx_std_17)

target_link_libraries(QtDocxTemplate
    PUBLIC
        Qt6::Core
        Qt6::Gui
    PRIVATE
        $<$<BOOL:${QTDOCTEMPLATE_USE_LIBZIP}>:libzip::zip>
    $<$<NOT:$<BOOL:${QTDOCTEMPLATE_USE_LIBZIP}>>:MINIZIP::minizip>
    pugixml::pugixml
)

target_compile_definitions(QtDocxTemplate PRIVATE QTDOCTEMPLATE_BUILD_LIB)
if(QTDOCTEMPLATE_USE_LIBZIP)
    target_compile_definitions(QtDocxTemplate PRIVATE QTDOCTEMPLATE_USE_LIBZIP=1)
else()
    target_compile_definitions(QtDocxTemplate PRIVATE QTDOCTEMPLATE_USE_MINIZIP=1)
endif()

install(TARGETS QtDocxTemplate
    EXPORT QtDocxTemplateTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Determine and export backend feature flags right before generating config
set(QDT_WITH_LIBZIP OFF)
set(QDT_WITH_MINIZIP OFF)
if(TARGET libzip::zip)
    set(QDT_WITH_LIBZIP ON)
elseif(TARGET MINIZIP::minizip)
    set(QDT_WITH_MINIZIP ON)
elseif(TARGET minizip::minizip)
    set(QDT_WITH_MINIZIP ON)
endif()

# Configure version header (generated)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/QtDocxTemplate/Version.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/generated/QtDocxTemplate/Version.hpp @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/generated/QtDocxTemplate/Version.hpp DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/QtDocxTemplate)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    cmake/QtDocxTemplateConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/QtDocxTemplateConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/QtDocxTemplate
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/QtDocxTemplateConfig.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/QtDocxTemplate
)

install(EXPORT QtDocxTemplateTargets
    NAMESPACE QtDocxTemplate::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/QtDocxTemplate
)

option(QTDOCTXTEMPLATE_BUILD_BENCH "Build local benchmark helpers (not installed)" OFF)
if(QTDOCTXTEMPLATE_BUILD_BENCH)
    add_subdirectory(bench)
endif()

## (Examples / tests removed in minimal distribution CMake)

message(STATUS "Configured QtDocxTemplate ${PROJECT_VERSION}")

# Uninstall target
if(NOT TARGET uninstall)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake IMMEDIATE @ONLY)
    add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()

## (Documentation generation removed in minimal distribution CMake)
