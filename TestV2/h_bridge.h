/*
 * h_bridge.h - XvR 2020
 */

#ifndef _H_BRIDGE_H_
#define _H_BRIDGE_H_

// These pins are available on the shield via the header:
//
//		Mega	Uno
// digital 5	PE3	PD5
// digital 6	PH3	PD6
// digital 9	PH6	PB1
// analog 5	PF5	PC5

// The settings below are for the Mega, modify
// in case you want to use other pins
#define PORT_RPWM	PORTG
#define PIN_RPWM	PG5
#define DDR_RPWM	DDRG

#define PORT_LPWM	PORTE
#define PIN_LPWM	PE5
#define DDR_LPWM	DDRE

void init_h_bridge(void);
void h_bridge_set_percentage(signed char percentage);

#endif /* _H_BRIDGE_H_ */
