#pragma once

#include <QMetaType>
#include <libavm-retom/driver.h>

// Shared by every header whose signals/slots carry a RetomSignalSet (must precede any moc use).
Q_DECLARE_METATYPE(retom::RetomSignalSet)
