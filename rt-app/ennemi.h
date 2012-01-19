/*
 * ennemi.h
 *
 *  Created on: 19 janv. 2012
 *      Author: Romain Failletaz
 */

#ifndef ENNEMI_H_
#define ENNEMI_H_

#define nbEnnemis 24		// doit être un multiple du nombre de vagues ennemies
#define nbVagueEnnemis 4
#define DIRECTION_EST 1
#define DIRECTION_OUEST -1
#define SHIP_SIZE 16
#define DEFAULT_PV_ENNEMI 1
#define X_SPACE 4
#define Y_SPACE 4
#define xStart 10
#define yStart 10

extern RT_TASK ennemi_task;
extern RT_MUTEX mutex_ennemi;

extern t_ennemi_ ennemi[nbEnnemis];

extern int ennemi_init(void);
extern void move_ennemi(void* cookie);
bool detectShipEnable(void);
void show_ennemi(void);

#endif /* ENNEMI_H_ */
