#include "pulse.h"

Pulse::Pulse(const QString &portname, QObject *parent)
  : QObject(parent), _port(portname)
{
  _port.open(QIODevice::ReadWrite);
}

