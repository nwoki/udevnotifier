#ifndef WEBCAM_H
#define WEBCAM_H


#include "device.h"

#include <libudev.h>

#include <QtCore/QString>


namespace UdevNotifier {

class UDEVNOTIFIERSHARED_EXPORT Webcam: public Device
{
public:
    Webcam(udev_device *device = nullptr);
    ~Webcam();
};

}
#endif // WEBCAM_H
