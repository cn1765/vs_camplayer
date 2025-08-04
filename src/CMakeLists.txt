cmake_minimum_required(VERSION 3.21)

project(MainApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set Qt installation path
set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../out/install/qt")

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Core Quick Qml)

# Enable automatic MOC, UIC, and RCC processing
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Define source files
set(SOURCES
    main.cpp
)

# Define resource files
set(RESOURCES
    qml.qrc
)

# Create executable
qt6_add_executable(MainApp
    ${SOURCES}
    ${RESOURCES}
)

# Link Qt libraries
target_link_libraries(MainApp PRIVATE
    Qt6::Core
    Qt6::Quick
    Qt6::Qml
)

# Install the executable
install(TARGETS MainApp
    RUNTIME DESTINATION bin
)
