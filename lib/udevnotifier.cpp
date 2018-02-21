#include "device.h"
#include "udevnotifier.h"
#include "udevnotifier_p.h"

#include <libudev.h>
#include <monitor.h>
#include <poll.h>
#include"webcam.h"
#include <QtCore/QDebug>

namespace UdevNotifier {

UdevNotifier::UdevNotifier(const QStringList &groups, QObject *parent)
    : QThread(parent)
    , d(new UdevNotifierPrivate)
{
    // register types needed for signals and slots
    qRegisterMetaType<UdevNotifier::Action>("UdevNotifier::Action");
    qRegisterMetaType<Device>("Device");

    d->groups = groups;
    d->udev = udev_new();

    if (!d->udev) {
        printf("Can't create udev\n");
        exit(1);
    }

    // TODO group monitoring goes here
    d->udevMonitor = udev_monitor_new_from_netlink(d->udev, "udev");

    if (d->udevMonitor) {
        // start receiving events
        udev_monitor_enable_receiving(d->udevMonitor);
    } else {
        qDebug("FAILED UDEv MONITOR");
    }

    d->display = XOpenDisplay(NULL);
    d->window = RootWindow(d->display, DefaultScreen(d->display));
    d->screenRes = XRRGetScreenResources(d->display, d->window);

    int errorBaseIgnored = 0;
    XRRQueryExtension(d->display, &d->xrandrEventBase, &errorBaseIgnored);

    XRRSelectInput(d->display, DefaultRootWindow(d->display), RROutputChangeNotifyMask);
}

UdevNotifier::~UdevNotifier()
{
    XRRFreeScreenResources(d->screenRes);
    XCloseDisplay(d->display);
    delete d;
}

UdevNotifier::Action UdevNotifier::actionFromString(const QString &actionStr)
{
    qDebug("[UdevNotifier::actionFromString]");
    qDebug() << actionStr;


    if (actionStr == QLatin1String("add")) {
        return ADD;
    } else if (actionStr == QLatin1String("remove")) {
        return REMOVE;
    } else {
        // not handled at the moment
        return NONE;
    }
}

void UdevNotifier::scan()
{
    struct udev *udev = d->udev;
    struct udev_enumerate *enumerate = udev_enumerate_new(udev);

    // XXX --- old udev display detection code
    // udev_enumerate_add_match_subsystem(enumerate, "drm");
    // udev_enumerate_add_match_sysname(enumerate, "card[0-9]-*");
    // udev_enumerate_scan_devices(enumerate);

    // struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);
    // struct udev_list_entry *dev_list_entry;
    // struct udev_device *dev;

    // udev_list_entry_foreach(dev_list_entry, devices) {
    //     const char *path;

    //     path = udev_list_entry_get_name(dev_list_entry);
    //     dev = udev_device_new_from_syspath(udev, path);

    //     if (!dev) {
    //         continue;
    //     }

    //     qDebug("[UdevNotifier::scan] Found display: %s", udev_device_get_sysname(dev));
    //     qDebug("[UdevNotifier::scan] enabled? %s", udev_device_get_sysattr_value(dev, "enabled"));

    //     if (QString(udev_device_get_sysattr_value(dev, "enabled")) == QLatin1String("enabled")) {
    //         Q_EMIT udevEvent(ADD, new Monitor(dev));
    //     }

    //     udev_device_unref(dev);
    // }

    // udev_enumerate_unref(enumerate);

    // enumerate = udev_enumerate_new(udev);
    // XXX --- old udev display detection code

    udev_enumerate_add_match_subsystem(enumerate, "video4linux");
    udev_enumerate_scan_devices(enumerate);

    struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);
    struct udev_list_entry *dev_list_entry;
    struct udev_device *dev;

    udev_list_entry_foreach(dev_list_entry, devices) {
        const char *path;

        path = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(udev, path);

        if (!dev) {
            continue;
        }

        qDebug("[UdevNotifier::scan] Found webcam: %s", udev_device_get_sysattr_value(dev, "name"));
        qDebug("[UdevNotifier::scan] node %s", udev_device_get_devnode(dev));

        Q_EMIT udevEvent(ADD, new Webcam(dev));

        udev_device_unref(dev);
    }

    for (int i = 0; i < d->screenRes->noutput; ++i) {
        XRROutputInfo *outputInfo = XRRGetOutputInfo(d->display, d->screenRes, d->screenRes->outputs[i]);

        qDebug() << "[UdevNotifier::scan] found output: " << outputInfo->name;
        if (outputInfo->connection == 0) {
            qDebug() << "[UdevNotifier::scan] output connected, sending signal...";
            Q_EMIT udevEvent(ADD, new Monitor(outputInfo->name));
        }

        XRRFreeOutputInfo(outputInfo);
    }

    udev_enumerate_unref(enumerate);
}

void UdevNotifier::run()
{
    qDebug("[UdevNotifier::run]");
    d->exit = false;

    while (!d->exit) {
       // create the poll item
        pollfd items[1];
        items[0].fd = udev_monitor_get_fd(d->udevMonitor);
        items[0].events = POLLIN;
        items[0].revents = 0;

        // while there are hotplug events to process
        while (poll(items, 1, 50) > 0) {
            // XXX
          //  qDebug() << "hotplug[ " << items[0].revents << " ]";

            // receive the relevant device
            udev_device* dev = udev_monitor_receive_device(d->udevMonitor);
            if (!dev) {
                // error receiving device, skip it
                qDebug("error rcv device. Skip");
                continue;
            }

            //qDebug() << "hotplug[" << udev_device_get_action(dev) << "] " << udev_device_get_devnode(dev) << "," << udev_device_get_subsystem(dev) << "," << udev_device_get_devtype(dev);
            // emit the found device
            // if (udev_device_get_devtype(dev) == QLatin1String("drm_minor")) {
            //     Q_EMIT udevEvent(actionFromString(udev_device_get_action(dev)), new Monitor(dev));
            // }

            if (udev_device_get_subsystem(dev) == QLatin1String("video4linux")) {
                Q_EMIT udevEvent(actionFromString(udev_device_get_action(dev)), new Webcam(dev));
            }
            // XXX
         //   qDebug("-> done");

            // clear the revents
            items[0].revents = 0;
        }

        while (XPending(d->display)) {
            XEvent e;
            XNextEvent(d->display, &e);

            if (e.type - d->xrandrEventBase == RRNotify) {
                qDebug() << "[UdevNotifier] output change event";

                XRROutputChangeNotifyEvent *ev = (XRROutputChangeNotifyEvent*)&e;
                XRROutputInfo *outputInfo = XRRGetOutputInfo(d->display, d->screenRes, ev->output);
                if (outputInfo) {
                    qDebug() << "[UdevNotifier] output:" << outputInfo->name;
                    qDebug() << "[UdevNotifier] output status:" << (outputInfo->connection == 0 ? "connected"
                                                                                                : outputInfo->connection == 1 ? "disconnected"
                                                                                                                              : "unknown");

                    UdevNotifier::Action action = outputInfo->connection == 0 ? ADD : REMOVE;
                    Q_EMIT udevEvent(action, new Monitor(outputInfo->name));

                    XRRFreeOutputInfo(outputInfo);
                }
            }
        }
    }

  //  qDebug("-> OUT");
}

void UdevNotifier::stop()
{
    d->exit = true;
}

}
