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
#include "ennemi.h"
#include "switchs.h"



t_player_ player[3];

t_shot_ shot[nbShotsMax];

unsigned int speed;

// 1 = easy, 2 = medium, 3 = hard
unsigned int difficulty;

unsigned int score;


RT_TASK move_task, shots_impacts_task;
#define PERIOD_TASK_MOVE 50



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


	/* Initialisation de l'interface i2c */
	if(pca9554_init() < 0)
		return -1;

	/* Initialisation des switchs */
	if(switchs_init() < 0)
		return -1;


	rt_mutex_create(&mutex_ennemi, "mutex ennemi");

	return 0;


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




void hp_update_leds() {

	char buf[1];
	int hp = player[0].lifes;

	if(hp < 0)
		hp = 0;
	else if(hp < MAX_HP)
		hp = MAX_HP;


	/* Conversion int -> LEDS */
	buf[0] = 0x0F << hp;

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

	if((err = pca9554_write(NULL, buf, 1, 0)) < 0) {
		printk("i2c write error : %d\n", err);
	}
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
