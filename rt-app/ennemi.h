/*
 * ennemi.h
 *
 *  Created on: 19 janv. 2012
 *      Author: Romain Failletaz
 */

#ifndef ENNEMI_H_
#define ENNEMI_H_

// doit être un multiple du nombre de vagues ennemies

extern int ennemi_init(void);
extern void move_ennemi(void* cookie);
bool detectShipEnable(void);
void show_ennemi(void);

#endif /* ENNEMI_H_ */
