#================= Project Setup ==========================

# CMake
cmake_minimum_required(VERSION 3.21.1)

# Project
# NOTE: DON'T USE TRAILING ZEROS IN VERSIONS
project(libfp
    VERSION 0.1.4
    LANGUAGES CXX
    DESCRIPTION "Command-line Interface for Flashpoint"
)
string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LC)
string(TOUPPER ${PROJECT_NAME} PROJECT_NAME_UC)
set(PROJECT_NAMESPACE "Fp")
string(TOLOWER ${PROJECT_NAMESPACE} PROJECT_NAMESPACE_LC)
string(TOUPPER ${PROJECT_NAMESPACE} PROJECT_NAMESPACE_UC)

# C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build augmentation
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Disable deprecated code
add_compile_definitions(QT_DISABLE_DEPRECATED_BEFORE=0x060000)

# Add local modules
set(PROJ_SCRIPTS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${PROJ_SCRIPTS_PATH}/module")

# Get helper scripts
include(libfp/FetchOBCMake)
fetch_ob_cmake("v0.1")

# Include utility functions
include(OB/Utility)

# General Variables
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(TARGET_ARCH x64)
else()
  set(TARGET_ARCH x86)
endif()

#--------------Top Level Project Setup-------------

if(${PROJECT_IS_TOP_LEVEL})
    message(STATUS "NOTE: ${PROJECT_NAME} is being configured as a top-level project")

    # Install (override the CMake default, but not a user set value)
    if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/out/install"
               CACHE PATH "Project install path" FORCE
        )
    endif()

    # Clean install when clean target is ran
    set_directory_properties(PROPERTIES ADDITIONAL_CLEAN_FILES "${CMAKE_INSTALL_PREFIX}")
else()
    message(STATUS "NOTE: ${PROJECT_NAME} is being configured as a sub-project")

    # Keep install components out of 'all' target
    set(SUB_PROJ_EXCLUDE_FROM_ALL "EXCLUDE_FROM_ALL")
endif()


#--------------------Setup Paths-------------------

# Cmake scripts
set(FILE_TEMPLATES_PATH "${PROJ_SCRIPTS_PATH}/file_templates")

# Sub projects
set(COMPONENTS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/comp")

# Package
set(PACKAGE_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/out/dist")

#------------Set Global Build Rules----------------

# Versioning
include(OB/VerboseVersioning)
setup_verbose_versioning(PROJECT_VERSION_VERBOSE)

################# Common Build #################

# Enumerate Components
get_subdirectory_list(${COMPONENTS_PATH} LIB_COMPONENTS)
set(BUILD_COMPONENTS ${LIB_COMPONENTS})

#================= Top Level Build =========================

# Import Qt
include(OB/BetterFindQt6)
find_qt6_package(REQUIRED COMPONENTS
    Core
    Sql
    Gui
)

# Fetch Qx (build and import from source)
include(OB/FetchQx)
fetch_qx("v0.4")

#--------------------Package Config-----------------------

# Create config file
configure_file("${FILE_TEMPLATES_PATH}/${PROJECT_NAME_LC}-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME_LC}-config.cmake"
    @ONLY
)

# Create version config file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME_LC}-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMinorVersion
)

#================= Process Componets =======================

# This lib only has one "component" but this setup is to make
# expanding to more in the future easier

# Build components
foreach(component ${BUILD_COMPONENTS})
    add_subdirectory(${COMPONENTS_PATH}/${component})
endforeach()

#================= Top Level Install =======================

set(TOP_LEVEL_INSTALL_COMPONENT ${PROJECT_NAME_LC})

# Install Package Config
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME_LC}-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME_LC}-config-version.cmake"
    COMPONENT ${TOP_LEVEL_INSTALL_COMPONENT}
    DESTINATION cmake
    ${SUB_PROJ_EXCLUDE_FROM_ALL} # "EXCLUDE_FROM_ALL" if project is not top-level
)

# Install README and LICENSE
install(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
    "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
    COMPONENT ${TOP_LEVEL_INSTALL_COMPONENT}
    DESTINATION .
    ${SUB_PROJ_EXCLUDE_FROM_ALL} # "EXCLUDE_FROM_ALL" if project is not top-level
)

#====================== CPack ==============================

set(CPACK_PACKAGE_VENDOR "oblivioncth")
set(CPACK_PACKAGE_DIRECTORY "${PACKAGE_PREFIX}")
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}_${PROJECT_VERSION_VERBOSE}_(Qt${Qt6_VERSION}-${Qt6_LINKAGE})_${CMAKE_SYSTEM_NAME}_${TARGET_ARCH}")
set(CPACK_GENERATOR "ZIP")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
include(CPack)
