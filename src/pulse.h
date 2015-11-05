#ifndef PULSE_H
#define PULSE_H

#include <QObject>
#include <QSerialPort>

class Pulse : public QObject
{
  Q_OBJECT

public:
  explicit Pulse(const QString &portname, QObject *parent = 0);

  bool isConnected() const;

protected:
  QSerialPort _port;
};

#endif // PULSE_H
