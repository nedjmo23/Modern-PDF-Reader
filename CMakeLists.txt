cmake_minimum_required(VERSION 3.16)

project(ModernPDFReader VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# البحث عن مكتبات Qt6 المطلوبة للواجهة والتبويبات
find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

# تحديد ملفات المشروع
set(PROJECT_SOURCES
    main.cpp
)

# إنشاء البرنامج التنفيذي
qt_add_executable(ModernPDFReader ${PROJECT_SOURCES})

# ربط البرنامج بمكتبات Qt6
target_link_libraries(ModernPDFReader PRIVATE Qt6::Widgets)
