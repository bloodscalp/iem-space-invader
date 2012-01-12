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
#include "ennemi.h"
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
	int xStart = 10;
	int yStart = 10;
	int err;
	int nbEnnemiParVague = nbEnnemis / nbVagueEnnemis;

	for (i = 0; i < nbVagueEnnemis; i++) {

		for (j = 0; j < nbEnnemiParVague; j++) {
			// Active tous les ennemis
			ennemi[i * nbEnnemiParVague + j].enable = 1;
			// Réinitialise les positions
			ennemi[i * nbEnnemiParVague + j].x = xStart + (j * 18);
			ennemi[i * nbEnnemiParVague + j].y = yStart + (i * 18);
		}

	}

	// Création de la tâche gérant le rafraichissement de l'écran
	err = rt_task_create(&ennemi_task, "ennemi", STACK_SIZE, 50, 0);
	if (err != 0) {
		printk("menu task creation failed: %d\n", err);

	}

	printk("Task created\n");

	err = rt_task_start(&ennemi_task, VaisseauxEnnemi, 0);
	if (err != 0) {
		printk("menu task start failed: %d\n", err);

	}

}

void game_main(void) {
	if (game_init() < 0) {
		printk("game_init() failed");
		return;
	}

}
