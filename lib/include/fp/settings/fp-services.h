#ifndef FLASHPOINT_SERVICES_H
#define FLASHPOINT_SERVICES_H

// Qt Includes
#include <QSet>

// Qx Includes
#include <qx/utility/qx-concepts.h>

// Project Includes
#include "fp/settings/fp-settings.h"
#include "fp/fp-macro.h"

namespace Fp
{
//-Structs--------------------------------------------------------------
struct FP_FP_EXPORT ServerDaemon
{
    QString name;
    std::optional<QStringList> aliases;
    QString path;
    QString filename;
    QStringList arguments;
    bool kill;
};

struct FP_FP_EXPORT StartStop
{
    QString path;
    QString filename;
    QStringList arguments;

    friend bool operator== (const StartStop& lhs, const StartStop& rhs) noexcept;
    friend size_t qHash(const StartStop& key, size_t seed) noexcept;
};

struct FP_FP_EXPORT Services : public Settings
{
    //QSet<Watch> watches;
    QHash<QString, ServerDaemon> server;
    QHash<QString, ServerDaemon> daemon;
    QSet<StartStop> start;
    QSet<StartStop> stop;
    // TODO: ^If Settings container obj is made (see other todo), move this there
};

//-Concepts--------------------------------------------------------------
template<typename T>
concept ServicesT = Qx::any_of<T, ServerDaemon, StartStop>;

template<typename T>
concept ServicesContainerT = Qx::any_of<T, QHash<QString, ServerDaemon>, QSet<StartStop>>;

class FP_FP_EXPORT ServicesReader : public SettingsReader
{
//-Instance Variables--------------------------------------------------------------------------------------------------
private:
    const MacroResolver* mHostMacroResolver;

//-Constructor--------------------------------------------------------------------------------------------------------
public:
    ServicesReader(Services* targetServices, std::shared_ptr<QFile> sourceJsonFile, const MacroResolver* macroResolver);

//-Instance Functions-------------------------------------------------------------------------------------------------
private:
    void resolveMacros(QString& str);
    void resolveMacros(QStringList& args);
    template<ServicesT T>
    void resolveMacros(T& st);
    template<ServicesContainerT T>
    void resolveMacros(T& sct);
    void resolveMacros(Fp::Services& s);

    Qx::JsonError parseDocument(const QJsonDocument& servicesDoc);
};

}

#endif // FLASHPOINT_SERVICES_H
