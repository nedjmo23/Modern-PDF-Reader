cmake_minimum_required(VERSION 3.16)

project(ModernPDFReader VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# البحث عن مكتبات الواجهات ومحرك الـ PDF المتطور
find_package(Qt6 REQUIRED COMPONENTS Widgets PdfWidgets)

qt_standard_project_setup()

set(PROJECT_SOURCES
    main.cpp
)

# بناء تطبيق واجهة رسومية نقي بدون نافذة CMD سوداء بفضل WIN32
qt_add_executable(ModernPDFReader WIN32 ${PROJECT_SOURCES})

# ربط البرنامج بالمحرك الرسومي ومحرك ملفات الـ PDF
target_link_libraries(ModernPDFReader PRIVATE Qt6::Widgets Qt6::PdfWidgets)
