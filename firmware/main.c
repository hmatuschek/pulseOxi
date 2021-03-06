#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
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
#define PORT_REG_NAME(port)  PULSE_CONCAT(PORT, port)

#define LED1_DDR_REG        DDR_REG_NAME(PULSE_LED1_PORT)
#define LED1_PORT_REG       PORT_REG_NAME(PULSE_LED1_PORT)

#define LED2_DDR_REG        DDR_REG_NAME(PULSE_LED2_PORT)
#define LED2_PORT_REG       PORT_REG_NAME(PULSE_LED2_PORT)

#define ADC_DDR_REG         DDR_REG_NAME(PULSE_ADC_PORT)
#define ADC_PORT_REG        PORT_REG_NAME(PULSE_ADC_PORT)
#define ADC_MUX             (PULSE_ADC_CHANNEL & 0x0f)

#define NOP                 asm volatile ("nop")

// ==============================================================================
// Globals
// ------------------------------------------------------------------------------

static Message usb_reply;
static uint16_t currentSum = 0;
static uint8_t  sampleCount = 0;

typedef enum {
  MODE_IDLE,
  MODE_START,
  MODE_LED1,
  MODE_LED2,
  MODE_BASE,
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
    // If there is some data and CMB is GET
    usbMsgLen_t len = sizeof(usb_reply);
    if (len > rq->wLength.word)
      len = rq->wLength.word;
    usbMsgPtr = (void *) &usb_reply;
    mode = MODE_IDLE;
    return len;
  } else if ((MODE_IDLE == mode) && (rq->bRequest == PULSE_CMD_START)) {
    // If idle and CMD is START
    mode = MODE_START;
  }
  return 0;
}


/* ********************************************************************************************* *
 * Hardware setup
 * ********************************************************************************************* */
static void
hardwareInit(void) {
  // Init USB interface
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

  // init state machine in "idle"
  mode = MODE_IDLE;
}


/* ********************************************************************************************* *
 * Measurement loop
 * ********************************************************************************************* */
static void
adcPoll() {
  // Dispatch by mode
  if (MODE_START == mode) {
    // enable LED1 & disable LED2
    LED1_PORT_REG |= (1<<PULSE_LED1_PIN);
    LED2_PORT_REG &= ~(1<<PULSE_LED2_PIN);
    NOP;
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
      // disable LED1 & enable LED2
      LED1_PORT_REG &= ~(1<<PULSE_LED1_PIN);
      LED2_PORT_REG |= (1<<PULSE_LED2_PIN);
      NOP;
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
      // disable LED 1 & 2
      LED1_PORT_REG &= ~(1<<PULSE_LED1_PIN);
      LED2_PORT_REG &= ~(1<<PULSE_LED2_PIN);
      NOP;
      // update mode, done.
      mode = MODE_BASE;
      // start converion
      ADCSRA |= (1<<ADSC);
    }
  } else if ((MODE_BASE == mode) && (0 == (ADCSRA & (1<<ADSC)))) {
    // add result to sum
    currentSum += ADC;
    sampleCount++;
    if (64 > sampleCount) {
      // start converion
      ADCSRA |= (1<<ADSC);
    } else {
      // store 16 bit value
      usb_reply.base = currentSum;
      // reset counter
      currentSum = 0; sampleCount = 0;
      // update mode
      mode = MODE_READY;
    }
  }
}


/* ------------------------------------------------------------------------- */
/* ------------------------ Oscillator Calibration ------------------------- */
/* ------------------------------------------------------------------------- */

/* Calibrate the RC oscillator to 8.25 MHz. The core clock of 16.5 MHz is
 * derived from the 66 MHz peripheral clock by dividing. Our timing reference
 * is the Start Of Frame signal (a single SE0 bit) available immediately after
 * a USB RESET. We first do a binary search for the OSCCAL value and then
 * optimize this value with a neighboorhod search.
 * This algorithm may also be used to calibrate the RC oscillator directly to
 * 12 MHz (no PLL involved, can therefore be used on almost ALL AVRs), but this
 * is wide outside the spec for the OSCCAL value and the required precision for
 * the 12 MHz clock! Use the RC oscillator calibrated to 12 MHz for
 * experimental purposes only!
 */
static void
calibrateOscillator(void) {
  uchar       step = 128;
  uchar       trialValue = 0, optimumValue;
  int         x, optimumDev, targetValue = (unsigned)(1499 * (double)F_CPU / 10.5e6 + 0.5);

  /* do a binary search: */
  do{
    OSCCAL = trialValue + step;
    x = usbMeasureFrameLength();    /* proportional to current real frequency */
    if(x < targetValue)             /* frequency still too low */
      trialValue += step;
    step >>= 1;
  }while(step > 0);
  /* We have a precision of +/- 1 for optimum OSCCAL here */
  /* now do a neighborhood search for optimum value */
  optimumValue = trialValue;
  optimumDev = x; /* this is certainly far away from optimum */
  for(OSCCAL = trialValue - 1; OSCCAL <= trialValue + 1; OSCCAL++){
    x = usbMeasureFrameLength() - targetValue;
    if(x < 0)
      x = -x;
    if(x < optimumDev){
      optimumDev = x;
      optimumValue = OSCCAL;
    }
  }
  OSCCAL = optimumValue;
}

void
usbEventResetReady(void) {
  /* Disable interrupts during oscillator calibration since
   * usbMeasureFrameLength() counts CPU cycles. */
  cli();
  calibrateOscillator();
  sei();
  eeprom_write_byte(0, OSCCAL);   /* store the calibrated value in EEPROM */
}


/* ********************************************************************************************* *
 * main()
 * ********************************************************************************************* */
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
    // handle measurement stuff
    adcPoll();
	}

  return 0;
}
