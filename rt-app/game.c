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

int game_init(void)
{
	int err;

	speed++;

	ennemi_init();

	player[0].enable = 1;
	player[0].x = LCD_MAX_X/2;
	player[0].enable = LCD_MAX_Y - 20;

	// Création de la tâche gérant le menu
	err =  rt_task_create (&refresh_task, "menu", STACK_SIZE, 50, 0);
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
void ennemi_init(void)
{
	int i;

	for(i = 0; i < nbEnnemis; i++)
	{
		// Active tous les ennemis
		ennemi[i].enable = 1;

		// Réinitialise les positions
		ennemi[i].x = (i % 6)*20;
		ennemi[i].y = ((i / 6)+1)*20;
	}
}

void game_main(void)
{
	if(game_init() < 0)
	{
		printk("game_init() failed");
		return;
	}


}
