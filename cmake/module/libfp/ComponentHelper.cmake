macro(register_libfp_component)
    #================= Setup ==========================
    # Utility functions
    include(libfp/Utility)

    # Determine component name via folder name
    get_filename_component(COMPONENT_NAME_LC "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
    string_to_proper_case(${COMPONENT_NAME_LC} COMPONENT_NAME)
    create_header_guard(${PROJECT_NAME} ${COMPONENT_NAME} COMPONENT_HEADER_GUARD)

    # Name here needs to be as unique as possible for when this project is inlcuded
    # in another via FetchContent or add_subdirectory (prevent target clashes)
    set(COMPONENT_TARGET_NAME ${PROJECT_NAME}_${COMPONENT_NAME_LC})

    # Make lib target
    qt_add_library(${COMPONENT_TARGET_NAME} ${COMPONENT_LIB_TYPE})

    # Make alias target so target can be referred to with its friendly
    # export name both internally and when part of another build tree
    add_library(${PROJECT_NAMESPACE}::${COMPONENT_NAME} ALIAS ${COMPONENT_TARGET_NAME})

    #================= Build ==========================

    # Get timestamp
    string(TIMESTAMP BUILD_DATE_TIME "%Y-%m-%d @ %H:%M:%S ")

    # Source Files
    if(COMPONENT_PRIVATE_HEADERS)
        foreach(p_header ${COMPONENT_PRIVATE_HEADERS})
            target_sources(${COMPONENT_TARGET_NAME} PRIVATE "src/${p_header}")
        endforeach()
    endif()

    if(COMPONENT_IMPL)
        foreach(impl ${COMPONENT_IMPL})
            target_sources(${COMPONENT_TARGET_NAME} PRIVATE "src/${impl}")
        endforeach()
    endif()

    # Includes
    if(${COMPONENT_LIB_TYPE} STREQUAL "INTERFACE")
        target_include_directories(${COMPONENT_TARGET_NAME} INTERFACE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
            $<INSTALL_INTERFACE:include/${COMPONENT_NAME_LC}>
        )
    else()
        target_include_directories(${COMPONENT_TARGET_NAME} PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
            $<INSTALL_INTERFACE:include/${COMPONENT_NAME_LC}>
        )
    endif()

    if(COMPONENT_INCLUDE_HEADERS)
        # Build pathed include file list
        foreach(api_header ${COMPONENT_INCLUDE_HEADERS})
            set(pathed_api_headers ${pathed_api_headers} "include/${PROJECT_NAMESPACE_LC}/${COMPONENT_NAME_LC}/${api_header}")
        endforeach()

        # Group include files with their parent directories stripped
        source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAMESPACE_LC}/${COMPONENT_NAME_LC}"
            PREFIX "Include Files"
            FILES ${pathed_api_headers}
        )

        # Add include files as private target source so that they aren't built nor marked as a dependency,
        # but are shown with the target in the IDE
        target_sources(${COMPONENT_TARGET_NAME} PRIVATE ${pathed_api_headers})
    endif()

    # Links
    if(COMPONENT_PUBLIC_LINKS)
            target_link_libraries(${COMPONENT_TARGET_NAME} PUBLIC ${COMPONENT_PUBLIC_LINKS})
    endif()

    if(COMPONENT_PRIVATE_LINKS)
            target_link_libraries(${COMPONENT_TARGET_NAME} PRIVATE ${COMPONENT_PRIVATE_LINKS})
    endif()

    #-----------Generate Primary Component Header------------

    # Generate include statements
    foreach(api_header ${COMPONENT_INCLUDE_HEADERS})
        set(PRIM_COMP_HEADER_INCLUDES "${PRIM_COMP_HEADER_INCLUDES}#include <${PROJECT_NAMESPACE_LC}/${COMPONENT_NAME_LC}/${api_header}>\n")
    endforeach()

    # Copy template with modifications
    configure_file(
        "${FILE_TEMPLATES_PATH}/primary_component_header.h.in"
        "${CMAKE_CURRENT_BINARY_DIR}/include/${PROJECT_NAMESPACE_LC}/${COMPONENT_NAME_LC}.h"
        @ONLY
        NEWLINE_STYLE UNIX
    )

    #--------------------Package Config-----------------------

    # Create config file
    configure_file("${FILE_TEMPLATES_PATH}/${PROJECT_NAME_LC}-component-config.cmake.in"
        "${PROJECT_BINARY_DIR}/cmake/${COMPONENT_NAME_LC}/${PROJECT_NAME_LC}-${COMPONENT_NAME_LC}-config.cmake"
        @ONLY
    )

    #---------- Configure Target Properties------------------
    set_target_properties(${COMPONENT_TARGET_NAME} PROPERTIES
        VERSION ${PROJECT_VERSION}
        OUTPUT_NAME "${PROJECT_NAME}-${COMPONENT_NAME_LC}"
        DEBUG_POSTFIX "d"
        EXPORT_NAME "${COMPONENT_NAME}"
    )

    #================= Install ==========================

    # Install component lib
    install(TARGETS ${COMPONENT_TARGET_NAME}
        COMPONENT ${COMPONENT_TARGET_NAME}
        EXPORT ${COMPONENT_NAME_LC}-targets
        ${SUB_PROJ_EXCLUDE_FROM_ALL} # "EXCLUDE_FROM_ALL" if project is not top-level
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin # For potential future shared version
    )

    # Install public headers
    install(DIRECTORY include/${PROJECT_NAMESPACE_LC}
        COMPONENT ${COMPONENT_TARGET_NAME}
        DESTINATION "include/${COMPONENT_NAME_LC}"
        ${SUB_PROJ_EXCLUDE_FROM_ALL} # "EXCLUDE_FROM_ALL" if project is not top-level
    )
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/include/${PROJECT_NAMESPACE_LC}/${COMPONENT_NAME_LC}.h"
        COMPONENT ${COMPONENT_TARGET_NAME}
        DESTINATION "${HEADER_INSTALL_SUFFIX}/${COMPONENT_NAME_LC}/${PROJECT_NAMESPACE_LC}"
        ${SUB_PROJ_EXCLUDE_FROM_ALL} # "EXCLUDE_FROM_ALL" if project is not top-level
    )

    # Install package target export
    install(EXPORT ${COMPONENT_NAME_LC}-targets
        COMPONENT ${COMPONENT_TARGET_NAME}
        FILE "${PROJECT_NAME}-${COMPONENT_NAME_LC}-targets.cmake"
        NAMESPACE ${PROJECT_NAMESPACE}::
        DESTINATION cmake/${COMPONENT_NAME_LC}
        ${SUB_PROJ_EXCLUDE_FROM_ALL} # "EXCLUDE_FROM_ALL" if project is not top-level
    )

    # Install package config
    install(FILES
        "${PROJECT_BINARY_DIR}/cmake/${COMPONENT_NAME_LC}/${PROJECT_NAME_LC}-${COMPONENT_NAME_LC}-config.cmake"
        COMPONENT ${COMPONENT_TARGET_NAME}
        DESTINATION cmake/${COMPONENT_NAME_LC}
        ${SUB_PROJ_EXCLUDE_FROM_ALL} # "EXCLUDE_FROM_ALL" if project is not top-level
    )

    #========Export For In-tree Builds =================
    # For in source builds
    export(EXPORT ${COMPONENT_NAME_LC}-targets
        FILE "${PROJECT_BINARY_DIR}/cmake/${COMPONENT_NAME_LC}/${PROJECT_NAME_LC}-${COMPONENT_NAME_LC}-targets.cmake"
        NAMESPACE ${PROJECT_NAME}::
    )
endmacro()
