#include "app.h"
#include <QHash>

uint qHash(const App& app, uint seed)
{
    return qHash(app.source.path, seed) ^ qHash(app.version.toString(), seed)
        ^ qHash(app.containerName, seed) ^ qHash(static_cast<int>(app.state), seed)
        ^ qHash(app.name, seed);
}
