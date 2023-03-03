# Add via ob standard library
include(OB/Library)
ob_add_standard_library(${LIB_TARGET_NAME}
    NAMESPACE "${PROJECT_NAMESPACE}"
    ALIAS "${LIB_ALIAS_NAME}"
    EXPORT_HEADER "${PROJECT_NAMESPACE_LC}/fp_export.h"
    HEADERS_API
        COMMON "${PROJECT_NAMESPACE_LC}"
        FILES
            fp-db.h
            fp-install.h
            fp-items.h
            fp-json.h
            fp-macro.h
    IMPLEMENTATION
        fp-db.cpp
        fp-install.cpp
        fp-items.cpp
        fp-json.cpp
        fp-macro.cpp
    LINKS
        PUBLIC
            Qt6::Core
            Qt6::Sql
            Qt6::Gui
            Qx::Core
            Qx::Io
    CONFIG STANDARD
 )
