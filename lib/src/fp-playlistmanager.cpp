// Unit Includes
#include "fp/fp-playlistmanager.h"

// Qt Includes
#include <QDirIterator>

// Qx Includes
#include <qx/core/qx-json.h>
#include <qx/io/qx-common-io.h>

namespace Json
{

struct PlaylistGame
{
    std::optional<int> id;
    std::optional<QString> playlistId;
    int order;
    QString gameId;

    QX_JSON_STRUCT(
        id,
        playlistId,
        order,
        gameId
    );
};

struct Playlist
{
    static inline const QString ERR_ICON_PARSE = u"Failed to parse Flashpoint playlist icon."_s;
    static inline const QString ICON_REGEX_FORMAT_G = u"format"_s;
    static inline const QString ICON_REGEX_DATA_G = u"data"_s;
    static inline const QRegularExpression ICON_REGEX = QRegularExpression(u"^data:image\\/(?<format>.+);base64,(?<data>.+)"_s);

    QString id;
    QList<PlaylistGame> games;
    QString title;
    QString description;
    QString author;
    QImage icon;
    QString library;

    QX_JSON_STRUCT(
        id,
        games,
        title,
        description,
        author,
        icon,
        library
    );
};

}

QX_JSON_MEMBER_OVERRIDE(Json::Playlist, icon,
    static Qx::JsonError fromJson(QImage& member, const QJsonValue& jv)
    {
        // Get string data
        if(!jv.isString())
            return Qx::JsonError(Json::Playlist::ERR_ICON_PARSE, Qx::JsonError::TypeMismatch);

        QString inlineImageUri = jv.toString();

        // Get Base64 data
        static Qx::JsonError convErr(Json::Playlist::ERR_ICON_PARSE, Qx::JsonError::InvalidValue);

        QRegularExpressionMatch rm = Json::Playlist::ICON_REGEX.match(inlineImageUri);
        if(!rm.hasMatch())
            return convErr;

        QString format = rm.captured(Json::Playlist::ICON_REGEX_FORMAT_G).toUpper();
        if(format.isEmpty())
            return convErr;

        QString base64 = rm.captured(Json::Playlist::ICON_REGEX_DATA_G);
        if(format.isEmpty())
            return convErr;

        // Convert to binary data
        QByteArray imageData = QByteArray::fromBase64(base64.toLatin1(), QByteArray::Base64Encoding | QByteArray::AbortOnBase64DecodingErrors);
        if(imageData.isEmpty())
            return convErr;

        // Load image
        member = QImage::fromData(imageData); // NOTE: For relying on format auto-detection as the explicit format is known to have been mismatched

        return member.isNull() ? convErr : Qx::JsonError();
    }
)

namespace Fp
{

//===============================================================================================================
// PlaylistManager
//===============================================================================================================

//-Constructor------------------------------------------------------------------------------------------------
//Public:
PlaylistManager::PlaylistManager(const QDir& folder, const Key&) :
    mPopulated(false),
    mFolder(folder)
{
    mFolder.setNameFilters({u"*.json"_s});
    mFolder.setFilter(QDir::Files);
}

//-Instance Functions------------------------------------------------------------------------------------------------
//Public:
bool PlaylistManager::isPopulated() const { return mPopulated; }

Qx::Error PlaylistManager::populate()
{
    if(mPopulated)
        return Qx::Error();

    mPopulated = true;

    // Parse each JSON format playlist
    QDirIterator playlistItr(mFolder);
    while (playlistItr.hasNext())
    {
        QFile playlistFile(playlistItr.next());

        // Read raw data
        QByteArray playlistData;
        if(Qx::IoOpReport rr = Qx::readBytesFromFile(playlistData, playlistFile); rr.isFailure())
            return rr;

        // Parse to JSON
        QJsonParseError parseError;
        QJsonDocument playlistDoc = QJsonDocument::fromJson(playlistData, &parseError);
        if(parseError.error != QJsonParseError::NoError)
            return parseError;

        // Parse to known JSON structure
        Json::Playlist jPlaylist;
        if(Qx::JsonError je = Qx::parseJson(jPlaylist, playlistDoc); je.isValid())
            return je.withContext(QxJson::File(playlistFile));

        // Convert to FP item
        Playlist::Builder pb;
        pb.wId(jPlaylist.id)
          .wTitle(jPlaylist.title)
          .wDescription(jPlaylist.description)
          .wAuthor(jPlaylist.author)
          .wLibrary(jPlaylist.library)
          .wIcon(jPlaylist.icon);

        for(const Json::PlaylistGame& jPlaylistGame : qAsConst(jPlaylist.games))
        {
            PlaylistGame::Builder pgb;
            pgb.wId(jPlaylistGame.id)
               .wPlaylistId(jPlaylistGame.playlistId ? jPlaylistGame.playlistId.value() : QString())
               .wOrder(jPlaylistGame.order)
               .wGameId(jPlaylistGame.gameId);

            pb.wPlaylistGame(pgb.build());
        }

        mTitles.append(jPlaylist.title);
        mPlaylists.append(pb.build());
    }

    return Qx::Error();
}

QList<Fp::Playlist> PlaylistManager::playlists() const { return mPlaylists; }
QStringList PlaylistManager::playlistTitles() const { return mTitles; }
}
