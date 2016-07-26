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
  /// Update period in ms
  const static uint16_t PERIOD  = 75;
  /// Convolution filter kernel size in samples
  const static uint16_t firSize = 128;

public:
  /** Constructs a Pulse instance and tries to connect to the pulse oximeter hardware. */
  explicit Pulse(bool swapChannels=false, QObject *parent = 0);
  /** Destructor. */
  ~Pulse();

  /** Returns @c true if the pulse oximeter was found. */
  bool isConnected() const;
  /** (Re-) connects to the pulse oximeter device. */
  bool reconnect();
  /** Starts a periodic measurement (period is defined in @c PERIOD in ms. */
  void start();
  /** Stops the periodic measurement. */
  void stop();

  /** Returns the time (in minutes) since the start of the measurement. */
  double t() const;
  /** Returns the current IR intensity level. */
  double ir() const;
  /** Returns the current mean IR intensity level (DC component). */
  double irMean() const;
  /** Returns the current IR intensity deviation from the mean level (AC component). */
  double irPulse() const;
  /** Returns the current amplitude if the IR intensity deviation (AC component). */
  double irStd() const;
  /** Returns the current RED intensity level. */
  double red() const;
  /** Returns the current mean RED intensity level (DC component). */
  double redMean() const;
  /** Returns the current RED intensity deviation from the mean level (AC component). */
  double redPulse() const;
  /** Returns the current amplitude if the RED intensity deviation (AC component). */
  double redStd() const;
  /** Returns the current estimate of the SpO2 level in percent. */
  double SpO2() const;
  /** Returns the current estimate of the pulse rate in BPM. */
  double pulse() const;

  /** Starts data logging to the given filename. */
  bool logTo(const QString &filename);
  /** Stops data logging. */
  void closeLog();

  /** (Re-)Sets if IR and RED channels are swaped. */
  void setSwapChannels(bool swap);

signals:
  /** Gets emitted if the connection to the pulse oximeter is lost. */
  void connectionLost();
  /** Gets emitted if a measurement is complete. */
  void measurement();
  /** Gets emitted on every detected heartbeat. */
  void pulseEvent();

protected slots:
  /** Updates the estimates. */
  void updateMeasurement();
  /** Starts the measurments. */
  bool startMeasurement();
  /** Reads the current IR and RED value from the pulse oximeter. */
  bool readMeasurement(double &base, double &ir, double &red);

protected:
  /** Saves the current measurements and estimates to the log file (if one is set). */
  void _logValues();

protected:
  /** The USB context. */
  libusb_context        *_usbctx;
  /** The USB device of the pulse oximeter. */
  libusb_device_handle  *_device;
  /** If @c true, the device has been found and connected to. */
  bool _connected;

  /** Update timer. */
  QTimer _timer;
  /** Start time. */
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

  bool _swapChannels;
};


#endif // PULSE_H
