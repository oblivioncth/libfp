// Unit Includes
#include "fp/fp-toolkit.h"

// Standard Library Includes
#include <ranges>

// Qx Includes
#include <qx/utility/qx-helpers.h>

// Project Includes
#include "fp/fp-install.h"

namespace Fp
{

//===============================================================================================================
// Toolkit
//===============================================================================================================

//-Constructor------------------------------------------------------------------------------------------------
//Public:
Toolkit::Toolkit(Install& install, const Key&) :
    mInstall(install)
{
    // Setup (if this class grows expansive enough, these should be shifted towards RAII instead of doing it all at construction
    const Preferences& p = mInstall.preferences();

    mEntryLocalLogoTemplate = mInstall.mEntryLogosDirectory.absoluteFilePath(u"%1"_s + IMAGE_EXT);
    mEntryLocalScreenshotTemplate = mInstall.mEntryScreenshotsDirectory.absoluteFilePath(u"%1"_s + IMAGE_EXT);
    mEntryRemoteLogoTemplate = p.onDemandBaseUrl + mInstall.LOGOS_FOLDER_NAME + u"/%1"_s + IMAGE_EXT;
    mEntryRemoteScreenshotTemplate = p.onDemandBaseUrl + mInstall.SCREENSHOTS_FOLDER_NAME + u"/%1"_s + IMAGE_EXT;
    if(!p.onDemandImagesCompressed.has_value() || p.onDemandImagesCompressed.value())
    {
        mEntryRemoteLogoTemplate += IMAGE_COMPRESSED_URL_SUFFIX;
        mEntryRemoteScreenshotTemplate += IMAGE_COMPRESSED_URL_SUFFIX;
    }
    mDatapackLocalDir = mInstall.mRootDirectory.absoluteFilePath(p.dataPacksFolderPath);
    if(p.gameDataSources && !p.gameDataSources->isEmpty())
    {
        Q_ASSERT(p.gameDataSources->contains(mInstall.MAIN_DATAPACK_SOURCE));
        mDatapackRemoteBase = p.gameDataSources->value(mInstall.MAIN_DATAPACK_SOURCE).arguments.value(0);

        if(mDatapackRemoteBase.back() == '/')
            mDatapackRemoteBase.chop(1);
    }
}

//-Class Functions------------------------------------------------------------------------------------------------
//Private:
QString Toolkit::standardImageSubPath(QUuid gameId)
{
    QString gameIdString = gameId.toString(QUuid::WithoutBraces);
    return gameIdString.left(2) + '/' + gameIdString.mid(2, 2) + '/' + gameIdString;
}

QString Toolkit::datapackFilename(const Fp::GameData& gameData) { return gameData.gameId().toString(QUuid::WithoutBraces) + '-' + QString::number(gameData.dateAdded().toMSecsSinceEpoch()) + u".zip"_s; }

//Public:
Qx::Error Toolkit::appInvolvesSecurePlayer(bool& involvesBuffer, QFileInfo appInfo)
{
    // Reset buffer
    involvesBuffer = false;

    if(appInfo.fileName().contains(SECURE_PLAYER_INFO.baseName()))
    {
        involvesBuffer = true;
        return Qx::Error();
    }
    else if(appInfo.suffix().compare(u"bat"_s, Qt::CaseInsensitive) == 0)
    {
        // Check if bat uses secure player
        QFile batFile(appInfo.absoluteFilePath());
        Qx::IoOpReport readReport = Qx::fileContainsString(involvesBuffer, batFile, SECURE_PLAYER_INFO.baseName());

        // Check for read errors
        if(readReport.isFailure())
            return Qx::Error(readReport).setSeverity(Qx::Critical);
        else
            return Qx::Error();
    }
    else
        return Qx::Error();
}

//-Instance Functions------------------------------------------------------------------------------------------------
//Public:
std::optional<ServerDaemon> Toolkit::getServer(QString server) const
{
    if(server.isEmpty())
        server = mInstall.mPreferences.server;

    auto servers = mInstall.mServices.server;
    if(servers.contains(server))
        return servers[server];

    for(const auto& s : std::as_const(servers))
        if(s.aliases && s.aliases.value().contains(server))
            return s;

    return std::nullopt;
}

QString Toolkit::platformLogoPath(const QString& platform) const
{
    QString path = mInstall.platformLogosDirectory().absoluteFilePath(platform + IMAGE_EXT);
    return QFile::exists(path) ? path : QString();
}

QString Toolkit::entryImageLocalPath(ImageType imageType, const QUuid& gameId) const
{
    // Defaults to using compression if the setting isn't present
    const QString& t = imageType == ImageType::Logo ? mEntryLocalLogoTemplate : mEntryLocalScreenshotTemplate;
    return t.arg(standardImageSubPath(gameId));
}

QUrl Toolkit::entryImageRemotePath(ImageType imageType, const QUuid& gameId) const
{
    const QString& t = (imageType == ImageType::Logo ? mEntryRemoteLogoTemplate : mEntryRemoteScreenshotTemplate);
    return QUrl(t.arg(standardImageSubPath(gameId)));
}


bool Toolkit::resolveAppPathOverrides(QString& appPath) const
{
    // Check if path has an associated override
    for(const AppPathOverride& override : std::as_const(mInstall.preferences().appPathOverrides))
    {
        if(override.path == appPath && override.enabled)
        {
            appPath = override.override;
            return true;
        }
    }

    return false;
}

bool Toolkit::resolveExecSwaps(QString& appPath, const QString& platform) const
{
    // Get swap preference
    bool preferNative = mInstall.preferences().nativePlatforms.contains(platform);

    // Check if path has an associated swap
    for(const Exec& swap : std::as_const(mInstall.execs().execs))
    {
        if(swap.win32 == appPath)
        {
            if(preferNative && !swap.linux.isEmpty())
            {
                appPath = swap.linux;
                return true;
            }
            else if(swap.wine.has_value() && !swap.wine->isEmpty())
            {
                appPath = swap.wine.value();
                return true;
            }
        }
    }

    return false;
}

bool Toolkit::resolveTrueAppPath(QString& appPath, const QString& platform, QHash<QString, QString> overrides, bool absolute) const
{
    // If appPath is absolute, convert it to relative temporarily
    QString fpPath = mInstall.mRootDirectory.absolutePath();
    bool isFpAbsolute = appPath.startsWith(fpPath);
    if(isFpAbsolute)
    {
        // Remove FP root and separator
        appPath.remove(fpPath);
        if(!appPath.isEmpty() && (appPath.front() == '/' || appPath.front() == '\\'))
            appPath = appPath.mid(1);
    }

    // Handle overrides
    for(auto [key, value] : overrides.asKeyValueRange())
    {
        if(appPath == key)
        {
            appPath = value;
            break;
        }
    }

    // Resolve both swap types
    bool swapped = resolveExecSwaps(appPath, platform) || resolveAppPathOverrides(appPath);

    // Rebuild full path if applicable or requested
    if(isFpAbsolute || absolute)
        appPath = fpPath + '/' + appPath;

    // Convert Windows separators to universal '/'
    appPath.replace('\\','/');

    return swapped;
}

bool Toolkit::canDownloadDatapacks() const { return !mDatapackRemoteBase.isEmpty(); }

QString Toolkit::datapackPath(const Fp::GameData& gameData) const { return mDatapackLocalDir.absoluteFilePath(datapackFilename(gameData)); }

QUrl Toolkit::datapackUrl(const Fp::GameData& gameData) const { return canDownloadDatapacks() ? mDatapackRemoteBase + '/' + datapackFilename(gameData) : QUrl(); }

bool Toolkit::datapackIsPresent(const Fp::GameData& gameData) const
{
    // Get current file checksum if it exists
    QFile packFile(datapackPath(gameData));
    bool checksumMatches = false;

    if(!gameData.presentOnDisk() || !packFile.exists())
        return false;

    // Checking the sum in addition to the flag is somewhat overkill, but may help in situations
    // where the flag is set but the datapack's contents have changed
    Qx::IoOpReport checksumReport = Qx::fileMatchesChecksum(checksumMatches, packFile, gameData.sha256(), QCryptographicHash::Sha256);
    if(checksumReport.isFailure() || !checksumMatches)
    {
        qWarning("Existing datapack checksum did not match the expected value");
        return false;
    }

    return true;
}

bool Toolkit::entryIsExtreme(const QUuid& id) const
{
    /* Going from an ID here to the Entry version, which just goes back to
     * an ID seems wasteful, but we don't know if `id` belongs to a game or
     * additional app, and in the case of an additional app we'd need to
     * retreive it's data anyway to get `parentGameId`, so this is really only
     * slightly wasteful in the case where `id` is a valid game id.
     */

    // Maybe this should return an error, but for now a warning is fine if the.
    Entry e;
    if(auto err = mInstall.database()->getEntry(e, id); err)
    {
        qWarning("could not check if %s is extreme, entry not found", qPrintable(id.toString()));
        return false;
    }

    return entryIsExtreme(e);
}

bool Toolkit::entryIsExtreme(const Entry& entry) const
{
    QUuid id = entry.visit(qxFuncAggregate{
        [](const Game& g) { return g.id(); },
        [](const AddApp& aa) { return aa.parentGameId(); }
    });

    // Maybe this should return an error, but for now a warning is fine if the.
    GameTags gt;
    if(auto err = mInstall.database()->getGameTags(gt, id); err)
    {
        qWarning("could not check if %s is extreme, entry tags not found", qPrintable(id.toString()));
        return false;
    }
    QStringList tags = gt.tags();

    auto& tagFilters = mInstall.preferences().tagFilters;

// Clang++15 does not support range adapters (i.e. which make Qt containers work with ranges)
#if defined(__clang__) && __clang_major__ <= 15
    return std::ranges::any_of(tags, [&](const QString& tag) {
        const auto ttag = tag.trimmed();
        for(const auto& tf : tagFilters)
        {
            if(!tf.extreme)
                continue;

            for(const QString& extremeTag : tf.tags)
                if(extremeTag == ttag)
                    return true;
        }

        return false;
    });

#else
    auto extremeTagsView =
        //tagFilters |
        std::ranges::subrange(tagFilters.begin(), tagFilters.end()) |
        std::views::filter([](const auto& tf) { return tf.extreme; }) |
        std::views::transform([](const auto& tf) -> const QStringList& {
            return tf.tags;
        }) |
        std::views::join;

    /* TODO: Perhaps the tags in GameTags should be pre-trimmed during construction, but for now we just
     * trim here as the vanilia launcher does in order to prevent any unexpeted edge cases where needing
     * to match tags exactly (including errant whitespace) matters.
     */
    return std::ranges::any_of(tags, [&](const QString& tag){
        const auto ttag = tag.trimmed();
        // TODO: C++23, replace with std::ranges::contains()
        return std::ranges::find(extremeTagsView, ttag) != std::ranges::end(extremeTagsView);
    });
#endif
}



}
