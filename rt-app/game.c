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

	// Création de la tâche gérant le déplacement des vaisseaux ennemis
	err =  rt_task_create (&ennemi_task, "move_ennemi", STACK_SIZE, 50, 0);
	if (err != 0) {
		printk("menu task creation failed: %d\n", err);
		return -1;
	}

	printk("Ennemi Task created\n");

	err = rt_task_start(&ennemi_task, move_ennemi, 0);
	if (err != 0) {
		printk("menu task start failed: %d\n", err);
		return -1;
	}


	return 0;
}

/*
 * Fonction qui retourne "true" s'il existe encore
 * un vaisseau ennemi en vie
 */
bool detectShitEnable() {
	int i;
	// Test s'il existe encore un vaisseau ennemi
	for (i = 0; i < nbEnnemis; i++) {
		if (ennemi[i].enable == 1) {
			return true;
		}
	}
	return false;
}

// Initiaisation des vaisseaux ennemis
int ennemi_init(void) {

	int i, j;
	int nbEnnemiParVague;

	// position de dÃ©part de la vague d'ennemis
    if(nbEnnemis%nbVagueEnnemis != 0 ){
        printk("Le nombre de vaisseaux par vague n'est pas conforme\n");
        return -1;
    }

    nbEnnemiParVague = nbEnnemis / nbVagueEnnemis;

	// initialisation vaisseaux ennemis
	for (i = 0; i < nbVagueEnnemis; i++) {

		for (j = 0; j < nbEnnemiParVague; j++) {
			// Active tous les ennemis
			ennemi[i * nbEnnemiParVague + j].enable = 1;
			// RÃ©initialise les positions
			ennemi[i * nbEnnemiParVague + j].x = xStart + (j * (SHIT_SIZE
					+ X_SPACE));
			ennemi[i * nbEnnemiParVague + j].y = yStart + (i * (SHIT_SIZE
					+ Y_SPACE));
			// Initialise le nombre de point de vie selon la difficultÃ©
			ennemi[i * nbEnnemiParVague + j].pv = difficulty
					* DEFAULT_PV_ENNEMI;
		}
	}
	return 0;

}

void show_ennemi(void){
	int i, j;
	// position de dÃ©part de la vague d'ennemis

	int nbEnnemiParVague = nbEnnemis / nbVagueEnnemis;

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

// Défini une nouvelle vague d'ennemis
void move_ennemi(void* cookie) {

	int i;
	int direction;
	int yFirstEnnemi = yStart;
	int err;

	// Configuration de la tâche périodique
	if (TIMER_PERIODIC)
	{
		err = rt_task_set_periodic(&ennemi_task, TM_NOW, PERIOD_TASK_ENNEMI);
		if (err != 0) {
			printk("Ennemi task set periodic failed: %d\n", err);
			return;
		}

	}
	else
	{
		err = rt_task_set_periodic(&ennemi_task, TM_NOW, PERIOD_TASK_ENNEMI*MS);
		if (err != 0) {
			printk("Ennemi task set periodic failed: %d\n", err);
			return;
		}
	}

	// Variable active lorsque qu'il y a (ou doit avoir) un changement
	// de direction des vaisseaux ennemis
	bool directionChanged = false;

	printk("*************************************************\n");
    printk("Init ennemi\n");
    printk("*************************************************\n");

    //initialisation des vaisseaux ennemis
	if(ennemi_init() < 0)
       return;

    // Test
    show_ennemi();
    printk("*************************************************\n");
    while(1){

        direction = DIRECTION_EST;

        while (detectShitEnable()) {

			// Position dernier vaisseaux en x
			int xLastEnnemi;
			// Position dernier vaisseaux en y
			int yLastEnnemi;

			/****************************************************************/

			/* DÃ©tection xLastEnnemi
			 *
			 * Nous testons si un des vaisseaux ennemis a touchÃ©
			 * un bord (est/ouest), ceci, en fonction de leurs directions.
			 *
			 */

			if (direction == DIRECTION_EST) {
				xLastEnnemi = 0;
				// detection du vaisseau le plus Ã  l'est
				for (i = 0; i < nbEnnemis; i++) {
					if ((ennemi[i].x > xLastEnnemi) && (ennemi[i].enable == 1)) {
						xLastEnnemi = ennemi[i].x;
					}

				}
				// dÃ©tection vaisseaux touchent le bord Ã  l'est
				if ( xLastEnnemi + SHIT_SIZE > EDGE_EAST - STEP_MOVE_ENNEMI){
					direction = DIRECTION_OUEST;
					directionChanged = true;
					yFirstEnnemi+=Y_SPACE;
				}

			} else {
				xLastEnnemi = EDGE_EAST;
				// detection du vaisseau le plus Ã  l'ouest
				for (i = 0; i < nbEnnemis; i++) {
					if ((ennemi[i].x < xLastEnnemi) && (ennemi[i].enable == 1)) {
						xLastEnnemi = ennemi[i].x;
					}
				}
				// dÃ©tection vaisseaux touchent le bord Ã  l'est
				if ( ( EDGE_WEST + STEP_MOVE_ENNEMI) > xLastEnnemi  ) { //&& (xLastEnnemi <= EDGE_WEST)
					direction = DIRECTION_EST;
				    yFirstEnnemi+=Y_SPACE;
                	directionChanged = true;
				}
			}

			/****************************************************************/

			/* Detection yLastEnnemi
			 *
			 * Peu etre utilisÃ© lorsque les vaisseaux ennemis
			 * atteignent les vaisseaux alliÃ©s
			 */

			yLastEnnemi = 0;
			// detection vaisseaux le plus au sud
			for (i = 0; i < nbEnnemis; i++) {
				if ((ennemi[i].y > yLastEnnemi) && (ennemi[i].enable == 1)) {
					yLastEnnemi = ennemi[i].y;
				}

			}

			/****************************************************************/

            /*
             * Test : affiche si la direction doit changer (est <-> ouest)
			 */
            if( directionChanged ){
                printk("changement de direction : oui\n");
            }else{
                printk("changement de direction : non\n");
            }


            printk("xLastEnnemi : %i\n", xLastEnnemi);
            printk("yLastEnnemi : %i\n", yLastEnnemi);

            printk("*************************************************\n");

			/****************************************************************/

			/* Deplacement vaisseaux ennemis
			 *
			 * AprÃšs avoir effectuÃ© les tests de direction, nous pouvons alors
			 * dÃ©placer les vaisseaux ennemis vers l'est ou l'ouest.
			 *
			 */

			for (i = 0; i < nbEnnemis; i++) {
				if (directionChanged) {
					ennemi[i].y += STEP_MOVE_ENNEMI;

				}else{
					ennemi[i].x += STEP_MOVE_ENNEMI * direction;
				}
			}
			directionChanged = false;

			/****************************************************************/

			/* Vaisseaux Ennemis : enable
			 *
			 * Le vaisseau ennemi est détruit lorsqu'il n'a plus de point de vie
			 *
			 */

   	         for (i = 0; i < nbEnnemis; i++) {
			     if ( ennemi[i].pv == 0 ){
			        ennemi[i].enable = 0;
                 }
             }
             /****************************************************************/
   	    	rt_task_wait_period(NULL);
		}

		// tous les vaisseaux ennemis ont Ã©tÃ© dÃ©truit : nouveau niveau !
		printk("Vaisseaux ennemis abattus\n");
		printk("new level\n");

        difficulty++;

        ennemi_init();

    }




}


void game_main(void) {
	if (game_init() < 0) {
		printk("game_init() failed");
		return;
	}

}
