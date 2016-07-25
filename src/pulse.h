#ifndef PULSE_H
#define PULSE_H

#include <QObject>
#include <libusb.h>
#include <QTimer>
#include <QDateTime>
#include <QFile>
#include "fir.hh"


/** Implements the communication with the device. */
class Pulse : public QObject
{
  Q_OBJECT
public:
  const static uint16_t firSize = 128;

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
  double irPulse() const;
  double irStd() const;
  double red() const;
  double redMean() const;
  double redPulse() const;
  double redStd() const;
  double SpO2() const;
  double pulse() const;

  bool logTo(const QString &filename);
  void closeLog();

signals:
  void connectionLost();
  void measurement();
  void pulseEvent();

protected slots:
  void updateMeasurement();
  bool startMeasurement();
  bool readMeasurement(double &base, double &ir, double &red);

protected:
  void _logValues();

protected:
  libusb_context        *_usbctx;
  libusb_device_handle  *_device;
  bool _connected;

  QTimer _timer;
  QDateTime _startTime;

  double _t;
  double _base;
  double _ir;
  double _irMean;
  double _irPulse;
  double _irStd;
  double _red;
  double _redMean;
  double _redPulse;
  double _redStd;

  FIR< LowPassKernel<firSize> >  _irDCFilter;
  FIR< BandPassKernel<firSize> > _irACFilter;
  FIR< LowPassKernel<firSize> >  _redDCFilter;
  FIR< BandPassKernel<firSize> > _redACFilter;

  double _SpO2;

  bool   _isFalling;
  double _lastPulse;
  double _pulse;
  double _pulseMean;

  QFile _logFile;
};


#endif // PULSE_H
