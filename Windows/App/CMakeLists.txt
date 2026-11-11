# KuocKey.exe: tray app + hooks. Built only on Windows (MSVC).
# One version for every platform: project.yml's MARKETING_VERSION, unless -DKUOCKEY_VERSION overrides it.
unset(KUOCKEY_VERSION CACHE)   # never a stale cached value
if(NOT KUOCKEY_VERSION)
  file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/../../project.yml" _mv REGEX "MARKETING_VERSION: \"")
  string(REGEX REPLACE ".*MARKETING_VERSION: \"([^\"]+)\".*" "\\1" KUOCKEY_VERSION "${_mv}")
endif()
if(NOT KUOCKEY_BUILD)
  file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/../../project.yml" _bv REGEX "CURRENT_PROJECT_VERSION: \"")
  string(REGEX REPLACE ".*CURRENT_PROJECT_VERSION: \"([^\"]+)\".*" "\\1" KUOCKEY_BUILD "${_bv}")
endif()
message(STATUS "KuocKey version ${KUOCKEY_VERSION} (${KUOCKEY_BUILD})")
add_executable(kuockey WIN32 main.cpp Core.cpp Prefs.cpp Site.cpp Settings.cpp Emoji.cpp Macro.cpp KuocKey.rc)
set_target_properties(kuockey PROPERTIES OUTPUT_NAME KuocKey)
target_include_directories(kuockey PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(kuockey PRIVATE UNICODE _UNICODE KUOCKEY_VERSION="${KUOCKEY_VERSION}" KUOCKEY_BUILD="${KUOCKEY_BUILD}")
target_link_libraries(kuockey PRIVATE kuockey_engine user32 shell32 gdi32 advapi32 imm32 psapi ole32 oleaut32 comctl32 gdiplus winmm d2d1 dwrite dwmapi normaliz comdlg32)
target_link_options(kuockey PRIVATE /MANIFEST:NO)
# Live test runner: a window with an edit control, types through SendInput while KuocKey runs, reads the text back.
add_executable(kuockey_livetest ../Tests/livetest.cpp Prefs.cpp)
target_include_directories(kuockey_livetest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(kuockey_livetest PRIVATE kuockey_engine user32 advapi32 shell32)
target_compile_definitions(kuockey_livetest PRIVATE UNICODE _UNICODE)
# Shortcut table (Bảng gõ tắt) test: lookup, validation, CSV import/export. check.ps1 runs it.
add_executable(kuockey_macrotest ../Tests/macrotest.cpp Macro.cpp)
target_include_directories(kuockey_macrotest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(kuockey_macrotest PRIVATE UNICODE _UNICODE)
target_link_libraries(kuockey_macrotest PRIVATE user32 normaliz)
