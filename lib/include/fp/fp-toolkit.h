#ifndef FLASHPOINT_TOOLKIT_H
#define FLASHPOINT_TOOLKIT_H

// Shared Lib Support
#include "fp/fp_export.h"

// Qt Includes
#include <QHash>
#include <QFileInfo>
#include <QDir>

// Qx Includes
#include <qx/core/qx-error.h>

// Project Includes
#include "fp/fp-items.h"
#include "fp/settings/fp-services.h"

using namespace Qt::Literals::StringLiterals;

namespace Fp
{

class Install;

class FP_FP_EXPORT Toolkit
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
    // File Info
    static inline const QString IMAGE_EXT = u".png"_s;
    static inline const QString IMAGE_COMPRESSED_URL_SUFFIX = u"?type=jpg"_s;

public:
    static inline const QFileInfo SECURE_PLAYER_INFO = QFileInfo(u"FlashpointSecurePlayer.exe"_s);

//-Instance Variables-----------------------------------------------------------------------------------------------
private:
    const Install& mInstall;

    // Implementation Details
    QString mEntryLocalLogoTemplate;
    QString mEntryLocalScreenshotTemplate;
    QString mEntryRemoteLogoTemplate;
    QString mEntryRemoteScreenshotTemplate;
    QDir mDatapackLocalDir;
    QString mDatapackRemoteBase;

//-Constructor-------------------------------------------------------------------------------------------------
public:
    Toolkit(const Install& install, const Key&);

 //-Class Functions-----------------------------------------------------------------------------------------------
private:
    static QString standardImageSubPath(QUuid gameId);

public:
    static Qx::Error appInvolvesSecurePlayer(bool& involvesBuffer, QFileInfo appInfo);

//-Instance Functions------------------------------------------------------------------------------------------------------
private:
    QString entryLocalLogoPath();

public:
    // Config
    std::optional<ServerDaemon> getServer(QString server = {}) const;

    // Images
    QString platformLogoPath(const QString& platform) const;
    QString entryImageLocalPath(ImageType imageType, const QUuid& gameId) const;
    QUrl entryImageRemotePath(ImageType imageType, const QUuid& gameId) const;

    // App paths
    bool resolveAppPathOverrides(QString& appPath) const;
    bool resolveExecSwaps(QString& appPath, const QString& platform) const;
    bool resolveTrueAppPath(QString& appPath, const QString& platform, QHash<QString, QString> overrides = {}, bool absolute = false) const;

    // Datapacks
    QString datapackPath(const Fp::GameData& gameData) const;
    QUrl datapackUrl(const Fp::GameData& gameData) const;
    bool datapackIsPresent(const Fp::GameData& gameData) const;

};

}

#endif // FLASHPOINT_TOOLKIT_H
