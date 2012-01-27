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
#include "gift.h"
#include "lcdlib.h"

/*
 * Auteur : Failletaz Romain
 *
 * But : Cette tache gere le deplacement du bonus (verticalement).  
 */
void gift_weapon(void *cookie) {

	int err;
	int nbRandom = 0;
	int tmpSpeed = 0;

	/* Configuration de la tache periodique */
	if (TIMER_PERIODIC) {
		err = rt_task_set_periodic(&gift_task, TM_NOW, PERIOD_TASK_GIFT);
		if (err != 0) {
			printk("Gift events task set periodic failed: %d\n", err);
			return;
		}

	} else {
		err = rt_task_set_periodic(&gift_task, TM_NOW, PERIOD_TASK_GIFT * MS);
		if (err != 0) {
			printk("Gift events task set aperiodic failed: %d\n", err);
			return;
		}
	}
	while (1) {
		// Le bonus est disponible tout les "GIFREVERYLEVEL" niveaux.
		if ((speed % GIFTEVERYLEVEL) == 0 ) {
			// Cree le bonus lors d un nouveau niveau
			if ((gift.enable == 0) && (tmpSpeed != speed)) {
				// Sauvegarde le niveau
                tmpSpeed = speed;

				printk("Bonus lance x : %i y: %i\n", gift.x, gift.y);

				// Choisi un endroit sur X au hasard
				nbRandom = get_random() % (EDGE_EAST - EDGE_WEST - GIFT_SIZE);

				// Fixe le cadeau
				gift.x = nbRandom;
				gift.y = EDGE_NORTH;
                // Rend le cadeau visible
				gift.enable = 1;

			} else {
				// Deplace le bonus verticalement si celui-ci est deja cree
				gift.y = gift.y + STEP_GIFT;
				
				// Detection : cadeau touche player
				if (touchPlayer()) {
					gift.enable = 0;

					// Bonus : Ajout des deux allies
					reinforcement_handler();

					//printk("Bonus obtenu\n");
				}
				// Detection : cadeau touche player
				if (touchGround()) {
					gift.enable = 0;
					//printk("Bonus perdu\n");
				}
			}
        // Continue la decente du bonus s il y changement de niveau avant
        // que le bonus ne soit ni gagne ou perdu par le joueur 
		} else if( (tmpSpeed == speed-1) && (gift.y > EDGE_NORTH)) {
			// se deplace verticalement
			gift.y = gift.y + STEP_GIFT;
			// Detection : cadeau touche player
			if (touchPlayer()) {
				gift.enable = 0;

				// TODO: ajoute les deux allies
				reinforcement_handler();

				//printk("Bonus obtenu\n");
			}
			// Detection : cadeau touche player
			if (touchGround()) {
				gift.enable = 0;
				//printk("Bonus perdu\n");
			}
		}
		// Si le niveau ne correspond pas au "GIFREVERYLEVEL" 
        // celui-ci n est pas disponible (affiche)
		else {
			gift.enable = 0;
		}
		rt_task_wait_period(NULL);
	}

}

/*
 * Auteur : Failletaz Romain
 *
 * But : Retourne si le bonus est touche par le joueur  
 */
int touchPlayer(void) {
	// Detection : ennemi touche player
	if (gift.enable) {
		// Detecte sur les x
		if ((((gift.x + GIFT_SIZE) >= player[0].x)
				&& ((gift.x + GIFT_SIZE) <= player[0].x + SHIP_SIZE))
				|| ((gift.x <= (player[0].x + SHIP_SIZE))
						&& (gift.x >= player[0].x))) {
			// Detecte sur les y
			if ((gift.y + GIFT_SIZE) >= player[0].y) {
				return 1;
			}
		}
	}
	return 0;
}

/*
 * Auteur : Failletaz Romain
 *
 * But : Retourne si le bonus touche le sol 
 */
int touchGround(void) {
	// Detecte si le cadeau touche le sol
	if (gift.enable) {
		if ((gift.y + GIFT_SIZE + STEP_GIFT) >= EDGE_SOUTH) {
			return 1;
		}
	}
	return 0;
}
