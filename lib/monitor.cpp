#include "monitor.h"
#include "device.h"

namespace UdevNotifier {

Monitor::Monitor(udev_device *device)
    :Device(device, MONITOR)
{
}

Monitor::Monitor(const QString &outputName)
    : Device(outputName, MONITOR)
{
}

Monitor::~Monitor()
{
}

}
