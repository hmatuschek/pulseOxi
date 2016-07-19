#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#include "usbdrv/usbdrv.h"
#include "proto.h"


#define PULSE_LED1_PORT    B
#define PULSE_LED1_PIN     0

#define PULSE_LED2_PORT    B
#define PULSE_LED2_PIN     3

#define PULSE_ADC_PORT     B
#define PULSE_ADC_PIN      4
#define PULSE_ADC_CHANNEL  2


#define PULSE_CONCAT(a,b)   a ## b
#define DDR_REG_NAME(port)  PULSE_CONCAT(DDR, port)
#define PORT_REG_NAME(port)  PULSE_CONCAT(DDR, port)

#define LED1_DDR_REG        DDR_REG_NAME(PULSE_LED1_PORT)
#define LED1_PORT_REG       PORT_REG_NAME(PULSE_LED1_PORT)

#define LED2_DDR_REG        DDR_REG_NAME(PULSE_LED2_PORT)
#define LED2_PORT_REG       PORT_REG_NAME(PULSE_LED2_PORT)

#define ADC_DDR_REG         DDR_REG_NAME(PULSE_ADC_PORT)
#define ADC_PORT_REG        PORT_REG_NAME(PULSE_ADC_PORT)
#define ADC_MUX             (PULSE_ADC_CHANNEL & 0x0f)


// ==============================================================================
// Globals
// ------------------------------------------------------------------------------

static Message usb_reply;

typedef enum {
  MODE_IDLE,
  MODE_START,
  MODE_LED1,
  MODE_LED2,
  MODE_READY
} Mode;

static Mode mode = MODE_IDLE;

// ------------------------------------------------------------------------------
// - usbFunctionSetup
// - see: http://vusb.wikidot.com/driver-api
// ------------------------------------------------------------------------------
usbMsgLen_t usbFunctionSetup(uchar data[8]) {
  usbRequest_t    *rq = (void *)data;
  // Dispatch by request type
  if(rq->bRequest == PULSE_CMD_GET) {
    if (MODE_READY != mode) {
      return 0;
    }
    usbMsgLen_t len = sizeof(usb_reply);
    if (len > rq->wLength.word)
      len = rq->wLength.word;
    usbMsgPtr = (void *) &usb_reply;
    mode = MODE_IDLE;
    return len;
  } else if((rq->bRequest == PULSE_CMD_START) && (MODE_IDLE == mode)) {
    mode = MODE_START;
  }
  return 0;
}


/*---------------------------------------------------------------------------*/
/* usbConf                                                              */
/*---------------------------------------------------------------------------*/
static void usbHardwareInit(void)
{
  uchar i, j;

	/* activate pull-ups except on USB lines */
  USB_CFG_IOPORT &= (uchar) ~ ((1 << USB_CFG_DMINUS_BIT) | (1 << USB_CFG_DPLUS_BIT));

  /* all pins input except USB (-> USB reset) */
#ifdef USB_CFG_PULLUP_IOPORT	/* use usbDeviceConnect()/usbDeviceDisconnect() if available */
  USBDDR = 0;		/* we do RESET by deactivating pullup */
  usbDeviceDisconnect();
#else
  USBDDR = (1 << USB_CFG_DMINUS_BIT) | (1 << USB_CFG_DPLUS_BIT);
#endif

  j = 0;
  while (--j) {		/* USB Reset by device only required on Watchdog Reset */
    i = 0;
    while (--i);	/* delay >10ms for USB reset */
  }

#ifdef USB_CFG_PULLUP_IOPORT
  usbDeviceConnect();
#else
  USBDDR = 0;		/*  remove USB reset condition */
#endif
}


static void hardwareInit(void) {
  usbHardwareInit();

  // Set LED output
  LED1_DDR_REG |= (1<<PULSE_LED1_PIN);
  LED2_DDR_REG |= (1<<PULSE_LED2_PIN);
  // Shut LEDs off
  LED1_PORT_REG |= (0<<PULSE_LED1_PIN);
  LED2_PORT_REG |= (0<<PULSE_LED2_PIN);
  // Set ADC pin input & disable pull-up
  ADC_DDR_REG &= ~(1<<PULSE_ADC_PIN);
  ADC_PORT_REG &= ~(1<<PULSE_ADC_PIN);

  // Config ADC (2.56V ref, right adjust, selected pin)
  ADMUX |= (1<<REFS2) | (1<<REFS1) | (0<<REFS0) | (0<<ADLAR) | ADC_MUX;
  // config prescaler to 128
  ADCSRA |= (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
  // enable ADC
  ADCSRA |= (1<<ADEN);

  mode = MODE_IDLE;
}

// ==============================================================================
// - main
// ------------------------------------------------------------------------------
int main(void)
{
  hardwareInit();
	usbInit();
	sei();

	for(;;) {
		usbPoll();

    if (MODE_START == mode) {
      mode = MODE_LED1;
      // enable LED1
      LED1_PORT_REG |= (1<<PULSE_LED1_PIN);
      // disable LED2
      LED1_PORT_REG &= ~(1<<PULSE_LED2_PIN);
      // start converion
      ADCSRA |= (1<<ADSC);
    } if ((MODE_LED1 == mode) && (0 == (ADCSRA & (1<<ADSC)))) {
      usb_reply.lower = ADC;
      mode = MODE_LED2;
      // disable LED1
      LED1_PORT_REG &= ~(1<<PULSE_LED1_PIN);
      // enable LED2
      LED1_PORT_REG |= (1<<PULSE_LED2_PIN);
      // start converion
      ADCSRA |= (1<<ADSC);
    } if ((MODE_LED2 == mode) && (0 == (ADCSRA & (1<<ADSC)))) {
      usb_reply.upper = ADC;
      // disable LED1 & 2
      LED1_PORT_REG &= ~(1<<PULSE_LED1_PIN);
      LED1_PORT_REG &= ~(1<<PULSE_LED2_PIN);
      mode = MODE_READY;
    }
	}

  return 0;
}

