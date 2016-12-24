
// ***********************************************************
// ***                    USB HID KEY DEFS                 ***
// *** http://www.usb.org/developers/hidpage/Hut1_12v2.pdf ***
// ***********************************************************
#define KEY_ESCAPE 0x29
#define KEY_MUTE 0x7F
#define KEY_VOL_DOWN 0x81
#define KEY_VOL_UP 0x80

// ***********************************************************
// ***                    USB HID LED DEFS                 ***
// *** http://www.usb.org/developers/hidpage/Hut1_12v2.pdf ***
// ***********************************************************
#define NUM_LOCK 0x01
#define CAPS_LOCK 0x02
#define SCROLL_LOCK 0x04

// ***********************************************************
// ***                      PIN MAPPING                    ***
// ***********************************************************
#define LED1 PB1
#define LED2 PB2
#define LED3 PB3
#define LED4 PB4
#define LED_KEY PB7
#define LED_DDR DDRB
#define LED_PORT PORTB

#define ENC_A PB6
#define ENC_B PB5
#define ENC_BTN PB7
#define ENC_DDR DDRB
#define ENC_PIN PINB
#define ENC_PORT PORTB

#define KEY PD0
#define KEY_DDR DDRD
#define KEY_PIN PIND
#define KEY_PORT PORTD

#define USB_DDR DDRD
#define USB_PORT PORTD
#define USB_PIN PIND
#define USB_P PD2
#define USB_N PD3
