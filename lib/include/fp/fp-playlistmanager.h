#ifndef FLASHPOINT_PLAYLISTMANAGER_H
#define FLASHPOINT_PLAYLISTMANAGER_H

// Shared Lib Support
#include "fp/fp_export.h"

// Qt Includes
#include <QDir>

// Qx Includes
#include <qx/core/qx-error.h>

// Project Includes
#include "fp/fp-items.h"

namespace Fp
{

class FP_FP_EXPORT PlaylistManager
{
//-Inner Classes-------------------------------------------------------------------------------------------------
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

//-Instance Variables-----------------------------------------------------------------------------------------------
private:
    bool mPopulated;
    QDir mFolder;
    QList<Fp::Playlist> mPlaylists;

//-Constructor-------------------------------------------------------------------------------------------------
public:
    explicit PlaylistManager(const QDir& folder, const Key&);

//-Instance Functions------------------------------------------------------------------------------------------------------
private:

public:
    bool isPopulated() const;
    Qx::Error populate();
    const QList<Fp::Playlist> playlists() const;
};

}

#endif // FLASHPOINT_PLAYLISTMANAGER_H
