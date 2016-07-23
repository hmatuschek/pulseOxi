#ifndef PULSE_H
#define PULSE_H

#include <QObject>
#include <libusb.h>
#include <QTimer>
#include <QDateTime>


/** Implements the communication with the device. */
class Pulse : public QObject
{
  Q_OBJECT

public:
  explicit Pulse(QObject *parent = 0);
  ~Pulse();

  bool isConnected() const;
  bool reconnect();

  void start();
  void stop();

  double t() const;
  double ir() const;
  double irMean() const;
  double irStd() const;
  double red() const;
  double redMean() const;
  double redStd() const;
  double SpO2() const;
  double SpO2Mean() const;
  double pulse() const;
  double pulseMean() const;

signals:
  void connectionLost();
  void measurement();

protected slots:
  void updateMeasurement();
  bool startMeasurement();
  bool readMeasurement(double &base, double &ir, double &red);

protected:
  libusb_context        *_usbctx;
  libusb_device_handle  *_device;
  bool _connected;

  QTimer _timer;
  QDateTime _startTime;

  double _t;
  double _base;
  double _baseMean;
  double _ir;
  double _irMean;
  double _irStd;
  double _red;
  double _redMean;
  double _redStd;

  double _SpO2;
  double _SpO2Mean;

  double _dIR, _dRed;
  double _lastPulse;
  double _pulse;
  double _pulseMean;
};


#endif // PULSE_H
