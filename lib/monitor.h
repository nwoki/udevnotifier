#ifndef UDEVNOTIFIER_MONITOR_H
#define UDEVNOTIFIER_MONITOR_H

#include "device.h"
#include "udevnotifier_global.h"


namespace UdevNotifier {

class UDEVNOTIFIERSHARED_EXPORT Monitor: public Device
{
public:
    Monitor(udev_device *device = nullptr);
    Monitor(const QString &outputName);
    ~Monitor();
};

}

#endif // UDEVNOTIFIER_MONITOR_H
