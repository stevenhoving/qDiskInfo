set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)

#qt_policy(SET QTP0001 NEW)
#qt_policy(SET QTP0002 NEW)
#qt_policy(SET QTP0003 NEW)

qt_standard_project_setup()

file(GLOB_RECURSE SOURCES
  CONFIGURE_DEPENDS
    "*.cpp"
    "*.h"
    "*.qml"
    "*.qrc"
)

qt_add_executable(qDiskInfo
    ${SOURCES}
)

set_target_properties(qDiskInfo PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

#qt_add_qml_module(qDiskInfo
#    URI DiskInfo
#    VERSION 1.0
#    QML_FILES
#        Main.qml
#        #DetectorImage.qml
#        #DetectorLegend.qml
#        #GradientGauge.qml
#    #SOURCES
#    #    QmlTheme.h
#    #    QmlTheme.cpp
#    #    MaskedMouseArea.h
#    #    MaskedMouseArea.cpp
#    RESOURCES
#        resources.qrc
#)

#file(GLOB_RECURSE SOURCES
#  CONFIGURE_DEPENDS
#    "*.qml"
#    "*.h"
#    "*.ui"
#    "*.qrc"
#)
#
#qt_add_qml_module(appDetectorLayout
#    URI DetectorLayout
#    VERSION 1.0
#    QML_FILES
#        Main.qml
#        DetectorImage.qml
#        DetectorLegend.qml
#        GradientGauge.qml
#    SOURCES
#        QmlTheme.h
#        QmlTheme.cpp
#        MaskedMouseArea.h
#        MaskedMouseArea.cpp
#    RESOURCES resources.qrc
#)

#if (WIN32)
#    # /ENTRY:mainCRTStartup keeps the same "main" function instead of requiring "WinMain"
#    set(SUBSYSTEM_LINKER_OPTIONS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
#else()
#    set(SUBSYSTEM_LINKER_OPTIONS "-mwindows")
#endif()
#
#target_link_options(chrome_extention_blocker_tray PRIVATE ${SUBSYSTEM_LINKER_OPTIONS})

set(MSVC_WARNINGS
      /W3 # Baseline reasonable warnings
      /w14242 # 'identifier': conversion from 'type1' to 'type1', possible loss of data
      /w14254 # 'operator': conversion from 'type1:field_bits' to 'type2:field_bits', possible loss of data
      /w14263 # 'function': member function does not override any base class virtual member function
      /w14265 # 'classname': class has virtual functions, but destructor is not virtual instances of this class may not
              # be destructed correctly
      /w14287 # 'operator': unsigned/negative constant mismatch
      /we4289 # nonstandard extension used: 'variable': loop control variable declared in the for-loop is used outside
              # the for-loop scope
      /w14296 # 'operator': expression is always 'boolean_value'
      /w14311 # 'variable': pointer truncation from 'type1' to 'type2'
      /w14545 # expression before comma evaluates to a function which is missing an argument list
      /w14546 # function call before comma missing argument list
      /w14547 # 'operator': operator before comma has no effect; expected operator with side-effect
      /w14549 # 'operator': operator before comma has no effect; did you intend 'operator'?
      /w14555 # expression has no effect; expected expression with side- effect
      /w14619 # pragma warning: there is no warning number 'number'
      /w14640 # Enable warning on thread un-safe static member initialization
      /w14826 # Conversion from 'type1' to 'type_2' is sign-extended. This may cause unexpected runtime behavior.
      /w14905 # wide string literal cast to 'LPSTR'
      /w14906 # string literal cast to 'LPWSTR'
      /w14928 # illegal copy-initialization; more than one user-defined conversion has been implicitly applied
      /permissive- # standards conformance mode for MSVC compiler.
  )

target_link_libraries(qDiskInfo
  PUBLIC
    Qt6::Core
    Qt6::Widgets
    Qt6::Quick
    libdiskinfo
)

target_include_directories(qDiskInfo
  PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
    .
)

target_compile_options(qDiskInfo
  PUBLIC
    /utf-8
    /diagnostics:caret
    ${MSVC_WARNINGS}
)

target_compile_definitions(qDiskInfo
  PUBLIC
    NOMINMAX
)

install(
    TARGETS qDiskInfo
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    BUNDLE  DESTINATION .
)

set(deploy_tool_options_arg
    --no-opengl-sw
    --no-ffmpeg
    --no-translations
    --no-pdf
    #--no-network
    --no-svg
    --no-system-d3d-compiler
    --no-system-dxc-compiler
)

qt_generate_deploy_qml_app_script(
    TARGET qDiskInfo
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
    DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
    #DEPLOY_TOOL_OPTIONS ${deploy_tool_options_arg}
)

install(SCRIPT ${deploy_script})

