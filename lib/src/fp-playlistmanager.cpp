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
    int id;
    QString playlistId;
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
    QString id;
    QList<PlaylistGame> games;
    QString title;
    QString description;
    QString author;

    QX_JSON_STRUCT(
        id,
        games,
        title,
        description,
        author
    );
};

}

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
    mFolder.setNameFilters({"*.json"});
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
            return je;

        // Convert to FP item
        Playlist::Builder pb;
        pb.wId(jPlaylist.id)
          .wTitle(jPlaylist.title)
          .wDescription(jPlaylist.description)
          .wAuthor(jPlaylist.author);

        for(const Json::PlaylistGame& jPlaylistGame : qAsConst(jPlaylist.games))
        {
            PlaylistGame::Builder pgb;
            pgb.wId(jPlaylistGame.id)
               .wPlaylistId(jPlaylistGame.playlistId)
               .wOrder(jPlaylistGame.order)
               .wGameId(jPlaylistGame.gameId);

            pb.wPlaylistGame(pgb.build());
        }

        mPlaylists.append(pb.build());
    }

    return Qx::Error();
}

const QList<Fp::Playlist> PlaylistManager::playlists() const { return mPlaylists; }

}
