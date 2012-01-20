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

	int i = 0;
	int j = 0;
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

	while (1) {

		// Place disponible dans le tableau	de tir
		while(shot_ennemi[shot_free].enable == 1)
			shot_free++;


		// Choisi un vaisseau ennemi au hasard sur la premiere vague
		nbRandom = get_random() % nbVagueEnnemis;

		// On tir, s il y a encore un vaisseau ennemi "vivant"
		if (detectShipEnable()) {

			// Met a jour la position des ennemis en y dans un tableau
			ennemi_pos_y();

			// Tir un missile seulement si un vaisseau est present sur la colonne
			// definie par le chiffre aleatoire
			if (ennemi_y_tab[nbRandom].y != EDGE_NORTH) {
				shot_ennemi[shot_free].x = ennemi_y_tab[nbRandom].x + SHIP_SIZE / 2;
				shot_ennemi[shot_free].y = ennemi_y_tab[nbRandom].y + SHIP_SIZE;
				shot_ennemi[shot_free].enable = 1;
				shot_ennemi[shot_free].direction = DIRECTION_DOWN;
			}
		}

		// Désactive le missile si celui-ci touche le bas de l ecran
		// TODO : doit etre desactive lorsque celui-ci touche un joueur
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

