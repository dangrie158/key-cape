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

#ifndef _CLICKENCODER_H_
#define _CLICKENCODER_H_

// ----------------------------------------------------------------------------

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdint.h>

// ----------------------------------------------------------------------------

void encoder_init();
int16_t encoder_getValue(void);

// ----------------------------------------------------------------------------

#endif // _CLICKENCODER_H_
