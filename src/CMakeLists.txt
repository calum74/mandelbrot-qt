
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

include_directories(../mandelbrot/include)
include_directories(../json/include)
link_libraries(extra_fractals)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        Renderer.hpp
        Renderer.cpp
        AsyncRenderer.hpp
        AsyncRenderer.cpp
        AnimatedRenderer.hpp
        AnimatedRenderer.cpp
        ColourMapImpl.cpp
        ColourMap.hpp
        ColourMapImpl.hpp
        ViewerWidget.h
        ViewerWidget.cpp
        Viewport.hpp
        Viewport.cpp
        Fractals.cpp
        gotodialog.h
        gotodialog.cpp
        registry.hpp
        registry.cpp
)

set(MACOSX_BUNDLE_ICON_FILE icon.icns)
set(app_icon_macos "${CMAKE_CURRENT_SOURCE_DIR}/icon.icns")

set_source_files_properties(${app_icon_macos} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(mandelbrot-qt
        MANUAL_FINALIZATION
        MACOSX_BUNDLE
        ${app_icon_macos}
        ${PROJECT_SOURCES}
        ViewerWidget.h ViewerWidget.cpp
        gotodialog.h gotodialog.cpp gotodialog.ui
        resources.qrc
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET mandelbrot-qt APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(mandelbrot-qt SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(mandelbrot-qt
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(mandelbrot-qt PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.mandelbrot-qt)
endif()

set_target_properties(mandelbrot-qt PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE_ICON_FILE icon.icns
)

include(GNUInstallDirs)
install(TARGETS mandelbrot-qt
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(mandelbrot-qt)
endif()

qt_generate_deploy_app_script(
    TARGET mandelbrot-qt
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)

install(SCRIPT ${deploy_script})

include(CPack)
set(CPACK_PACKAGE_NAME "mandelbrot-qt")
set(CPACK_PACKAGE_VENDOR "github.com/calum74")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A viewer for the Mandelbrot set")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "MandelbrotQt")
set(CPACK_PACKAGE_EXECUTABLES mandelbrot-qt "MandelbrotQt")
