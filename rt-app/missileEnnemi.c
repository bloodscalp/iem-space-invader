/*
 * missileEnnemi.c
 *
 *  Created on: 19 janv. 2012
 *      Author: Romain Failletaz
 */

#include <native/task.h>

#include "pca9554-m.h"
#include "rt-app-m.h"
#include "game.h"
#include "missileEnnemi.h"
#include "lcdlib.h"
#include "ennemi.h"


void missile_ennemi(void *cookie) {

	int err;
	int nbRandom = 0;
	int shot_free = 0;

	/* Configuration de la tâche périodique */
	if (TIMER_PERIODIC) {
		err = rt_task_set_periodic(&missile_ennemi_task, TM_NOW,
				PERIOD_TASK_MISSILE_ENNEMI);
		if (err != 0) {
			printk("Missile ennemi events task set periodic failed: %d\n", err);
			return;
		}

	} else {
		err = rt_task_set_periodic(&missile_ennemi_task, TM_NOW,
				PERIOD_TASK_MISSILE_ENNEMI * MS);
		if (err != 0) {
			printk("Missile ennemi events task set periodic failed: %d\n", err);
			return;
		}
	}

	while (1) {

		rt_mutex_lock(&mutex_shots, TM_INFINITE);

		// Trouve le premier tir disponible
		while(shot[shot_free].enable == 1)
			shot_free++;


		// Choisi un vaisseau ennemi au hasard
		nbRandom = get_random() % nbEnnemis;

		// On tir, s il y a encore un vaisseau ennemi "vivant"
		if (detectShipEnable()) {

			// Met a jour la position des ennemis en y dans un tableau
			//ennemi_pos_y();

			// Tir un missile depuis un ennemi au hasard
			if (ennemi[nbRandom].enable == 1) {
				shot[shot_free].x = ennemi[nbRandom].x + SHIP_SIZE / 2;
				shot[shot_free].y = ennemi[nbRandom].y + SHIP_SIZE;
				shot[shot_free].enable = 1;
				shot[shot_free].direction = DIRECTION_DOWN;
			}
		}

		rt_mutex_unlock(&mutex_shots);

		rt_task_wait_period(NULL);
	}

}

