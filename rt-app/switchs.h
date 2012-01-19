/*
 * switchs.h
 *
 *  Created on: 19 janv. 2012
 *      Author: redsuser
 */

#ifndef SWITCHS_H_
#define SWITCHS_H_

int SW2_event, SW3_event, SW4_event, SW5_event;

void check_switch_events_once(void);

int switchs_init(void);

#endif /* SWITCHS_H_ */
