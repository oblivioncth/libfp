#================= Setup Component ==========================

set(COMPONENT_LIB_TYPE STATIC)

################# Common Build #################

# Headers
set(COMPONENT_INCLUDE_HEADERS
    fp-db.h
    fp-install.h
    fp-items.h
    fp-json.h
    fp-macro.h
)

# Source Files
set(COMPONENT_IMPL
    fp-db.cpp
    fp-install.cpp
    fp-items.cpp
    fp-json.cpp
    fp-macro.cpp
)

# Links
set(COMPONENT_PUBLIC_LINKS
    Qt6::Core
    Qt6::Sql
    Qt6::Gui
    Qx::Core
    Qx::Io
)

# Qt Component Dependencies
set(COMPONENT_QT_DEPENDS
    Core
    Sql
    Gui
)

#=============== Component Handler =======================
include(FpComponentHelper)
register_libfp_component()
