cmake_minimum_required(VERSION 3.16)

project(ModernPDFReader VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# مهم جدا لـ Q_OBJECT
set(CMAKE_AUTOMOC ON)

# البحث عن Qt6 و Poppler
find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Poppler REQUIRED COMPONENTS Qt6)

qt_standard_project_setup()

set(PROJECT_SOURCES
    main.cpp
)

# إنشاء التطبيق
qt_add_executable(ModernPDFReader
    ${PROJECT_SOURCES}
)

# ربط المكتبات
target_link_libraries(ModernPDFReader PRIVATE
    Qt6::Widgets
    Poppler::Qt6
)
