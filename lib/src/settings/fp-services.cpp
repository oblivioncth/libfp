// Unit Includes
#include "fp/settings/fp-services.h"

// Qx Includes
#include <qx/core/qx-json.h>

// Json struct parsing implementation

// Configure key generator for mappable types
namespace QxJson
{

template<>
QString keygen<QString, Fp::ServerDaemon>(const Fp::ServerDaemon& value)
{
    return value.name;
};

}

// Json struct parsing implementation
QX_JSON_STRUCT_OUTSIDE(Fp::ServerDaemon,
    name,
    aliases,
    path,
    filename,
    arguments,
    kill
);

QX_JSON_MEMBER_OVERRIDE(Fp::ServerDaemon, aliases,
    static Qx::JsonError fromJson(std::optional<QStringList>& aliases, const QJsonValue& jv)
    {
        /* This requires manual conversion because we always want the result to be an optional
         * list, but sometimes the aliases entry in JSON is a single string instead of an array
         */


        // TODO: Maybe make Qx default conversions not in a private namespace and then document how to call them
        // specificially so they an be used as a fallback, or make public wrappers that call the private method
        if(jv.isArray())
        {
            QStringList aliasList;
            auto err = QxJsonPrivate::standardParse<QStringList>(aliasList, jv);
            aliases = aliasList;
            return err;
        }
        else if(jv.isString())
        {
            QString alias;
            auto err = QxJsonPrivate::standardParse<QString>(alias, jv);
            aliases = !alias.isNull() ?  QStringList({alias}) : QStringList();
            return err;
        }
        else // TODO: Make private accessor to type conversion error
        {
            return Qx::JsonError(QxJsonPrivate::ERR_CONV_TYPE.arg(QxJsonPrivate::typeString<QJsonArray>()), Qx::JsonError::TypeMismatch)
            .withContext(QxJson::Array());
        }
    }

    static QJsonArray toJson(const std::optional<QStringList>& aliases)
    {
        Q_UNUSED(aliases);
        qFatal("Never used.");
        return {};
    }
)

QX_JSON_STRUCT_OUTSIDE(Fp::StartStop,
    path,
    filename,
    arguments
);

QX_JSON_STRUCT_OUTSIDE(Fp::Services,
    server,
    daemon,
    start,
    stop,
);

namespace Fp
{
//===============================================================================================================
// StartStop
//===============================================================================================================

//-Operators----------------------------------------------------------------------------------------------------
//Public:
bool operator== (const StartStop& lhs, const StartStop& rhs) noexcept
{
    return lhs.path == rhs.path && lhs.filename == rhs.filename && lhs.arguments == rhs.arguments;
}

//-Hashing------------------------------------------------------------------------------------------------------
size_t qHash(const StartStop& key, size_t seed) noexcept
{
    QtPrivate::QHashCombine hash;
    seed = hash(seed, key.path);
    seed = hash(seed, key.filename);
    seed = hash(seed, key.arguments);

    return seed;
}

//===============================================================================================================
// ServicesReader
//===============================================================================================================

//-Constructor--------------------------------------------------------------------------------------------------------
//Public:
ServicesReader::ServicesReader(Services* targetServices, std::shared_ptr<QFile> sourceJsonFile, const MacroResolver* macroResolver) :
    SettingsReader(targetServices, sourceJsonFile),
    mHostMacroResolver(macroResolver)
{}

//-Instance Functions-------------------------------------------------------------------------------------------------
//Private:
void ServicesReader::resolveMacros(QString& str) { mHostMacroResolver->resolve(str); }

void ServicesReader::resolveMacros(QStringList& args)
{
    for(auto& str : args)
        resolveMacros(str);
}

template<ServicesT T>
void ServicesReader::resolveMacros(T& st)
{
    resolveMacros(st.path);
    resolveMacros(st.arguments);
}

template<ServicesContainerT T>
void ServicesReader::resolveMacros(T& sct)
{
    for(auto& st : sct)
    {
        /* NOTE: This ugly hack is to drop the const from QSet element references as you can
         * only get a const& to its elements; however, we know for sure that the type of
         * element within the set is not a const type, which means const_cast is a safe,
         * albeit dirty way, to modify items within the set. This can be avoided if the
         * Services sets are just changed to lists, which may be best in the end.
         */
        using MutR = std::add_lvalue_reference_t<std::remove_const_t<std::remove_reference_t<decltype(st)>>>;
        resolveMacros(const_cast<MutR>(st));
    }
}

void ServicesReader::resolveMacros(Fp::Services& s)
{
    resolveMacros(s.server);
    resolveMacros(s.daemon);
    resolveMacros(s.start);
    resolveMacros(s.stop);
}

Qx::JsonError ServicesReader::parseDocument(const QJsonDocument& servicesDoc)
{
    // Get derivation specific target
    Services* targetServices = static_cast<Services*>(mTargetSettings);

    // Parse
    if(Qx::JsonError err = Qx::parseJson(*targetServices, servicesDoc); err.isValid())
        return err;

    // Perform macro substitution
    resolveMacros(*targetServices);

    return Qx::JsonError();
}

}
