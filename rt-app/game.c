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


RT_TASK move_task, shots_impacts;
#define PERIOD_TASK_MOVE 50

int game_init(void)
{
	int err;

	speed++;

	ennemi_init();

	player[0].enable = 1;
	player[0].x = LCD_MAX_X/2 - 8;
	player[0].y = LCD_MAX_Y - 20;


	// Création de la tâche gérant le rafraichissement de l'écran
	err =  rt_task_create (&refresh_task, "refresh", STACK_SIZE, 50, 0);
	if (err != 0) {
		printk("refresh task creation failed: %d\n", err);
		return -1;
	}

	err = rt_task_start(&refresh_task, refresh, 0);
	if (err != 0) {
		printk("refresh task start failed: %d\n", err);
		return -1;
	}

	printk("Refresh Task created\n");


	// Création de la tâche gérant le déplacement du joueur
	err =  rt_task_create (&move_task, "move_player", STACK_SIZE, 50, 0);
	if (err != 0) {
		printk("menu task creation failed: %d\n", err);
		return -1;
	}

	printk("Move Task created\n");

	err = rt_task_start(&move_task, move_player, 0);
	if (err != 0) {
		printk("menu task start failed: %d\n", err);
		return -1;
	}


	// Création de la tâche gérant les tirs et les impacts
	err =  rt_task_create (&shots_impacts, "shots_impacts", STACK_SIZE, 50, 0);
	if (err != 0) {
		printk("Shots & impacts task creation failed: %d\n", err);
		return -1;
	}

	printk("Shots & impacts task created\n");

	err = rt_task_start(&shots_impacts, shots_impacts, 0);
	if (err != 0) {
		printk("Shots & impacts task start failed: %d\n", err);
		return -1;
	}

	return 0;

}

// Défini une nouvelle vague d'ennemis
void ennemi_init(void)
{
	int i;

	for(i = 0; i < nbEnnemis; i++)
	{
		// Active tous les ennemis
		ennemi[i].enable = 1;

		// Réinitialise les positions
		ennemi[i].x = ((i % 6)+1)*30;
		ennemi[i].y = ((i / 6)+1)*30;
	}
}

void move_player(void * cookie)
{

	int err;
	int border = 15;
	int EdgeX_left = border;
	int EdgeX_right = LCD_MAX_X - border;
	int touch = 0;
	struct ts_sample touch_info;
	int speed = 5;

	// Configuration de la tâche périodique
	if(TIMER_PERIODIC)
	{
		err = rt_task_set_periodic(&move_task, TM_NOW, PERIOD_TASK_MOVE);
		if (err != 0) {
			printk("Move task set periodic failed: %d\n", err);
			return;
		}

	}
	else
	{
		err = rt_task_set_periodic(&move_task, TM_NOW, PERIOD_TASK_MOVE*MS);
		if (err != 0) {
			printk("Move task set periodic failed: %d\n", err);
			return;
		}
	}


    while (1){

    	// Attend que l'utilisateur touche l'écran
		while(touch == 0)
		{
			rt_task_wait_period(NULL);

			if(xeno_ts_read(&touch_info, 1, O_NONBLOCK) > 0)
			{
				printk("x = %d, y = %d\n", touch_info.x, touch_info.y);
				touch = 1;

				if(touch_info.x > player[0].x)
				{
					if(player[0].x + 16 < EdgeX_right)
						player[0].x += speed;
				}
				else
				{
					if(player[0].x > EdgeX_left)
						player[0].x -= speed;
				}

				while(xeno_ts_read(&touch_info, 1, O_NONBLOCK) > 0);
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

	// Configuration de la tâche périodique
	if (TIMER_PERIODIC) {
		err = rt_task_set_periodic(&shots_impacts, TM_NOW, PERIOD_TASK_MOVE);
		if (err != 0) {
			printk("Move task set periodic failed: %d\n", err);
			return;
		}

	} else {
		err = rt_task_set_periodic(&shots_impacts, TM_NOW, PERIOD_TASK_MOVE * MS);
		if (err != 0) {
			printk("Move task set periodic failed: %d\n", err);
			return;
		}
	}
}

void game_main(void)
{
	printk("game_main()");
	rt_task_wait_period(NULL);

	if(game_init() < 0)
	{
		printk("game_init() failed");
		return;
	}



	while(player[0].enable == 1){
		rt_task_wait_period(NULL);
	}
}
