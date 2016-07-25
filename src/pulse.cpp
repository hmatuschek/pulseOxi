#include "pulse.h"
#include <qDebug>
#include <cmath>
#include <QtEndian>

#include "../firmware/proto.h"      /* custom request numbers */
#include "../firmware/usbconfig.h"  /* device's VID/PID and names */

#define PERIOD   75
#define THETA    (float(PERIOD)/5e3)
#define Fmin     (float(PERIOD*30)/60e3)
#define Fmax     (float(PERIOD*180)/60e3)


Pulse::Pulse(QObject *parent)
  : QObject(parent), _usbctx(0), _device(0), _irDCFilter(LowPassKernel<firSize>(Fmin)),
    _irACFilter(BandPassKernel<firSize>(Fmin, Fmax)), _redDCFilter(LowPassKernel<firSize>(Fmin)),
    _redACFilter(BandPassKernel<firSize>(Fmin, Fmax))
{
  // Init USB context
  libusb_init(&_usbctx);
  _connected = reconnect();

  _timer.setInterval(PERIOD);
  _timer.setSingleShot(false);
  connect(&_timer, SIGNAL(timeout()), this, SLOT(updateMeasurement()));
}


Pulse::~Pulse() {
  if (_device) {
    // If there is a device -> free interface
    libusb_release_interface(_device, 0);
    // destroy device
    libusb_close(_device);
  }
  // If there is a USB context -> free
  if (_usbctx)
    libusb_exit(_usbctx);
  // done.
  _connected = false;
  closeLog();
}

bool
Pulse::isConnected() const {
  return _connected;
}

double
Pulse::t() const {
  return _t;
}

double
Pulse::ir() const {
  return _ir;
}

double
Pulse::irMean() const {
  return _irMean;
}

double
Pulse::irPulse() const {
  return _irPulse;
}

double
Pulse::irStd() const {
  return _irStd;
}

double
Pulse::red() const {
  return _red;
}

double
Pulse::redMean() const {
  return _redMean;
}

double
Pulse::redPulse() const {
  return _redPulse;
}

double
Pulse::redStd() const {
  return _redStd;
}

double
Pulse::SpO2() const {
  return _SpO2;
}

double
Pulse::pulse() const {
  return _pulseMean;
}

void
Pulse::start() {
  _t=0;
  _irMean = _redMean = 0;
  _irPulse = _redPulse = 0;
  _irStd = _redStd = 0;

  _pulseMean = 70;

  _startTime = QDateTime::currentDateTime();
  _timer.start();
}

void
Pulse::stop() {
  _timer.stop();
}

bool
Pulse::startMeasurement() {
  if (! isConnected())
    return false;

  uint8_t request_type = LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_OUT;
  if (0 > libusb_control_transfer(_device, request_type, PULSE_CMD_START, 0, 0, 0, 0, 100)){
    qDebug() << "Failed to send START_MEASUREMENT...";
    // Assume connection loss
    bool was_connected = _connected;
    // close device
    libusb_close(_device); _device = 0; _connected = false;
    // Emit event if device was connected
    if (was_connected)
      emit connectionLost();
    return false;
  }
  return true;
}

bool
Pulse::readMeasurement(double &base, double &ir, double &red) {
  if (! isConnected())
    return false;

  Message msg;
  uint8_t request_type = LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_IN;
  int r = libusb_control_transfer(_device, request_type, PULSE_CMD_GET, 0, 0,
                                  (unsigned char *)&msg, sizeof(Message), 1000);
  if (sizeof(Message) != r) {
    qDebug() << "Got invalid or incomplete response.";
    return false;
  }
  base = (0xffff-double(qFromLittleEndian(msg.base)))/0xffff;
  ir   = (0xffff-double(qFromLittleEndian(msg.upper)))/0xffff;
  red  = (0xffff-double(qFromLittleEndian(msg.lower)))/0xffff;
  return true;
}

void
Pulse::updateMeasurement() {
  if (readMeasurement(_base, _ir, _red)) {
    // Get time point
    _t = _startTime.msecsTo(QDateTime::currentDateTime());
    _t /= 60e3;
    // adjust _ir & _red with base
    _ir -= _base;
    _red -= _base;

    // Detect pulse (last value was positive)
    double lastIrPulse = _irPulse;
    bool wasAboveA = (_irPulse > _irStd/2);
    bool wasAboveB = (_irPulse > -_irStd/2);

    _irMean = _irDCFilter.apply(_ir);
    _irPulse = _irACFilter.apply(_ir);
    _irStd   = (1.-THETA)*_irStd + THETA*std::abs(_irPulse);

    _redMean = _redDCFilter.apply(_red);
    _redPulse = _redACFilter.apply(_red);
    _redStd   = (1.-THETA)*_redStd + THETA*std::abs(_redPulse);

    double r = (_redStd*_irMean)/(_irStd*_redMean);
    if (r < 1)
      _SpO2 = -25*r + 110;
    else
      _SpO2 = -35.4167*r + 120.4167;

    // If last pulse was positive and current negative -> pulse event
    bool isBelowA = (_irPulse <= _irStd/2);
    bool isBelowB = (_irPulse <= -_irStd/2);
    bool isFalling = ((lastIrPulse - _irPulse)>0);
    _isFalling = (wasAboveA && isBelowA) || (_isFalling && isFalling);
    bool isPulse = _isFalling && wasAboveB && isBelowB;
    double f = 1./(_t - _lastPulse);
    if (isPulse) {
      _pulse = f;
      _lastPulse = _t;
      emit pulseEvent();
    }
    _pulseMean = (1.-THETA/2)*_pulseMean + THETA/2*_pulse;

    if (_logFile.isOpen())
      _logValues();

    emit measurement();
  }
  if (! startMeasurement()) {
    if (! isConnected())
      emit connectionLost();
    qDebug() << "Cannot restart measurement.";
  }
}

bool
Pulse::reconnect() {
  /* compute VID/PID from usbconfig.h so that there is a central source of information */
  const uint8_t rawVid[2] = {USB_CFG_VENDOR_ID}, rawPid[2] = {USB_CFG_DEVICE_ID};
  int           vid, pid;
  vid = int(rawVid[1]) * 256 + rawVid[0];
  pid = int(rawPid[1]) * 256 + rawPid[0];

  // Get device list (is this needed to discover the device?)
  libusb_device **devices;
  libusb_get_device_list(_usbctx, &devices);

  // Try to open device (identified by vendor & device id)
  _device = libusb_open_device_with_vid_pid(_usbctx, vid, pid);
  if (0 == _device) {
    qDebug() << "Cannot open device for vid=" << vid << ", pid=" << pid;
    return false;
  }

  // Free list (not needed anymore)
  libusb_free_device_list(devices, 1);

  if (0 > libusb_claim_interface(_device, 0)) {
    qDebug() << "Cannot claim interface.";
    libusb_close(_device); _device=0;
    return false;
  }

  return true;
}


bool
Pulse::logTo(const QString &filename) {
  if (_logFile.isOpen())
    closeLog();
  _logFile.setFileName(filename);
  if (_logFile.open(QIODevice::WriteOnly)) {
    _logFile.write("#PULSE\tSpO2\tIR_RAW\tIR_DC\tIR_AC\tIR_STD\tRED_RAW\tRED_DC\tRED_AC\tRED_STD\n");
  }
  return false;
}

void
Pulse::closeLog() {
  _logFile.close();
}

void
Pulse::_logValues() {
  if (! _logFile.isOpen())
    return;

  _logFile.write(QString::number(_pulseMean).toUtf8()); _logFile.write("\t");
  _logFile.write(QString::number(_SpO2).toUtf8()); _logFile.write("\t");
  _logFile.write(QString::number(_ir).toUtf8()); _logFile.write("\t");
  _logFile.write(QString::number(_irMean).toUtf8()); _logFile.write("\t");
  _logFile.write(QString::number(_irPulse).toUtf8()); _logFile.write("\t");
  _logFile.write(QString::number(_irStd).toUtf8()); _logFile.write("\t");
  _logFile.write(QString::number(_red).toUtf8()); _logFile.write("\t");
  _logFile.write(QString::number(_redMean).toUtf8()); _logFile.write("\t");
  _logFile.write(QString::number(_redPulse).toUtf8()); _logFile.write("\t");
  _logFile.write(QString::number(_redStd).toUtf8()); _logFile.write("\n");
}
