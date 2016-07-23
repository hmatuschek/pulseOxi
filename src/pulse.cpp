#include "pulse.h"
#include <qDebug>
#include <cmath>

#include "../firmware/proto.h"      /* custom request numbers */
#include "../firmware/usbconfig.h"  /* device's VID/PID and names */

#define PERIOD  75
#define THETA   (double(PERIOD)/1e3)


Pulse::Pulse(QObject *parent)
  : QObject(parent), _usbctx(0), _device(0)
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
Pulse::redStd() const {
  return _redStd;
}

double
Pulse::SpO2() const {
  return _SpO2;
}

double
Pulse::SpO2Mean() const {
  return _SpO2Mean;
}

double
Pulse::pulse() const {
  return _pulse;
}

double
Pulse::pulseMean() const {
  return _pulseMean;
}

void
Pulse::start() {
  _baseMean = _irMean = _redMean = 0;;
  _irStd = _redStd = 0;
  _t = _pulseMean = _dIR = _dRed = 0;
  _SpO2Mean = 90;

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
  base = (0xffff-double(msg.base))/0xffff;
  ir   = (0xffff-double(msg.upper))/0xffff;
  red  = (0xffff-double(msg.lower))/0xffff;
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

    // Detect pulse
    bool isPulse = (_dIR > 0) && (_dRed > 0);
    _dIR = _ir-_irMean; _dRed = _red-_redMean;
    isPulse &= (_dIR < 0) && (_dRed < 0);
    double f = 1./(_t - _lastPulse);
    if (isPulse && (f<180)) {
      _pulse = f;
      _lastPulse = _t;
    }
    _pulseMean = (1.-THETA)*_pulseMean + THETA*_pulse;

    _irStd  = (1.-THETA)*_irStd + THETA*std::abs(_ir-_irMean);
    _irMean = (1.-THETA)*_irMean + THETA*_ir;
    _redStd = (1.-THETA)*_redStd + THETA*std::abs(_red-_redMean);
    _redMean = (1.-THETA)*_redMean + THETA*_red;

    double r = (_redStd*_irMean)/(_irStd*_redMean);
    _SpO2 = 100*(-r/3 + 3.4/3);
    _SpO2Mean = (1.-THETA)*_SpO2Mean + THETA*_SpO2;

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
