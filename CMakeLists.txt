cmake_minimum_required(VERSION 3.21)

project(avm-retom
    VERSION 0.1.0
    DESCRIPTION "GUI for operating the RETOM-51 test bench"
    LANGUAGES CXX
)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.2 REQUIRED COMPONENTS Core SerialPort Widgets)
qt_standard_project_setup()

# libavm-retom: installed package, else a local checkout next to this repo
# (-DAVM_RETOM_LOCAL_DIR=...), else fetched from git like avm-tester does.
find_package(libavm-retom QUIET)
if(libavm-retom_FOUND)
  message(STATUS "Found package libavm-retom")
  # The installed package exports libavm-retom::libavm-retom; the rest of this file links the plain name.
  if(NOT TARGET libavm-retom AND TARGET libavm-retom::libavm-retom)
    add_library(libavm-retom ALIAS libavm-retom::libavm-retom)
  endif()
else()
  set(AVM_RETOM_LOCAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../libavm-retom" CACHE PATH "Local libavm-retom checkout")
  if(EXISTS "${AVM_RETOM_LOCAL_DIR}/CMakeLists.txt")
    message(STATUS "Using local libavm-retom: ${AVM_RETOM_LOCAL_DIR}")
    set(RETOM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(RETOM_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    add_subdirectory("${AVM_RETOM_LOCAL_DIR}" libavm-retom EXCLUDE_FROM_ALL)
  else()
    include(retom)
  endif()
endif()

# libavm-vectorwidget (vector diagram): same lookup order.
find_package(libavm-vectorwidget QUIET)
if(libavm-vectorwidget_FOUND)
  message(STATUS "Found package libavm-vectorwidget")
else()
  set(AVM_VECTORWIDGET_LOCAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../libavm-vectorwidget" CACHE PATH "Local libavm-vectorwidget checkout")
  if(EXISTS "${AVM_VECTORWIDGET_LOCAL_DIR}/CMakeLists.txt")
    message(STATUS "Using local libavm-vectorwidget: ${AVM_VECTORWIDGET_LOCAL_DIR}")
    add_subdirectory("${AVM_VECTORWIDGET_LOCAL_DIR}" libavm-vectorwidget EXCLUDE_FROM_ALL)
  else()
    include(vectorwidget)
  endif()
endif()

qt_add_executable(avm-retom
    src/main.cpp
    src/MainWindow.h
    src/MainWindow.cpp
    src/RetomWorker.h
    src/RetomWorker.cpp
    src/RockerSwitch.h
    src/RockerSwitch.cpp
)

target_link_libraries(avm-retom PRIVATE Qt6::Core Qt6::SerialPort Qt6::Widgets libavm-retom libavm-vectorwidget)

# --- Install & packaging (CPack: DEB / RPM on Linux; the Windows installer is built by packaging/windows) ---
include(GNUInstallDirs)
install(TARGETS avm-retom RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
if(UNIX AND NOT APPLE)
  install(FILES packaging/linux/avm-retom.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
endif()

# CI passes -DAVM_RETOM_VERSION=<tag or tag-less build version>; defaults to the project version.
set(AVM_RETOM_VERSION "${PROJECT_VERSION}" CACHE STRING "Version stamped on the packages")
set(CPACK_PACKAGE_NAME "avm-retom")
set(CPACK_PACKAGE_VERSION "${AVM_RETOM_VERSION}")
set(CPACK_PACKAGE_VENDOR "AVM-Energo LLC")
set(CPACK_PACKAGE_CONTACT "AVM-Energo LLC <info@avmenergo.ru>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "GUI client for the RETOM-51 test bench")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://git.avmenergo.ru/avm-energo/avm-retom")
set(CPACK_PACKAGE_FILE_NAME "avm-retom-${AVM_RETOM_VERSION}-${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_DEBIAN_PACKAGE_SECTION "electronics")
set(CPACK_RPM_PACKAGE_LICENSE "Proprietary")
set(CPACK_RPM_PACKAGE_GROUP "Applications/Engineering")
set(CPACK_RPM_FILE_NAME RPM-DEFAULT)
set(CPACK_RPM_PACKAGE_AUTOREQ ON)
include(CPack)
