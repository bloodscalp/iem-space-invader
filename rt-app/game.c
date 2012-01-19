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

RT_MUTEX mutex_ennemi;

RT_TASK move_task, shots_impacts_task, ennemi_task, switch_events_task;
#define PERIOD_TASK_MOVE 50

int SW2_event, SW3_event, SW4_event, SW5_event;

int err;
int i2c_fd;

int game_init(void) {
	int err;

	speed++;

	player[0].enable = 1;
	player[0].x = LCD_MAX_X / 2 - 8;
	player[0].y = LCD_MAX_Y - 20;
	player[0].lifes = 4;

	// Création de la tâche gérant le rafraichissement de l'écran
	err = rt_task_create(&refresh_task, "refresh", STACK_SIZE, 50, 0);
	if (err != 0) {
		printk("refresh creation failed: %d\n", err);
		return -1;
	}

	printk("Task created\n");

	err = rt_task_start(&refresh_task, refresh, 0);
	if (err != 0) {
		printk("refresh task start failed: %d\n", err);
		return -1;
	}

	// Création de la tâche gérant le rafraichissement de l'écran
	err = rt_task_create(&move_task, "move_player", STACK_SIZE, 50, 0);
	if (err != 0) {
		printk("move player creation failed: %d\n", err);
		return -1;
	}

	printk("Task created\n");

	err = rt_task_start(&move_task, move_player, 0);
	if (err != 0) {
		printk("move player start failed: %d\n", err);
		return -1;
	}

	// Création de la tâche gérant le déplacement des vaisseaux ennemis
	err = rt_task_create(&ennemi_task, "move_ennemi", STACK_SIZE, 50, 0);
	if (err != 0) {
		printk("ennemi task creation failed: %d\n", err);
		return -1;
	}

	printk("Ennemi Task created\n");

	err = rt_task_start(&ennemi_task, move_ennemi, 0);
	if (err != 0) {
		printk("ennemi task start failed: %d\n", err);
		return -1;
	}


	// Création de la tâche gérant les tirs et les impacts
	err =  rt_task_create (&shots_impacts_task, "shots_impacts", STACK_SIZE, 50, 0);
	if (err != 0) {
		printk("Shots & impacts task creation failed: %d\n", err);
		return -1;
	}

	printk("Shots & impacts task created\n");

	err = rt_task_start(&shots_impacts_task, shots_impacts, 0);
	if (err != 0) {
		printk("Shots & impacts task start failed: %d\n", err);
		return -1;
	}


	// Création de la tâche gérant les switchs
	err =  rt_task_create (&switch_events_task, "switch_events", STACK_SIZE, 50, 0);
	if (err != 0) {
		printk("Switch events task creation failed: %d\n", err);
		return -1;
	}

	printk("Switch events task created\n");

	err = rt_task_start(&switch_events_task, switch_events, 0);
	if (err != 0) {
		printk("Switch events task start failed: %d\n", err);
		return -1;
	}


	rt_mutex_create(&mutex_ennemi, "mutex ennemi");

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
	if (nbEnnemis % nbVagueEnnemis != 0) {
		printk("Le nombre de vaisseaux par vague n'est pas conforme\n");
		return -1;
	}

	nbEnnemiParVague = nbEnnemis / nbVagueEnnemis;

	rt_mutex_lock(&mutex_ennemi, TM_INFINITE);

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
	rt_mutex_unlock(&mutex_ennemi);
	return 0;

}

void show_ennemi(void) {
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
	int border = 15;
	int EdgeX_left = border;
	int EdgeX_right = LCD_MAX_X - border;
	int touch = 0;
	struct ts_sample touch_info;
	int speed = 5;

	// Configuration de la tâche périodique
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
			printk("Ennemi task set periodic failed: %d\n", err);
			return;
		}
	}

	// Variable active lorsque qu'il y a (ou doit avoir) un changement
	// de direction des vaisseaux ennemis
	bool directionChanged;
	directionChanged = false;

	printk("*************************************************\n");
	printk("Init ennemi\n");
	printk("*************************************************\n");

	//initialisation des vaisseaux ennemis
	if (ennemi_init() < 0)
		return;

	// Test
	//show_ennemi();
	printk("*************************************************\n");
	while (1) {

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
				if (xLastEnnemi + SHIT_SIZE > EDGE_EAST - speed) {
					direction = DIRECTION_OUEST;
					directionChanged = true;
					yFirstEnnemi += Y_SPACE;
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
				if ((EDGE_WEST + speed) > xLastEnnemi) { //&& (xLastEnnemi <= EDGE_WEST)
					direction = DIRECTION_EST;
					yFirstEnnemi += Y_SPACE;
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
			if (directionChanged) {
				printk("changement de direction : oui\n");
			} else {
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

			rt_mutex_lock(&mutex_ennemi, TM_INFINITE);

			for (i = 0; i < nbEnnemis; i++) {
				if (directionChanged) {
					ennemi[i].y += speed;

				} else {
					ennemi[i].x += speed * direction;
				}
			}

			rt_mutex_unlock(&mutex_ennemi);

			directionChanged = false;

			/****************************************************************/

			/* Vaisseaux Ennemis : enable
			 *
			 * Le vaisseau ennemi est détruit lorsqu'il n'a plus de point de vie
			 *
			 */

			for (i = 0; i < nbEnnemis; i++) {
				if (ennemi[i].pv == 0) {
					ennemi[i].enable = 2;
				}
			}
			/****************************************************************/
			rt_task_wait_period(NULL);
		}

		// tous les vaisseaux ennemis ont Ã©tÃ© dÃ©truit : nouveau niveau !
		printk("Vaisseaux ennemis abattus\n");
		printk("new level\n");

		speed++;

		ennemi_init();

	}

}


void move_player(void * cookie) {

	int err;
	int border = 15;
	int EdgeX_left = border;
	int EdgeX_right = LCD_MAX_X - border;
	int touch = 0;
	struct ts_sample touch_info;
	int speed = 5;

	// Configuration de la tâche périodique
	if (TIMER_PERIODIC) {
		err = rt_task_set_periodic(&move_task, TM_NOW, PERIOD_TASK_MOVE);
		if (err != 0) {
			printk("Move task set periodic failed: %d\n", err);
			return;
		}

	} else {
		err = rt_task_set_periodic(&move_task, TM_NOW, PERIOD_TASK_MOVE * MS);
		if (err != 0) {
			printk("Move task set periodic failed: %d\n", err);
			return;
		}
	}

	while (1) {

		// Attend que l'utilisateur touche l'écran
		while (touch == 0) {
			rt_task_wait_period(NULL);

			if (xeno_ts_read(&touch_info, 1, O_NONBLOCK) > 0) {
				printk("x = %d, y = %d\n", touch_info.x, touch_info.y);
				touch = 1;

				if (touch_info.x > player[0].x) {
					if (player[0].x + 16 < EdgeX_right)
						player[0].x += speed;
				} else {
					if (player[0].x > EdgeX_left)
						player[0].x -= speed;
				}

				while (xeno_ts_read(&touch_info, 1, O_NONBLOCK) > 0)
					;
			}
		}

		printk("x_player = %d \n", player[0].x);

		rt_task_wait_period(NULL);

		touch = 0;

	}

}




/**
 * Tâche gérant les mouvements des projectiles et les impacts de
 * ces derniers avec les vaisseaux
 */
void shots_impacts(void * cookie) {

	int i, j;

	// Configuration de la tâche périodique
	if (TIMER_PERIODIC) {
		err = rt_task_set_periodic(&shots_impacts_task, TM_NOW, PERIOD_TASK_MOVE);
		if (err != 0) {
			printk("Shots and impacts task set periodic failed: %d\n", err);
			return;
		}

	} else {
		err = rt_task_set_periodic(&shots_impacts_task, TM_NOW, PERIOD_TASK_MOVE * MS);
		if (err != 0) {
			printk("Shots and impacts task set periodic failed: %d\n", err);
			return;
		}
	}

	while (1) {
		rt_task_wait_period(NULL);

		/* Parcours la liste des tirs */
		for(i=0; i<nbShotsMax; i++) {
			if(shot[i].enable == 1)
			{
				/* Fait avancer/reculer le tir s'il est enabled */
				shot[i].y += shot[i].direction;

				// Si le shot est à la hauteur du joueur
				if(shot[i].y > (LCD_MAX_Y-20))
				{
					for(j = 0; j < 3; j++)
					{
						if(player[j].enable == 1)
						{
							// S'il y a une collision avec le joueur
							if((shot[i].x > player[j].x) && (shot[i].x < (player[j].x + 16)))
							{
								printk("Player touched\n");
								player[j].enable++;
								shot[i].enable = 0;
							}
						}
					}
				}
				// Si le shot est dans la zone ennemis
				else
				{
					for(j = 0; j < nbEnnemis; j++)
					{
						if(ennemi[j].enable == 1)
						{
							// S'il y a une collision avec un ennemi
							if( (shot[i].x > ennemi[j].x) && (shot[i].x < (ennemi[j].x + 16))
							 && (shot[i].y > ennemi[j].y) && (shot[i].y < (ennemi[j].y + 16)) )
							{
								printk("Ennemi n°%d touched\n", i);
								ennemi[j].pv--;
								shot[i].enable = 0;
							}
						}
					}
				}
			}
		}
	}
}


void switch_events(void *cookie) {

	int i;
	int ctr;

	// Configuration de la tâche périodique
	if (TIMER_PERIODIC) {
		err = rt_task_set_periodic(&switch_events_task, TM_NOW, PERIOD_TASK_MOVE);
		if (err != 0) {
			printk("Switch events task set periodic failed: %d\n", err);
			return;
		}

	} else {
		err = rt_task_set_periodic(&switch_events_task, TM_NOW, PERIOD_TASK_MOVE * MS);
		if (err != 0) {
			printk("Switch events task set periodic failed: %d\n", err);
			return;
		}
	}

	while(1) {
		rt_task_wait_period(NULL);

		i = 0;
		ctr = 0;

		check_switch_events_once();

		/* This is a cheat :p */
		if(SW5_event && SW3_event) {
			SW5_event = 0;
			SW3_event = 0;

			player[0].lifes++;
			hp_update_leds();

		}


		if(SW2_event) {
			SW2_event = 0;

			/* Parcours le tableau des tirs */
			while(1) {
				ctr++;

				/* Si le tir courant est inactif */
				if(shot[i].enable == 0) {
					/* L'initialise et l'active */
					shot[i].x = player[1].x;
					shot[i].y = player[1].y;
					shot[i].direction = -1; // Moves up
					shot[i].enable = 1;
					break;
				} else {
					/* Pase au tir suivant */
					i = ((i+1) % nbShotsMax);

					/* Si on a parcouru plus de 2 fois le tableau */
					if(ctr > nbShotsMax*2)
						/* Annule le tir pour éviter de planter les suivants */
						break;
				}
			}
		}

		if(SW3_event) {
			SW3_event = 0;

			if(player[0].lifes < 4) {
				player[0].lifes++;
				hp_update_leds();
			}
		}

		if(SW4_event) {
			SW4_event = 0;

			if(player[0].lifes > 0) {
				player[0].lifes--;
				hp_update_leds();
			}
		}
	}
}


void check_switch_events_once() {

	char buf[1];
	char lastBuf[1];

	char switch_change, switch_change_up;

	if((err = read(i2c_fd, buf, 1)) < 0) {
		printk("i2c read error : %d\n", err);
	} else if(buf[0] != lastBuf[0]) {

		switch_change = (buf[0] ^ lastBuf[0]) >> 4;
		switch_change_up = switch_change & ~(buf[0] >> 4);

		if(switch_change_up & 0x1) {
			SW5_event = 1;
		}

		if(switch_change_up & 0x2) {
			SW4_event = 1;
		}

		if(switch_change_up & 0x4) {
			SW3_event = 1;
		}

		if(switch_change_up & 0x8) {
			SW2_event = 1;
		}

		lastBuf[0] = buf[0];
	}
}

void hp_update_leds() {

	char buf[1];

	/* Conversion int -> LEDS */
	buf[0] = 0x0F << player[0].lifes;

	/* Inversion des hp pour décrémentation depuis le haut */
	switch(buf[0]) {
	case 0x1:
		buf[0] = 0x8;
		break;

	case 0x3:
		buf[0] = 0xC;
		break;

	case 0x7:
		buf[0] = 0xE;
		break;
	}

	if((err = write(i2c_fd, buf, 1)) < 0) {
		printk("i2c write error : %d\n", err);
	}
}


void switch_init() {
	mknod("/var/dev/i2c", S_IFCHR|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH, makedev(10,0));

	i2c_fd = open("/var/dev/i2c", O_RDWR);
}



void game_main(void) {
	if (game_init() < 0) {
		printk("game_init() failed");
		return;
	}

	while (player[0].lifes > 0) {
		rt_task_wait_period(NULL);
	}
}
