/*
 * ennemi.h
 *
 *  Created on: 19 janv. 2012
 *      Author: Romain Failletaz
 */

#ifndef ENNEMI_H_
#define ENNEMI_H_

extern int ennemi_init(void);
extern void move_ennemi(void* cookie);
extern void ennemi_pos_y (void);
extern bool detectShipEnable(void);
void show_ennemi(void);

#endif /* ENNEMI_H_ */
