#ifndef FLASHPOINT_EXECS_H
#define FLASHPOINT_EXECS_H

// Project Includes
#include "fp/settings/fp-settings.h"

/* Remove the ancient built-in 'linux' define to avoid clash with Exec.linux.
 * No one should still be using it anyway and instead using __linux__.
 */
#undef linux

namespace Fp
{

struct FP_FP_EXPORT Exec
{
    QString linux;
    QString win32;
    QString wine;
};

struct FP_FP_EXPORT Execs : public Settings
{
    QList<Exec> list;
};

class FP_FP_EXPORT ExecsReader : public SettingsReader
{
//-Constructor--------------------------------------------------------------------------------------------------------
public:
    ExecsReader(Execs* targetExecs, std::shared_ptr<QFile> sourceJsonFile);

//-Instance Functions-------------------------------------------------------------------------------------------------
private:
    Qx::GenericError parseDocument(const QJsonDocument& execsDoc);
    Qx::GenericError parseExec(Exec& execBuffer, const QJsonValue& jvExec);
};

}

#endif // FLASHPOINT_EXECS_H
