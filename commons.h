/*
 * 20190409.011
 * SD Card
 *
 * File: commons.h
 * Processor: PIC12F1840
 * Author: wizlab.it
 */

#ifndef COMMONS_H
#define COMMONS_H

#include <xc.h>
#include <stdint.h>
#include "SD.h"

#define _XTAL_FREQ 32000000     //CPU Frequency

#define _LED    PORTAbits.RA5   //Led

//External functions
extern void init(void);

#endif