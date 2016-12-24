// ----------------------------------------------------------------------------
// Rotary Encoder Driver with Acceleration
// Supports Click, DoubleClick, Long Click
//
// (c) 2010 karl@pitrich.com
// (c) 2014 karl@pitrich.com
//
// Timer-based rotary encoder logic by Peter Dannegger
// http://www.mikrocontroller.net/articles/Drehgeber
// ----------------------------------------------------------------------------

#include "clickencoder.h"
#include "pinmapping.h"

int16_t delta;
int16_t last;

void encoder_init() {

  delta = 0;
  last = 0;

  if (ENC_PIN & (1 << ENC_A)) {
    last = 3;
  }

  if (ENC_PIN & (1 << ENC_B)) {
    last ^= 1;
  }
}

int16_t encoder_getValue(void) {
  int8_t curr = 0;

  if (ENC_PIN & (1 << ENC_A)) {
    curr = 3;
  }

  if (ENC_PIN & (1 << ENC_B)) {
    curr ^= 1;
  }

  int8_t diff = last - curr;

  if (diff & 1) // bit 0 = step
  {
    last = curr;
    delta += (diff & 2) - 1; // bit 1 = direction (+/-)
  }
  /*if (steps == 2)
    delta = val & 1;
  else if (steps == 4)*/
  delta = delta & 3;
  /*else
    delta = 0; // default to 1 step per notch


  /*if (steps == 4)*/
  delta >>= 2;
  /*if (steps == 2)
    val >>= 1;*/

  return delta;
}
