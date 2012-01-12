/*
 * game.c
 *
 *  Created on: 10 janv. 2012
 *      Author: redsuser
 */

#include <linux/module.h>

#include <native/task.h>
#include <native/intr.h>
#include <native/event.h>
#include <native/alarm.h>
#include <native/timer.h>
#include <native/mutex.h>

#include "xeno-i2c.h"
#include "xeno-ts.h"
#include "lcdlib.h"
#include "pca9554-m.h"
#include "rt-app-m.h"

#include "game.h"
#include "lcdlib.h"
#include "display.h"

t_ennemi_ ennemi[nbEnnemis];

t_player_ player[3];

t_shot_ shot[nbShotsMax];

unsigned int speed;

// 1 = easy, 2 = medium, 3 = hard
unsigned int difficulty;

unsigned int score;

int game_init(void) {
	int err;

	speed++;

	ennemi_init();

	player[0].enable = 1;
	player[0].x = LCD_MAX_X / 2;
	player[0].y = LCD_MAX_Y - 20;

	// Création de la tâche gérant le rafraichissement de l'écran
	err = rt_task_create(&refresh_task, "menu", STACK_SIZE, 50, 0);
	if (err != 0) {
		printk("menu task creation failed: %d\n", err);
		return -1;
	}

	printk("Task created\n");

	err = rt_task_start(&refresh_task, refresh, 0);
	if (err != 0) {
		printk("menu task start failed: %d\n", err);
		return -1;
	}

}

// Défini une nouvelle vague d'ennemis
void ennemi_init(void) {

	int i, j;
	// position de départ de la vague d'ennemis

	int err;
	int nbEnnemiParVague = nbEnnemis / nbVagueEnnemis;

	// initialisation vaisseaux ennemis
	for (i = 0; i < nbVagueEnnemis; i++) {

		for (j = 0; j < nbEnnemiParVague; j++) {
			// Active tous les ennemis
			ennemi[i * nbEnnemiParVague + j].enable = 1;
			// Réinitialise les positions
			ennemi[i * nbEnnemiParVague + j].x = xStart + (j * (SHIT_SIZE
					+ X_SPACE));
			ennemi[i * nbEnnemiParVague + j].y = yStart + (i * (SHIT_SIZE
					+ Y_SPACE));
			// Initialise le nombre de point de vie selon la difficulté
			ennemi[i * nbEnnemiParVague + j].pv = difficulty
					* DEFAULT_PV_ENNEMI;
		}

	}

	// Création de la tâche gérant le rafraichissement de l'écran
	err = rt_task_create(&ennemi_task, "ennemi", STACK_SIZE, 50, 0);
	if (err != 0) {
		printk("menu task creation failed: %d\n", err);

	}

	printk("Task created\n");

	err = rt_task_start(&ennemi_task, move_ennemi, 0);
	if (err != 0) {
		printk("menu task start failed: %d\n", err);

	}

}

void move_ennemi(void* cookie) {

	int err;
	int i;
	//int ennemisEnable = 1;
	int direction = DIRECTION_EST;
	int yFirstEnnemi = yStart;
	bool directionChanged = false;

	// Configuration de la tâche périodique
	if (TIMER_PERIODIC) {
		err = rt_task_set_periodic(&ennemi_task, TM_NOW, PERIOD_TASK_ENNEMI);
		if (err != 0) {
			printk("Menu task set periodic failed: %d\n", err);
			return;
		}

	} else {
		err = rt_task_set_periodic(&ennemi_task, TM_NOW, PERIOD_TASK_ENNEMI
				* MS);
		if (err != 0) {
			printk("Menu task set periodic failed: %d\n", err);
			return;
		}
	}
	while (1) {
		while (detectShitEnable()) {

			// Position dernier vaisseaux en x
			int xLastEnnemi;
			// Position dernier vaisseaux en y
			int yLastEnnemi;

			/****************************************************************/

			/* Détection xLastEnnemi
			 *
			 * Nous testons si un des vaisseaux ennemis a touché
			 * un bord (est/ouest), ceci, en fonction de leurs directions.
			 *
			 */

			if (direction == DIRECTION_EST) {
				xLastEnnemi = 0;
				// detection du vaisseau le plus à l'est
				for (i = 0; i < nbEnnemis; i++) {
					if ((ennemi[i].x > xLastEnnemi) && (ennemi[i].enable == 1)) {
						xLastEnnemi = ennemi[i].x;
						directionChanged = true;
					}

				}
				// détection vaisseaux touchent le bord à l'est
				if (xLastEnnemi + SHIT_SIZE == EDGE_EAST) {
					direction = DIRECTION_OUEST;
					yFirstEnnemi+=Y_SPACE;
				}

			} else {
				xLastEnnemi = EDGE_EAST;
				// detection du vaisseau le plus à l'ouest
				for (i = 0; i < nbEnnemis; i++) {
					if ((ennemi[i].x < xLastEnnemi) && (ennemi[i].enable == 1)) {
						xLastEnnemi = ennemi[i].x;
					}
				}
				// détection vaisseaux touchent le bord à l'est
				if (xLastEnnemi == EDGE_WEST) {
					direction = DIRECTION_EST;
					directionChanged = true;
				}
			}

			/****************************************************************/

			/* Détection yLastEnnemi
			 *
			 * Peu etre utilisé lorsque les vaisseaux ennemis
			 * atteignent les vaisseaux alliés
			 */

			yLastEnnemi = 0;
			// detection vaisseaux le plus au sud
			for (i = 0; i < nbEnnemis; i++) {
				if ((ennemi[i].y > yLastEnnemi) && (ennemi[i].enable == 1)) {
					yLastEnnemi = ennemi[i].y;
				}

			}

			/****************************************************************/

			/* Deplacement vaisseaux ennemis
			 *
			 * Après avoir effectué les tests de direction, nous pouvons alors
			 * déplacer les vaisseaux ennemis vers l'est ou l'ouest.
			 *
			 */

			for (i = 0; i < nbEnnemis; i++) {
				if (directionChanged) {
					ennemi[i].y += STEP_MOVE_ENNEMI;
					directionChanged = false;
				}else{
					ennemi[i].x += STEP_MOVE_ENNEMI * direction;
				}

			}
			/****************************************************************/

		}
		// tous les vaisseaux ennemis ont été détruit : nouveau niveau !
		printk("Vaisseaux ennemis abattus");
		printk("new level");
	}

}
/*
 * Fonction qui retourne "true" s'il existe encore
 * un vaisseau ennemi en vie
 */
bool detectShitEnable(void) {
	int i;
	// Test s'il existe encore un vaisseau ennemi
	for (i = 0; i < nbEnnemis; i++) {
		if (ennemi[i].enable == 1) {
			return true;
		}
	}
	return false;
}
void game_main(void) {
	if (game_init() < 0) {
		printk("game_init() failed");
		return;
	}

}
