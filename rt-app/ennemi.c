/*
 * ennemi.c
 *
 *  Created on: 19 janv. 2012
 *      Author: Romain Failletaz
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
#include "ennemi.h"


/*
 * Auteur : Failletaz Romain
 *
 * But : Fonction qui retourne "true" s'il existe encore
 *       un vaisseau ennemi en vie
 */
bool detectShipEnable(void) {
	int i;
	// Test s'il existe encore un vaisseau ennemi
	for (i = 0; i < nbEnnemis; i++) {
		if (ennemi[i].enable == 1) {
			return true;
		}
	}
	return false;
}

/*
 * Auteur : Failletaz Romain
 *
 * But : Initiaisation des vaisseaux ennemis
 */
int ennemi_init(void) {

	int i, j;
	//int nbEnnemiParVague;

	// position de da©part de la vague d'ennemis
	if (nbEnnemis % nbVagueEnnemis != 0) {
		printk("Le nombre de vaisseaux par vague n'est pas conforme\n");
		return -1;
	}

	//nbEnnemiParVague = nbEnnemis / nbVagueEnnemis;

	rt_mutex_lock(&mutex_ennemi, TM_INFINITE);

	// initialisation vaisseaux ennemis
	for (i = 0; i < nbVagueEnnemis; i++) {

		for (j = 0; j < nbEnnemiParVague; j++) {
			// Active tous les ennemis
			ennemi[i * nbEnnemiParVague + j].enable = 1;
			// Ra©initialise les positions
			ennemi[i * nbEnnemiParVague + j].x = xStart + (j * (SHIP_SIZE
					+ X_SPACE));
			ennemi[i * nbEnnemiParVague + j].y = yStart + (i * (SHIP_SIZE
					+ Y_SPACE));
			// Initialise le nombre de point de vie selon la difficulta©
			if(speed >= 1 && speed < 5)
			{
				ennemi[i * nbEnnemiParVague + j].pv = 1;
			}
			else if(speed >= 5 && speed < 10)
			{
				ennemi[i * nbEnnemiParVague + j].pv = 2;
			}
			else if(speed >= 10 && speed < 15 )
			{
				ennemi[i * nbEnnemiParVague + j].pv = 3;
			}
			else if(speed >= 15 )
			{
				ennemi[i * nbEnnemiParVague + j].pv = 4;
			}
		}
	}
	rt_mutex_unlock(&mutex_ennemi);
	return 0;

}

/*
 * Auteur : Failletaz Romain
 *
 * But : Cette fonction affiche la position des ennemi.
 *       A utiliser lors de phases de tests
 */
void show_ennemi(void) {
	int i, j;

	for (j = 0; j < nbEnnemiParVague; j++) {
		// Active tous les ennemis
		printk("X\tY\tPV\ten\t\t");
	}
	printk("\n");
	// initialisation vaisseaux ennemis
	for (i = 0; i < nbVagueEnnemis; i++) {

		for (j = 0; j < nbEnnemiParVague; j++) {
			// Active tous les ennemis
			printk("%i\t", ennemi[i * nbEnnemiParVague + j].x);
			printk("%i\t", ennemi[i * nbEnnemiParVague + j].y);
			printk("%i\t", ennemi[i * nbEnnemiParVague + j].pv);
			printk("%i", ennemi[i * nbEnnemiParVague + j].enable);
			printk("\t\t");

		}
		printk("\n");
	}
}

/*
 * Auteur : Failletaz Romain
 *
 * But : Cette tache gere le deplacement et formation des ennemis
 */
void move_ennemi(void* cookie) {

	int i;
	int direction = DIRECTION_EST;
	int yFirstEnnemi = yStart;
	int err;
	bool directionChanged;

	// Position dernier vaisseaux en x
	int xLastEnnemi;
	// Position dernier vaisseaux en y
	int yLastEnnemi;

	// Configuration de la tache periodique
	if (TIMER_PERIODIC) {
		err = rt_task_set_periodic(&ennemi_task, TM_NOW, PERIOD_TASK_ENNEMI);
		if (err != 0) {
			printk("Ennemi task set periodic failed: %d\n", err);
			return;
		}

	} else {
		err = rt_task_set_periodic(&ennemi_task, TM_NOW, PERIOD_TASK_ENNEMI
				* MS);
		if (err != 0) {
			printk("Ennemi task set aperiodic failed: %d\n", err);
			return;
		}
	}

	// Variable active lorsque qu'il y a (ou doit avoir) un changement
	// de direction des vaisseaux ennemis

	directionChanged = false;

	//initialisation des vaisseaux ennemis
	if (ennemi_init() < 0) {
		printk("Error on ennemi init\n");
		return;
	}

	while (1) {

		/****************************************************************/

		/* Detection xLastEnnemi
		 *
		 * Nous testons si un des vaisseaux ennemis a touche
		 * un bord (est/ouest), ceci, en fonction de leurs directions.
		 *
		 */

		if (direction == DIRECTION_EST) {
			xLastEnnemi = 0;
			// detection du vaisseau le plus a l'est
			for (i = 0; i < nbEnnemis; i++) {
				if ((ennemi[i].x > xLastEnnemi) && (ennemi[i].enable == 1)) {
					xLastEnnemi = ennemi[i].x;
				}

			}
			// detection vaisseaux touchent le bord a l'est
			if (xLastEnnemi + SHIP_SIZE > EDGE_EAST - speed) {
				direction = DIRECTION_OUEST;
				directionChanged = true;
				yFirstEnnemi += Y_SPACE;
			}

		} else {
			xLastEnnemi = EDGE_EAST;
			// detection du vaisseau le plus a l'ouest
			for (i = 0; i < nbEnnemis; i++) {
				if ((ennemi[i].x < xLastEnnemi) && (ennemi[i].enable == 1)) {
					xLastEnnemi = ennemi[i].x;
				}
			}
			// detection vaisseaux touchent le bord a l'est
			if ((EDGE_WEST + speed) > xLastEnnemi) { //&& (xLastEnnemi <= EDGE_WEST)
				direction = DIRECTION_EST;
				yFirstEnnemi += Y_SPACE;
				directionChanged = true;
			}
		}

		/****************************************************************/

		/* Detection yLastEnnemi
		 *
		 * Peu etre utilise lorsque les vaisseaux ennemis
		 * atteignent les vaisseaux allies
		 */

		yLastEnnemi = 0;
		// detection vaisseaux le plus au sud
		for (i = 0; i < nbEnnemis; i++) {
			if ((ennemi[i].y > yLastEnnemi) && (ennemi[i].enable == 1)) {
				yLastEnnemi = ennemi[i].y;
			}

		}

		// Detection : ennemi touche player

        if(yLastEnnemi >= (EDGE_SOUTH-2*SHIP_SIZE)){
			for(i = 0; i < NB_PLAYER; i++)
				if(player[i].enable == 1)
					// provoque l explosion du joueur
					player[i].enable = 2;
		}

		/****************************************************************/

		/* Deplacement vaisseaux ennemis
		 *
		 * Apres avoir effectue les tests de direction, nous pouvons alors
		 * deplacer les vaisseaux ennemis vers l'est ou l'ouest.
		 *
		 */
		 
 		rt_mutex_lock(&mutex_ennemi, TM_INFINITE); 
		 		
		for (i = 0; i < nbEnnemis; i++) {
			if (directionChanged) {
				ennemi[i].y += MOVE_ENNEMI_Y;

			} else {
				ennemi[i].x += speed * direction;
			}
		}

		rt_mutex_unlock(&mutex_ennemi);
        
        // Le changement de direction a ete effectue
		directionChanged = false;


		rt_task_wait_period(NULL);

	}

}

// Met a jour la valeur des vaisseaux les plus en bas de l ecran
// dans le tableau ennemi_y_tab[nbEnnemiParVague]
void ennemi_pos_y(void) {

	int i, j;

	// Initialisation de la liste des ennemis
	for (i = 0; i < nbEnnemiParVague; i++)
		ennemi_y_tab[i].y = EDGE_NORTH;

	// initialisation vaisseaux ennemis
	for (i = 0; i < nbVagueEnnemis; i++) {

		for (j = 0; j < nbEnnemiParVague; j++) {

			if (ennemi[i * nbEnnemiParVague + j].enable == 1)
				if (ennemi_y_tab[j].y < ennemi[i * nbEnnemiParVague + j].y) {
					ennemi_y_tab[j].y = ennemi[i * nbEnnemiParVague + j].y;
					ennemi_y_tab[j].x = ennemi[i * nbEnnemiParVague + j].x;
				}
		}
	}

}
