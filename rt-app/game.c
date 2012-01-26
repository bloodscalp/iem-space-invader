/*
 * game.c
 *
 *  Created on: 10 janv. 2012
 *      Author: redsuser
 */

#include <linux/module.h>


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
#include "missileEnnemi.h"
#include "gift.h"



t_player_ player[NB_PLAYER];

t_shot_ shot[NB_MAX_SHOTS];

t_shot_ shot_ennemi[NB_MAX_SHOTS];

t_ennemi_ ennemi[nbEnnemis];

t_ennemi_ ennemi_y_tab[nbEnnemis/nbVagueEnnemis];

t_gift_ gift;

unsigned int speed;

// 1 = easy, 2 = medium, 3 = hard
unsigned int difficulty;

unsigned int score;
unsigned int highScore[10];

RT_MUTEX mutex_ennemi;
RT_MUTEX mutex_shots;
RT_MUTEX mutex_score;
RT_TASK move_task, ennemi_task, shots_impacts_task, switch_events_task, refresh_task, missile_ennemi_task, score_task, gift_task;

#define PERIOD_TASK_MOVE 50

int err;
int i2c_fd;

int game_init(void) {
	int err, i;

	player[0].enable = 1;
	player[1].enable = 0;
	player[2].enable = 0;
	player[0].x = LCD_MAX_X / 2 - 8;
	player[0].y = LCD_MAX_Y - 20;
	player[0].lifes = MAX_HP;

	gift.enable = 0;

	for(i = 0; i < NB_MAX_SHOTS; i++)
	{
		shot[i].enable = 0;
	}

	for(i = 0; i < nbEnnemis; i++)
	{
		ennemi[i].enable = 1;
	}

	hp_update_leds();

	err = rt_mutex_create(&mutex_ennemi, "mutex ennemi");
	if (err != 0) {
		printk("mutex ennemi creation failed: %d\n", err);
		return -1;
	}


	err = rt_mutex_create(&mutex_shots, "mutex shots");
	if (err != 0) {
		printk("mutex shots creation failed: %d\n", err);
		return -1;
	}

	err = rt_mutex_create(&mutex_score, "mutex score");
	if (err != 0) {
		printk("mutex score creation failed: %d\n", err);
		return -1;
	}


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

	err = rt_task_set_priority(&move_task, 80);
	if (err < 0) {
		printk("Switch events task set prio failed: %d\n", err);
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

	// Création de la tâche gérant les tirs et les impacts
	err =  rt_task_create (&missile_ennemi_task, "shots_ennemi", STACK_SIZE, 50, 0);
	if (err != 0) {
		printk("Shots ennemi task creation failed: %d\n", err);
		return -1;
	}

	printk("Shots ennemi task created\n");

	err = rt_task_start(&missile_ennemi_task, missile_ennemi, 0);
	if (err != 0) {
		printk("Shots ennemi task start failed: %d\n", err);
		return -1;
	}

	// Création de la tâche gérant le gift
	err =  rt_task_create (&gift_task, "gift", STACK_SIZE, 50, 0);
	if (err != 0) {
		printk("gift task creation failed: %d\n", err);
		return -1;
	}

	printk("gift task created\n");

	err = rt_task_start(&gift_task, gift_weapon, 0);
	if (err != 0) {
		printk("gift task start failed: %d\n", err);
		return -1;
	}


	// Création de la tâche gérant les switchs
	err =  rt_task_create (&switch_events_task, "switch_events", STACK_SIZE, 50, 0);
	if (err != 0) {
		printk("Switch events task creation failed: %d\n", err);
		return -1;
	}

	printk("Switch events task created\n");

	err = rt_task_start(&switch_events_task, switch_events_handler, 0);
	if (err != 0) {
		printk("Switch events task start failed: %d\n", err);
		return -1;
	}

	err = rt_task_set_priority(&switch_events_task, 99);
	if (err < 0) {
		printk("Switch events task set prio failed: %d\n", err);
		return -1;
	}

	return 0;


}


int switchs_init(void) {


	return 0;
}

void move_player(void * cookie) {

	int i;
	int err;
	int border = 15;
	int EdgeX_left = border;
	int EdgeX_right = LCD_MAX_X - border;
	int touch = 0;
	struct ts_sample touch_info;
	int speed = 5;
	int maxLeft, maxRight;

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
				//printk("x = %d, y = %d\n", touch_info.x, touch_info.y);
				touch = 1;

				maxLeft = maxRight = 0;

				for(i=0; i<NB_PLAYER; i++) {
					if(player[i].enable) {
						if(i%2)
							maxRight = i;
						else
							maxLeft = i;
					}
				}

				if (touch_info.x > player[0].x) {
					if (player[maxRight].x + 16 < EdgeX_right) {
						for(i=0; i<NB_PLAYER; i++)
							player[i].x += speed;
					}
				} else if(touch_info.x < player[0].x) {
					if (player[maxLeft].x > EdgeX_left) {
						for(i=0; i<NB_PLAYER; i++)
							player[i].x -= speed;
					}
				}

				while (xeno_ts_read(&touch_info, 1, O_NONBLOCK) > 0);
			}
		}

		//printk("x_player = %d \n", player[0].x);

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
		err = rt_task_set_periodic(&shots_impacts_task, TM_NOW, PERIOD_TASK_SHOT);
		if (err != 0) {
			printk("Shots and impacts task set periodic failed: %d\n", err);
			return;
		}

	} else {
		err = rt_task_set_periodic(&shots_impacts_task, TM_NOW, PERIOD_TASK_SHOT * MS);
		if (err != 0) {
			printk("Shots and impacts task set periodic failed: %d\n", err);
			return;
		}
	}

	while (1) {
		rt_task_wait_period(NULL);

		/* Parcours la liste des tirs */
		for(i=0; i<NB_MAX_SHOTS; i++) {
			if(shot[i].enable == 1)
			{
				/* Fait avancer/reculer le tir s'il est enabled */
				shot[i].y += shot[i].direction;

				// Désactive le missile si celui-ci touche le bas de l ecran
				// TODO : doit etre desactive lorsque celui-ci touche un joueur
				if (shot[i].y >= EDGE_SOUTH - MISSILE_SIZE)
					shot[i].enable = 0;
				else if(shot[i].y <= EDGE_NORTH + MISSILE_SIZE)
					shot[i].enable = 0;



				// Si le shot est à la hauteur du joueur
				if((shot[i].y > (LCD_MAX_Y-20)) && (shot[i].direction == DIRECTION_DOWN))
				{
					for(j = 0; j < 3; j++)
					{
						if(player[j].enable == 1)
						{
							// S'il y a une collision avec le joueur
							if((shot[i].x > player[j].x) && (shot[i].x < (player[j].x + 16)))
							{
								//printk("Player touched\n");
								player[j].enable++;
								shot[i].enable = 0;
							}
						}
					}
				}
				// Si le shot est dans la zone ennemis
				else if((shot[i].y <= (LCD_MAX_Y-20)) && (shot[i].direction == DIRECTION_UP))
				{
					for(j = 0; j < nbEnnemis; j++)
					{
						if(ennemi[j].enable == 1)
						{
							// S'il y a une collision avec un ennemi
							if( (shot[i].x > ennemi[j].x) && (shot[i].x < (ennemi[j].x + 16))
							 && (shot[i].y > ennemi[j].y) && (shot[i].y < (ennemi[j].y + 16)) )
							{
								//printk("Ennemi n°%d touched\n", i);
								ennemi[j].pv--;

								rt_mutex_lock(&mutex_score, TM_INFINITE);
								score += speed*10;
								rt_mutex_unlock(&mutex_score);

								if(ennemi[j].pv == 0)
								{

									ennemi[j].enable++;
								}
								shot[i].enable = 0;
							}
						}
					}
				}
			}
		}
	}
}



// Actualise l'état des leds en fonction du nombre de vies du joueur
void hp_update_leds() {

	char buf;
	int hp = player[0].lifes;

	if(hp < 0)
		hp = 0;
	else if(hp > MAX_HP)
		hp = MAX_HP;


	/* Conversion int -> LEDS */
	buf = 0x0F << hp;
	buf &= 0xF0;
	buf = buf >> MAX_HP;

	if((err = pca9554_write(NULL, &buf, 1, NULL)) < 0) {
		printk("i2c write error : %d\n", err);
	}
}

void tri_score(){

	int i;
	int temp;

	for(i = 0; i < 10; i++)
	{
		if(score > highScore[i])
		{
			temp = highScore[i];
			highScore[i] = score;
			score = temp;
		}
	}

}

void player_died()
{
	int i, j;

	player[0].lifes--;
	hp_update_leds();


	for(i=1; i<NB_PLAYER; i++) {
		player[i].enable = 0;
	}

	if(player[0].lifes == 0)
		return;

	player[0].enable = 1;
	player[0].x = LCD_MAX_X / 2 - 8;
	player[0].y = LCD_MAX_Y - 20;

	for(i = 0; i < NB_MAX_SHOTS; i++)
	{
		shot[i].enable = 0;
	}

	rt_mutex_lock(&mutex_ennemi, TM_INFINITE);

	// initialisation vaisseaux ennemis
	for (i = 0; i < nbVagueEnnemis; i++) {

		for (j = 0; j < nbEnnemiParVague; j++) {

			// RÃ©initialise les positions
			ennemi[i * nbEnnemiParVague + j].x = xStart + (j * (SHIP_SIZE
					+ X_SPACE));
			ennemi[i * nbEnnemiParVague + j].y = yStart + (i * (SHIP_SIZE
					+ Y_SPACE));

		}
	}
	rt_mutex_unlock(&mutex_ennemi);
}

void level_up()
{
	int i;

	printk("LEVEL UP\n");

	speed++;

	ennemi_init();

//	for(i=1; i<NB_PLAYER; i++) {
//		player[i].enable = 0;
//	}

	for(i = 0; i < NB_MAX_SHOTS; i++)
	{
		shot[i].enable = 0;
	}

}



// Supprime toutes les tâches créées par game_init
int end_game(void)
{
	err = rt_task_delete(&refresh_task);
	if (err != 0) {
		printk("delete refresh task failed: %d\n", err);
		return -1;
	}

	err = rt_task_delete(&move_task);
	if (err != 0) {
		printk("delete move task failed: %d\n", err);
		return -1;
	}

	err = rt_task_delete(&ennemi_task);
	if (err != 0) {
		printk("delete move task failed: %d\n", err);
		return -1;
	}

	err = rt_task_delete(&shots_impacts_task);
	if (err != 0) {
		printk("delete shots_impacts task failed: %d\n", err);
		return -1;
	}

	err = rt_task_delete(&missile_ennemi_task);
	if (err != 0) {
		printk("delete shots_impacts task failed: %d\n", err);
		return -1;
	}

	err = rt_task_delete(&switch_events_task);
	if (err != 0) {
		printk("delete switch_events task failed: %d\n", err);
		return -1;
	}

	err = rt_task_delete(&gift_task);
	if (err != 0) {
		printk("delete gift task failed: %d\n", err);
		return -1;
	}

	err = rt_mutex_delete(&mutex_ennemi);
	if (err != 0) {
		printk("delete mutex ennemi failed: %d\n", err);
		return -1;
	}

	err = rt_mutex_delete(&mutex_shots);
	if (err != 0) {
			printk("delete mutex shots failed: %d\n", err);
			return -1;
	}

	err = rt_mutex_delete(&mutex_score);
	if (err != 0) {
			printk("delete mutex score failed: %d\n", err);
			return -1;
	}

	return 0;

}


void game_main(void) {

	int i;
	int sum;

	if (game_init() < 0) {
		printk("game_init() failed");
		return;
	}

	while(player[0].lifes > 0)
	{
		rt_task_wait_period(NULL);

		if(player[0].enable == 0)
		{
			//printk("player died\n");
			player_died();
			//printk("player lifes: %d\n", player[0].lifes);
		}

		sum = 0;

		for(i = 0; i < nbEnnemis; i++)
		{
			sum += ennemi[i].enable;
		}

		if(sum == 0)
		{
			level_up();
		}

	}

	tri_score();

	if (end_game() < 0) {
		printk("end_game() failed");
		return;
	}

}

void reinforcement_handler() {
	int i;

	/* Pour chaque vaisseau allié */
	for(i=1; i<NB_PLAYER; i++) {
		/* L'active */
		player[i].enable = 1;
		player[i].lifes = ALLIED_SHIPS_HP;
		player[i].y = player[0].y;

		/* Le décale à droite/gauche du vaisseau du joueur */
		if(i%2)
			player[i].x = player[0].x+SHIP_SIZE*i/2+ALLIED_SHIPS_SPACING*i/2;
		else
			player[i].x = player[0].x-ALLIED_SHIPS_SPACING*i/2;
	}
}

void player_shots_handler() {
	int i, k;

	/* Pour chaque vaisseau actif */
	for(k=0; k<NB_PLAYER; k++) {

		/* S'il n'est pas actif, passe au suivant */
		if(!player[k].enable)
			continue;

		/* Parcours le tableau des tirs */
		for (i = 0; i < NB_MAX_SHOTS; i++) {

			rt_mutex_lock(&mutex_shots, TM_INFINITE);

			/* Si le tir courant est inactif */
			if (shot[i].enable == 0) {
				/* L'initialise et l'active */
				shot[i].x = player[k].x + SHIP_SIZE / 2;
				shot[i].y = player[k].y;
				shot[i].direction = DIRECTION_UP; // Moves up
				shot[i].enable = 1;
				rt_mutex_unlock(&mutex_shots);
				break;
			} else {
				rt_mutex_unlock(&mutex_shots);
			}
		}
	}
}
