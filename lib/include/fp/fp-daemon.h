#ifndef FLASHPOINT_DAEMON_H
#define FLASHPOINT_DAEMON_H

// Qt Includes
#include <QString>

namespace Fp
{

enum Daemon { Unknown, Docker, Qemu, FpProxy };

Daemon daemonFromString(QStringView name);

}
#endif // FLASHPOINT_DAEMON_H
