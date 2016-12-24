#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include "pinmapping.h"

#include "clickencoder.h"
#include "usbdrv/usbdrv.h"

// ************************
// *** USB HID ROUTINES ***
// ************************

// From Frank Zhao's USB Business Card project
// http://www.frank-zhao.com/cache/usbbusinesscard_details.php
PROGMEM const char
    usbHidReportDescriptor[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] = {
        0x05, 0x01, // USAGE_PAGE (Generic Desktop)
        0x09, 0x06, // USAGE (Keyboard)
        0xa1, 0x01, // COLLECTION (Application)
        0x75, 0x01, //   REPORT_SIZE (1)
        0x95, 0x08, //   REPORT_COUNT (8)
        0x05, 0x07, //   USAGE_PAGE (Keyboard)(Key Codes)
        0x19, 0xe0, //   USAGE_MINIMUM (Keyboard LeftControl)(224)
        0x29, 0xe7, //   USAGE_MAXIMUM (Keyboard Right GUI)(231)
        0x15, 0x00, //   LOGICAL_MINIMUM (0)
        0x25, 0x01, //   LOGICAL_MAXIMUM (1)
        0x81, 0x02, //   INPUT (Data,Var,Abs) ; Modifier byte
        0x95, 0x01, //   REPORT_COUNT (1)
        0x75, 0x08, //   REPORT_SIZE (8)
        0x81, 0x03, //   INPUT (Cnst,Var,Abs) ; Reserved byte
        0x95, 0x05, //   REPORT_COUNT (5)
        0x75, 0x01, //   REPORT_SIZE (1)
        0x05, 0x08, //   USAGE_PAGE (LEDs)
        0x19, 0x01, //   USAGE_MINIMUM (Num Lock)
        0x29, 0x05, //   USAGE_MAXIMUM (Kana)
        0x91, 0x02, //   OUTPUT (Data,Var,Abs) ; LED report
        0x95, 0x01, //   REPORT_COUNT (1)
        0x75, 0x03, //   REPORT_SIZE (3)
        0x91, 0x03, //   OUTPUT (Cnst,Var,Abs) ; LED report padding
        0x95, 0x06, //   REPORT_COUNT (6)
        0x75, 0x08, //   REPORT_SIZE (8)
        0x15, 0x00, //   LOGICAL_MINIMUM (0)
        0x25, 0x65, //   LOGICAL_MAXIMUM (101)
        0x05, 0x07, //   USAGE_PAGE (Keyboard)(Key Codes)
        0x19, 0x00, //   USAGE_MINIMUM (Reserved (no event indicated))(0)
        0x29, 0x65, //   USAGE_MAXIMUM (Keyboard Application)(101)
        0x81, 0x00, //   INPUT (Data,Ary,Abs)
        0xc0        // END_COLLECTION
};

typedef struct {
  uint8_t modifier;
  uint8_t reserved;
  uint8_t keycode[6];
} keyboardReport_t;

static keyboardReport_t keyboardReport;  // sent to PC
volatile static uint8_t ledState = 0xFF; // received from PC
static uint8_t idleRate;                 // repeat rate for keyboards

usbMsgLen_t usbFunctionSetup(uchar data[8]) {

  LED_PORT |= 1 << LED_KEY;
  usbRequest_t *rq = (void *)data;

  if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) {
    switch (rq->bRequest) {
    case USBRQ_HID_GET_REPORT: // send "no keys pressed" if asked here
      // wValue: ReportType (highbyte), ReportID (lowbyte)
      usbMsgPtr = (void *)&keyboardReport; // we only have this one
      keyboardReport.modifier = 0;
      keyboardReport.keycode[0] = 0;
      return sizeof(keyboardReport);
    case USBRQ_HID_SET_REPORT: // if wLength == 1, should be LED state
      return (rq->wLength.word == 1) ? USB_NO_MSG : 0;
    case USBRQ_HID_GET_IDLE: // send idle rate to PC as required by spec
      usbMsgPtr = &idleRate;
      return 1;
    case USBRQ_HID_SET_IDLE: // save idle rate as required by spec
      idleRate = rq->wValue.bytes[1];
      return 0;
    }
  }

  return 0; // by default don't return any data
}

usbMsgLen_t usbFunctionWrite(uint8_t *data, uchar len) {
  LED_PORT |= 1 << LED_KEY;
  // nice hack to save code, as it happens the LEDs are incorrect order and at
  // the right offset so we're able to just set the port status directly
  LED_PORT = data[0] & 0x0E;
  ledState = data[0];
  return 1; // Data read, not expecting more
}

void hardwareInit() {
  // set up the LEDs (all outputs), all initially off
  LED_DDR |=
      (1 << LED1) | (1 << LED2) | (1 << LED3) | (1 << LED4) | (1 << LED_KEY);
  LED_PORT &=
      ~((1 << LED1) | (1 << LED2) | (1 << LED3) | (1 << LED4) | (1 << LED_KEY));

  // set up the encoder (all inputs with pullups)
  // ENC_DDR &= ~((1 << ENC_A) | ~(1 << ENC_B) | ~(1 << ENC_BTN));
  // ENC_PORT |= (1 << ENC_A) | (1 << ENC_B) | (1 << ENC_BTN);

  // set up the keyswitch button (input with pullup)
  KEY_DDR &= ~(1 << KEY);
  KEY_PORT |= (1 << KEY);

  // set up the USB lines (inputs without pullups)
  USB_DDR &= ~((1 << USB_N) | (1 << USB_P));
  USB_PORT &= ~((1 << USB_N) | (1 << USB_P));
}

/*void timerInit() {
  // setup Timer 1 to trigger an interrupt every 1ms
  // see http://www.atmel.com/Images/Atmel-2543-AVR-ATtiny2313_Datasheet.pdf (p.
  // 106)
  // Mode 4: CTC
  TCCR1A &= ~((1 << WGM11) | (1 << WGM10));
  TCCR1B |= (1 << WGM12);
  TCCR1B &= ~(1 << WGM13);

  // Prescaler 1/1024
  TCCR1B |= (1 << CS11);

  // enable interrupt in output compare match
  TIMSK |= (1 << OCIE1A);

  // F_CPU = 12MHz
  // F_CPU / Prescaler = 12MHz / 8 = 1.5MHz
  // 1.5 MHz / 1500 = 1kHz
  OCR1A = 1500;
}

// Timer 1 interrupt vector
ISR(TIMER1_COMPA_vect) { encoder_service(); }

*/

// Now only supports letters 'a' to 'z' and 0 (NULL) to clear buttons
void buildReport(uchar send_key) {
  keyboardReport.modifier = 0;

  keyboardReport.modifier = 5;

  if (send_key >= 'a' && send_key <= 'z')
    keyboardReport.keycode[0] = 4 + (send_key - 'a');
  else
    keyboardReport.keycode[0] = 0;
}

int main() {
  hardwareInit();
  // timerInit();
  encoder_init();

  // clear report initially
  for (int i = 0; i < sizeof(keyboardReport); i++) {
    ((uchar *)&keyboardReport)[i] = 0;
  }

  wdt_enable(WDTO_1S); // enable 1s watchdog timer

  // enforce re-enumeration
  usbDeviceDisconnect();
  // wait 500 ms
  for (int i = 0; i < 250; i++) {
    wdt_reset(); // keep the watchdog happy
    _delay_ms(2);
  }
  usbDeviceConnect();

  usbInit();

  TCCR0B |= (1 << CS01); // timer 0 at clk/8 will generate randomness

  sei(); // Enable interrupts after re-enumeration

  while (1) {
    wdt_reset(); // keep the watchdog happy
    usbPoll();
    /*int16_t value = encoder_getValue();
    if (value % 8 == 0) {
      LED_PORT |= 1 << LED1;
    } else {
      LED_PORT &= ~(1 << LED1);
    }*/

    // characters are sent when messageState == STATE_SEND and after receiving
    // the initial LED state from PC (good way to wait until device is
    // recognized)
    /*if (usbInterruptIsReady()) {
      switch (state) {
      case STATE_SEND_KEY:

        state = STATE_RELEASE_KEY; // release next
        break;
      case STATE_RELEASE_KEY:
        buildReport(NULL);
        state = STATE_WAIT; // go back to waiting
        break;
      default:
        state = STATE_WAIT; // should not happen
      }

    // start sending
  }*/
    /*if (usbInterruptIsReady()) {
      buildReport('x');
      usbSetInterrupt((void *)&keyboardReport, sizeof(keyboardReport));
    }*/
  }

  return 0;
}
