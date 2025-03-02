cmake_minimum_required(VERSION 3.16)
project(ZXingBarcodeReader VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# Include Qt packages
find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Multimedia Concurrent)

# Configure ZXing-CPP as a subproject (with limited formats)
set(ENABLE_ENCODERS ON CACHE BOOL "Enable encoders")
set(ENABLE_DECODERS ON CACHE BOOL "Enable decoders")
set(ENABLE_ONE_D_BARCODES ON CACHE BOOL "Enable 1D barcodes")
set(ENABLE_CODE_128 ON CACHE BOOL "Enable Code128 barcodes")
# Turn off other formats we don't need
set(ENABLE_QR_CODE OFF CACHE BOOL "Disable QR Code")
set(ENABLE_DATA_MATRIX OFF CACHE BOOL "Disable DataMatrix")
set(ENABLE_AZTEC OFF CACHE BOOL "Disable Aztec")
set(ENABLE_PDF417 OFF CACHE BOOL "Disable PDF417")
set(ENABLE_MAXICODE OFF CACHE BOOL "Disable MaxiCode")
set(ENABLE_DATA_BAR OFF CACHE BOOL "Disable DataBar")
set(ENABLE_DATA_BAR_EXPANDED OFF CACHE BOOL "Disable DataBarExpanded")

add_subdirectory(zxing-cpp)

set(APP_SOURCES
    main.cpp
    # src/barcodemanager.h
    # src/barcodemanager.cpp
    # src/imageprovider.h
    # src/imageprovider.cpp
    # qml.qrc
)

qt_add_executable(${PROJECT_NAME}
    ${APP_SOURCES}
    barcodemanager.h barcodemanager.cpp
    generatedimageprovider.h generatedimageprovider.cpp
    imageprovider.h imageprovider.cpp
)


qt_add_qml_module(${PROJECT_NAME}
    URI ZXingBarcodeReader
    VERSION 1.0
    QML_FILES Main.qml
)

# Include directories for ZXing (if needed)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/zxing-cpp/core/src
)

# Qt configuration
target_compile_definitions(${PROJECT_NAME} PRIVATE
    QT_DEPRECATED_WARNINGS
    $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::Multimedia
    Qt6::Concurrent
    ZXing::Core
)

# Install directives
install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
