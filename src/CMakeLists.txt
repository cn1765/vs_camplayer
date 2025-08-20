cmake_minimum_required(VERSION 3.21)

project(MainApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set Qt installation path
set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../out/install/qt;${CMAKE_CURRENT_SOURCE_DIR}/../out/install/opencv")

# Find Qt6 components - 添加 Network 组件
find_package(Qt6 REQUIRED COMPONENTS Core Quick Qml Network)

# Find OpenCV
set(OpenCV_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../out/install/opencv/lib/cmake/opencv4")
find_package(OpenCV REQUIRED COMPONENTS core imgproc)

message(STATUS "OpenCV libraries: ${OpenCV_LIBS}")
message(STATUS "OpenCV include directories: ${OpenCV_INCLUDE_DIRS}")

# Enable automatic MOC, UIC, and RCC processing
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Define source files - 添加 NetworkClient 相关文件
set(SOURCES
    main.cpp
    NetworkClient.cpp
    ImageProvider.cpp
)

# Define header files
set(HEADERS
    NetworkClient.h
    ImageProvider.h
    Logger.h
)

# Define resource files
set(RESOURCES
    qml.qrc
)

# Create executable
qt6_add_executable(MainApp
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
)

# Link Qt libraries - 添加 Network 库
target_link_libraries(MainApp PRIVATE
    Qt6::Core
    Qt6::Quick
    Qt6::Qml
    Qt6::Network
    ${OpenCV_LIBS}
)

# Install the executable
install(TARGETS MainApp
    RUNTIME DESTINATION bin
)