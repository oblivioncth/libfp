#================= Project Setup ==========================

# CMake
cmake_minimum_required(VERSION 3.23.0...3.26.0)

# Project
# NOTE: DON'T USE TRAILING ZEROS IN VERSIONS
project(libfp
    VERSION 0.5.2
    LANGUAGES CXX
    DESCRIPTION "C++ support library for Flashpoint Archive"
)

# Get helper scripts
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/FetchOBCMake.cmake)
fetch_ob_cmake("v0.3.4")

# Initialize project according to standard rules
include(OB/Project)
ob_standard_project_setup(NAMESPACE "Fp")

# Configuration options
option(BUILD_SHARED_LIBS "Build shared libraries." OFF) # Redundant due to OB, but explicit

# C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build augmentation
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

#================= Build =========================

# Import Qt
set(LIBFP_QT_COMPONENTS
    Core
    Sql
    Gui
)

add_compile_definitions(QT_DISABLE_DEPRECATED_BEFORE=0x060000)
include(OB/BetterFindQt6)
ob_find_qt6_package(REQUIRED COMPONENTS ${LIBFP_QT_COMPONENTS})

# Fetch Qx (build and import from source)
set(LIBFP_QX_COMPONENTS
    Core
    Io
)

include(OB/FetchQx)
ob_fetch_qx(
    REF "v0.5.6"
    COMPONENTS
        ${LIBFP_QX_COMPONENTS}
)

# Process Targets
set(LIB_TARGET_NAME ${PROJECT_NAMESPACE_LC}_${PROJECT_NAMESPACE_LC})
set(LIB_ALIAS_NAME ${PROJECT_NAMESPACE})
set(LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib")
add_subdirectory("${LIB_PATH}")

#--------------------Package Config-----------------------

ob_standard_project_package_config(
    COMPATIBILITY "SameMinorVersion"
    CONFIG STANDARD
        TARGET_CONFIGS
            TARGET "${PROJECT_NAMESPACE}::${LIB_ALIAS_NAME}" COMPONENT "${LIB_ALIAS_NAME}" DEFAULT
        DEPENDS
            PACKAGE "Qt6" COMPONENTS ${LIBFP_QT_COMPONENTS}
            PACKAGE "Qx" VERSION ${Qx_VERSION} COMPONENTS ${LIBFP_QX_COMPONENTS}
)

#================= Top Level Install =======================

ob_standard_project_misc_install()

#====================== CPack ==============================

include(OB/Packaging)
ob_standard_project_package(VENDOR "oblivioncth")

