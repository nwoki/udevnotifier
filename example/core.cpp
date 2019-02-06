#include "core.h"

#include <QtCore/QDebug>

#include <UdevNotifier/Device>


Core::Core(QObject *parent)
    : QObject(parent)
    , m_udevNotifier(new UdevNotifier::UdevNotifier(QStringList(), this))
{
    connect(m_udevNotifier, &UdevNotifier::UdevNotifier::udevEvent, this, &Core::onUdevEvent);

    m_udevNotifier->scan();
    m_udevNotifier->start();
}

Core::~Core()
{
}

void Core::onUdevEvent(UdevNotifier::UdevNotifier::Action action, UdevNotifier::Device *device)
{
    qDebug("[Core::onUdevEvent]");

    Q_UNUSED(action);
    Q_UNUSED(device);

    // not used anymore, get rid of it
//    delete device;
}
