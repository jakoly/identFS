cmake_minimum_required(VERSION 3.16)

project(IdentFS VERSION 0.1 LANGUAGES CXX C)

# ----------------------------------
# Qt Setup
# ----------------------------------
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# ----------------------------------
# Projektquellen
# ----------------------------------
set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    resources.qrc
)

# ----------------------------------
# SQLite
# ----------------------------------
# Nur sqlite3.c und sqlite3.h benötigen wir
set(SQLITE_SOURCES
    libs/sqlite/sqlite3.c
)

# ----------------------------------
# Executable erstellen
# ----------------------------------
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(IdentFS
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        addfiles.h addFiles.cpp addfiles.ui
        addfiles.h addfiles.cpp addfiles.ui
        tools.h tools.cpp


    )
else()
    add_executable(IdentFS
        ${PROJECT_SOURCES}
    )
endif()

# SQLite als C-Quelle hinzufügen
target_sources(IdentFS PRIVATE ${SQLITE_SOURCES})

# Qt Widgets Library verlinken
target_link_libraries(IdentFS PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# ----------------------------------
# MacOS / iOS Bundle Einstellungen
# ----------------------------------
if(${QT_VERSION} VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.IdentFS)
endif()

set_target_properties(IdentFS PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# ----------------------------------
# Installation
# ----------------------------------
include(GNUInstallDirs)
install(TARGETS IdentFS
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# ----------------------------------
# Qt6: Finale
# ----------------------------------
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(IdentFS)
endif()
