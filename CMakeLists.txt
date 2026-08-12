cmake_minimum_required(VERSION 3.16)
project(ModernPDFReader VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()
set(PROJECT_SOURCES
    main.cpp
    Common.h
    IslandToggleButton.h
    DynamicIsland.h
    BookTab.h
    RecentCard.h
    HomeButton.h
    ModernPDFReader.h
)
qt_add_executable(ModernPDFReader WIN32
    ${PROJECT_SOURCES}
)
target_link_libraries(ModernPDFReader PRIVATE
    Qt6::Widgets
)
# ربط مكتبة ويندوز DWM للأنيميشن وحواف النافذة حصراً عند العمل على ويندوز
if(WIN32)
    target_link_libraries(ModernPDFReader PRIVATE dwmapi)
endif()
