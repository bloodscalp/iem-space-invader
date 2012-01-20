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

void missile_ennemi(void *cookie) {

	int i, j;
	int ctr;
	int err;
	int nbRandom = 0;

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

	// Start le generateur de nombre aleatoire
	srand(time(NULL));

	while (1) {

		for (i = 0; i < nbShotsMax; i++) {
			if (shot_ennemi[i].enable = 0)
				// Place disponible dans le tableau
				exit;
		}

		// Choisi un vaisseau ennemi au hasard sur la premiere vague
		nbRandom = rand() % nbVagueEnnemis;

		if (detectShipEnable()) {

			// Met a jour la position des ennemis en y dans un tableu
			ennemi_pos_y();

			// Tir un missile seulement si un vaisseau est present sur la colonne
			// definie par le chiffre aleatoire
			if (ennemi_y_tab[nbRandom].y != EDGE_NORTH) {
				shot_ennemi[j].x = ennemi_y_tab[nbRandom].x + SHIP_SIZE / 2;
				shot_ennemi[j].y = ennemi_y_tab[nbRandom].y + SHIP_SIZE;
				shot_ennemi[j].enable = 1;
				shot_ennemi[j].direction = DIRECTION_DOWN;
			}
			j++;
		}


		// Désactive le missile si celui-ci touche le bas de l ecran
		for (i = 0; i < nbShotsMax; j++) {
			if (shot_ennemi[i].y >= EDGE_SOUTH - MISSILE_SIZE)
				shot_ennemi[i].enable = 0;
		}

		// Mouvement des missiles pas a pas
		for (i = 0; i < nbShotsMax; j++) {
			if (shot_ennemi[i].enable == 1)
				shot_ennemi[i].y += MOVE_MISSILE;
		}

		rt_task_wait_period(NULL);
	}

}

