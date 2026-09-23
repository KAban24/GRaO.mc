cmake_minimum_required(VERSION 3.20)
project(GRaO_mc CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_definitions(IMGUI_USE_WCHAR32)

if(MSVC)
    add_compile_options(/O2 /Oi /Ot /MP /W3 /arch:AVX2 /DNOMINMAX /D_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-O3 -mavx2 -DNOMINMAX)
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/ImGui
    ${CMAKE_CURRENT_SOURCE_DIR}/ImGui/backends
)

set(IMGUI_SOURCES
    ImGui/imgui.cpp
    ImGui/imgui_draw.cpp
    ImGui/imgui_tables.cpp
    ImGui/imgui_widgets.cpp
    ImGui/backends/imgui_impl_win32.cpp
    ImGui/backends/imgui_impl_dx11.cpp
)

add_executable(GRaO_mc WIN32
    main.cpp
    ${IMGUI_SOURCES}
)

target_link_libraries(GRaO_mc PRIVATE
    d3d11
    dxgi
    d3dcompiler
    dwmapi
    winmm
)
