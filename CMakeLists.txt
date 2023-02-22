cmake_minimum_required(VERSION 3.7)

project(QYaml VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(${PROJECT_NAME} "")
add_library(QYaml::QYaml ALIAS QYaml)

find_package(Qt6
  REQUIRED
  COMPONENTS
    Core
    Gui
    Widgets
    Network
    Xml
    Svg
    LinguistTools
    Concurrent
    Sql
  OPTIONAL_COMPONENTS
    Core5Compat
#        Zlib
)
qt_standard_project_setup()


target_include_directories(${PROJECT_NAME}
    PUBLIC
        include
#        $<INSTALL_INTERFACE:include>
#        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_options(${PROJECT_NAME}
    PRIVATE
        -Werror
)

target_sources(
    ${PROJECT_NAME}

  PRIVATE
    # These have to be added to get MOC to work correctly apparently
    # Not certain if there is a better way - yet.
    include/qyaml/qyamlbuilder.h
    include/qyaml/qyamlhighlighter.h
    include/qyaml/qyamledit.h
    include/qyaml/qyamlparser.h
    include/qyaml/qyamldocument.h
    include/qyaml/yamlnode.h
    include/qyaml/yamlerrors.h
    include/qyaml_global.h
    # end of MOC shit


    # YAML stuff
    src/qyaml/qyamlbuilder.cpp
    src/qyaml/qyamlhighlighter.cpp
    src/qyaml/qyamledit.cpp
    src/qyaml/qyamlparser.cpp
    src/qyaml/qyamldocument.cpp
    src/qyaml/yamlnode.cpp

)

target_compile_features(${PROJECT_NAME}
    PRIVATE
        cxx_std_17
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Widgets
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Network
        Qt${QT_VERSION_MAJOR}::Xml
        Qt${QT_VERSION_MAJOR}::Svg
        Widgets::Widgets
        QuaZip::QuaZip
        Utilities::Utilities
        QLNPlainText::QLNPlainText
        QMarkdown::QMarkdown
        yaml-cpp
)

option(BUILD_DOC "Build documentation" ON)
find_package(Doxygen)
if (DOXYGEN_FOUND)
  # Only build documentation when building in Release mode
  if (CMAKE_BUILD_TYPE MATCHES "^[Rr]elease")
    # set input and output files
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/doxygen/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    # request to configure the file
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    message("Doxygen build started")

    # note the option ALL which allows to build the docs together with the application
    add_custom_target( doc_doxygen ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM )
  endif()
else (DOXYGEN_FOUND)
  message("Doxygen need to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)
