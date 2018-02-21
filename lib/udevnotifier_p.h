#ifndef UDEVNOTIFIER_P_H
#define UDEVNOTIFIER_P_H

#include <QtCore/QStringList>


#include <libudev.h>

#include <qtextstream.h> // must be include before headers that define a Status type
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>

namespace UdevNotifier {


/**
 * @class UdevNotifierPrivate
 * @brief The UdevNotifier's private class
 * @author Francesco Nwokeka <francesco.nwokeka@gmail.com>
 */
class UdevNotifierPrivate
{
public:
    UdevNotifierPrivate()
        : fdUdev(-1)
        , exit(true)
        , udev(nullptr)
        , udevMonitor(nullptr)
    {}

    ~UdevNotifierPrivate()
    {
        udev_monitor_unref(udevMonitor);
        udev_unref(udev);
    }


    QStringList groups;

    int fdUdev;

    /** flag that indicates if the thread should be running or not */
    bool exit;

    struct udev *udev;
    struct udev_monitor *udevMonitor;
    Display *display;
    Window window;
    XRRScreenResources* screenRes;
    int xrandrEventBase;
};

}

#endif // UDEVNOTIFIER_P_H
