#ifndef FLASHPOINT_MACRO_H
#define FLASHPOINT_MACRO_H

// Shared Lib Support
#include "fp/fp_export.h"

// Qt Includes
#include <QHash>

using namespace Qt::Literals::StringLiterals;

namespace Fp
{

class FP_FP_EXPORT MacroResolver
{
//-Inner Classes----------------------------------------------------------------------------------------------------
public:
    class Key
    {
        friend class Install;
    private:
        Key() {};
        Key(const Key&) = default;
    };

//-Class Variables-----------------------------------------------------------------------------------------------
private:
    static inline const QString FP_PATH = u"<fpPath>"_s;

//-Instance Variables-----------------------------------------------------------------------------------------------
private:
    QHash<QString, QString> mMacroMap; // Will make sense if more macros are added

//-Constructor-------------------------------------------------------------------------------------------------
public:
    MacroResolver(const QString& installPath, const Key&); // Will need to be improved if many more macros are added

//-Instance Functions------------------------------------------------------------------------------------------------------
public:
    QString resolve(const QString& macroStr) const;
};

}

#endif // FLASHPOINT_MACRO_H
