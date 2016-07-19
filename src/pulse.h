#ifndef PULSE_H
#define PULSE_H

#include <QObject>
#include <libusb.h>

class Pulse : public QObject
{
  Q_OBJECT

public:
  explicit Pulse(QObject *parent = 0);
  ~Pulse();

  bool isConnected() const;

  bool startMeasurement();
  bool readMeasurement(double &value);

protected:
  void _dumpDevices();

protected:
  libusb_context        *_usbctx;
  libusb_device_handle  *_device;
  bool _connected;
};

#endif // PULSE_H
