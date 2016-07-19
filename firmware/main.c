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

volatile static Message usb_reply;

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
  if ((MODE_READY == mode) && (rq->bRequest == PULSE_CMD_GET)) {
    usbMsgLen_t len = sizeof(usb_reply);
    if (len > rq->wLength.word)
      len = rq->wLength.word;
    usbMsgPtr = (void *) &usb_reply;
    mode = MODE_IDLE;
    return len;
  } else if ((MODE_IDLE == mode) && (rq->bRequest == PULSE_CMD_START)) {
    mode = MODE_START;
  }
  return 0;
}


/*---------------------------------------------------------------------------*/
/* usbConf                                                              */
/*---------------------------------------------------------------------------*/
static void
usbHardwareInit(void)
{
  uchar i, j;

  /* deactivate pull-ups on USB lines */
  USB_CFG_IOPORT &= ~ ((1 << USB_CFG_DMINUS_BIT) | (1 << USB_CFG_DPLUS_BIT));
  USBDDR |= (1 << USB_CFG_DMINUS_BIT) | (1 << USB_CFG_DPLUS_BIT);

  j = 0;
  while (--j) {		/* USB Reset by device only required on Watchdog Reset */
    i = 0;
    while (--i);	/* delay >10ms for USB reset */
  }

  /*  remove USB reset condition */
  USBDDR &= ~ ( (1 << USB_CFG_DMINUS_BIT) | (1 << USB_CFG_DPLUS_BIT) );
}


static void
hardwareInit(void) {
  // Init USB interface
  usbHardwareInit();

  // Set LED output
  LED1_DDR_REG |= (1<<PULSE_LED1_PIN);
  LED2_DDR_REG |= (1<<PULSE_LED2_PIN);
  // Shut LEDs off
  LED1_PORT_REG &= ~(1<<PULSE_LED1_PIN);
  LED2_PORT_REG &= ~(1<<PULSE_LED2_PIN);
  // Set ADC pin input & disable pull-up
  ADC_DDR_REG &= ~(1<<PULSE_ADC_PIN);
  ADC_PORT_REG &= ~(1<<PULSE_ADC_PIN);

  // Config ADC (2.56V ref, right adjust, selected pin)
  ADMUX |= (1<<REFS2) | (1<<REFS1) | (0<<REFS0) | (0<<ADLAR) | ADC_MUX;
  // config prescaler to 128
  ADCSRA |= (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
  // enable ADC
  ADCSRA |= (1<<ADEN);

  // Config timer0, prescaler 64 -> approx 1kHz
  TCCR0B |= (0<<CS02) | (1<<CS01) | (1<<CS00) ;
  // enable overflow interrupt
  TIMSK |= (1<<TOIE0);

  // init state machine in "idle"
  mode = MODE_IDLE;
}


// ==============================================================================
// - main
// ------------------------------------------------------------------------------
int
main(void)
{
  // Init hardware
  hardwareInit();
  usbInit();
  // Enable interrupts
	sei();

	for(;;) {
    // handle USB stuff
		usbPoll();
	}

  return 0;
}


static uint16_t currentSum = 0;
static uint8_t  sampleCount = 0;

ISR(TIMER0_OVF_vect) {
  // Dispatch by mode
  if (MODE_START == mode) {
    // enable LED1
    LED1_PORT_REG |= (1<<PULSE_LED1_PIN);
    // disable LED2
    LED2_PORT_REG &= ~(1<<PULSE_LED2_PIN);
    // start converion
    ADCSRA |= (1<<ADSC);
    // update mode
    mode = MODE_LED1;
    // reset counter
    currentSum = 0;
    sampleCount = 0;
  } else if ((MODE_LED1 == mode) && (0 == (ADCSRA & (1<<ADSC)))) {
    // add result to sum
    currentSum += ADC;
    sampleCount++;
    if (64 > sampleCount) {
      // start converion
      ADCSRA |= (1<<ADSC);
    } else {
      // store 16 bit value
      usb_reply.lower = currentSum;
      // reset counter
      currentSum = 0; sampleCount = 0;
      // disable LED1
      LED1_PORT_REG &= ~(1<<PULSE_LED1_PIN);
      // enable LED2
      LED2_PORT_REG |= (1<<PULSE_LED2_PIN);
      // start converion
      ADCSRA |= (1<<ADSC);
      // update mode
      mode = MODE_LED2;
    }
  } else if ((MODE_LED2 == mode) && (0 == (ADCSRA & (1<<ADSC)))) {
    // add result to sum
    currentSum += ADC;
    sampleCount++;
    if (64 > sampleCount) {
      // start converion
      ADCSRA |= (1<<ADSC);
    } else {
      // store 16 bit value
      usb_reply.upper = currentSum;
      // reset counter
      currentSum = 0; sampleCount = 0;
      // disable LED1 & 2
      LED1_PORT_REG &= ~(1<<PULSE_LED1_PIN);
      LED2_PORT_REG &= ~(1<<PULSE_LED2_PIN);
      // update mode, done.
      mode = MODE_READY;
    }
  }
}
