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
	int maxshotlvl = 10;

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
		while (shot[shot_free].enable == 1) {
			shot_free++;
			shot_free = shot_free % NB_MAX_SHOTS;

		}

		// Random pour savoir si on va effectuer un tir ou non
		// en fonction du level où l'on est
		// On génére un nombre aléatoire de 1 à maxshotlvl (10)
		// Si ce nombre est inférieur au niveau auxquel on est,
		// on tir
		if ((get_random() % maxshotlvl) + 1 <= speed) {
			if (detectShipEnable()) {

				while (1) {
					// Choisi un vaisseau ennemi au hasard
					nbRandom = get_random() % nbEnnemis;

					if (ennemi[nbRandom].enable == 1)
						break;
				}

				// On tir, s il y a encore un vaisseau ennemi "vivant"

				// Tir un missile si le vaisseau ennemi existe
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

