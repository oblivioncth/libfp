# Add via ob standard library
include(OB/Library)
ob_add_standard_library(${LIB_TARGET_NAME}
    NAMESPACE "${PROJECT_NAMESPACE}"
    ALIAS "${LIB_ALIAS_NAME}"
    EXPORT_HEADER
        PATH "${PROJECT_NAMESPACE_LC}/fp_export.h"
    HEADERS_API
        COMMON "${PROJECT_NAMESPACE_LC}"
        FILES
            fp-daemon.h
            fp-db.h
            fp-install.h
            fp-items.h
            fp-macro.h
            fp-playlistmanager.h
            fp-toolkit.h
            settings/fp-config.h
            settings/fp-execs.h
            settings/fp-preferences.h
            settings/fp-services.h
            settings/fp-settings.h
    IMPLEMENTATION
        fp-db.cpp
        fp-install.cpp
        fp-macro.cpp
        fp-items.cpp
        fp-playlistmanager.cpp
        fp-toolkit.cpp
        settings/fp-config.cpp
        settings/fp-execs.cpp
        settings/fp-preferences.cpp
        settings/fp-services.cpp
        settings/fp-settings.cpp
    LINKS
        PUBLIC
            Qt6::Core
            Qt6::Sql
            Qt6::Gui
            Qx::Core
            Qx::Io
    CONFIG STANDARD
 )
