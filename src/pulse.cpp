#include "pulse.h"
#include <qDebug>

#include "../firmware/proto.h"      /* custom request numbers */
#include "../firmware/usbconfig.h"  /* device's VID/PID and names */

Pulse::Pulse(QObject *parent)
  : QObject(parent), _usbctx(0), _device(0)
{
  const unsigned char rawVid[2] = {USB_CFG_VENDOR_ID}, rawPid[2] = {USB_CFG_DEVICE_ID};
  //char                vendor[] = {USB_CFG_VENDOR_NAME, 0}, product[] = {USB_CFG_DEVICE_NAME, 0};
  int                 vid, pid;

  libusb_init(&_usbctx);

  /* compute VID/PID from usbconfig.h so that there is a central source of information */
  vid = int(rawVid[1]) * 256 + rawVid[0];
  pid = int(rawPid[1]) * 256 + rawPid[0];

  //_dumpDevices();

  // Get device list
  libusb_device **devices;
  libusb_get_device_list(_usbctx, &devices);

  // Try to open device (identified by vendor & device id)
  _device = libusb_open_device_with_vid_pid(_usbctx, vid, pid);
  if (0 == _device) {
    qDebug() << "Cannot open device for vid=" << vid << ", pid=" << pid;
    return;
  }

  // Free list (not needed anymore)
  libusb_free_device_list(devices, 1);

  if (0 > libusb_claim_interface(_device, 0)) {
    qDebug() << "Cannot claim interface.";
    return;
  }

  // done.
  _connected = true;
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

bool
Pulse::startMeasurement() {
  if (! isConnected())
    return false;

  uint8_t request_type = LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_OUT;
  if (0 > libusb_control_transfer(_device, request_type, PULSE_CMD_START, 0, 0, 0, 0, 100)){
    qDebug() << "Failed to send START_MEASUREMENT...";
    return false;
  }
  return true;
}

bool
Pulse::readMeasurement(double &value) {
  if (! isConnected())
    return false;

  Message msg;
  uint8_t request_type = LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_IN;
  if (sizeof(Message) != libusb_control_transfer(_device, request_type, PULSE_CMD_GET, 0, 0,
                                                 (unsigned char *)&msg, sizeof(Message), 100))
  {
    qDebug() << "Got invalid or incomplete response.";
    return false;
  }
  qDebug() << "Got " << msg.lower << ", " << msg.upper;
  value = (double(msg.upper)-msg.lower)/(double(msg.upper)+msg.lower);
  return true;
}

void
Pulse::_dumpDevices() {
  // Find devices
  libusb_device **devices;
  int ndevs = libusb_get_device_list(_usbctx, &devices);
  for (int i=0; i<ndevs; i++) {
    libusb_device_descriptor desc;
    if (0 > libusb_get_device_descriptor(devices[i], &desc))
      continue;
    qDebug() << "Found class" << (int) desc.bDeviceClass
             << " device" <<  QString("%1").arg(desc.idProduct, 0, 16)
             << "from" << QString("%1").arg(desc.idVendor,0,16)
             << " with" << (int)desc.bNumConfigurations << "configs: ";
    libusb_config_descriptor *config;
    libusb_get_config_descriptor(devices[i], 0, &config);
    qDebug() << " Interfaces:"<< (int)config->bNumInterfaces;
    const libusb_interface *inter;
    const libusb_interface_descriptor *interdesc;
    const libusb_endpoint_descriptor *epdesc;
    for (int j = 0; j<config->bNumInterfaces; j++) {
      inter = &config->interface[i];
      qDebug() << "  Alternate settings:" << inter->num_altsetting;
      for (int k=0; k<inter->num_altsetting; k++) {
        interdesc = &inter->altsetting[k];
        qDebug() << "   Interface Number:"<< (int)interdesc->bInterfaceNumber;
        qDebug() << "   Number of endpoints:"<<(int)interdesc->bNumEndpoints;
        for (int l=0; l<(int)interdesc->bNumEndpoints; l++) {
          epdesc = &interdesc->endpoint[l];
          qDebug() << "    Endpoint:" << l;
          qDebug() << "    Descriptor Type:"<<(int)epdesc->bDescriptorType;
          qDebug() << "    EP Address:"<<(int)epdesc->bEndpointAddress;
        }
      }
    }
    libusb_free_config_descriptor(config);
  }
  libusb_free_device_list(devices, 1);
}
