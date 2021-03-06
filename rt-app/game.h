/*
 * game.h
 *
 *  Created on: 10 janv. 2012
 *      Author: redsuser
 */

#ifndef GAME_H_
#define GAME_H_

#define EDGE_EAST LCD_MAX_X
#define EDGE_WEST 0
#define EDGE_NORTH 0
#define EDGE_SOUTH LCD_MAX_Y


#include <native/task.h>
#include <native/intr.h>
#include <native/event.h>
#include <native/alarm.h>
#include <native/timer.h>
#include <native/mutex.h>

typedef enum {
	false = 0, true = 1
} bool;

typedef struct t_ennemi {
	int enable;
	int x;
	int y;
	unsigned int pv;
} t_ennemi_;

typedef struct t_player {
	int enable;
	int x;
	int y;
	unsigned int lifes;
} t_player_;

typedef struct t_shot {
	int enable;
	int x;
	int y;
	int direction;
} t_shot_;

typedef struct t_gift {
	int enable;
	int x;
	int y;
} t_gift_;

extern t_ennemi_ ennemi[nbEnnemis];
extern t_ennemi_ ennemi_y_tab[nbEnnemis/nbVagueEnnemis];
extern t_player_ player[NB_PLAYER];
extern t_shot_ shot[NB_MAX_SHOTS];
extern t_shot_ shot_ennemi[NB_MAX_SHOTS];
extern t_gift_ gift;

extern unsigned int difficulty;
extern unsigned int score;
extern unsigned int speed;
extern unsigned int highScore[10];


void game_main(void);
void init_ennemi_init(void);
void move_player(void *cookie);
void move_ennemi(void* cookie);
void show_ennemi(void);
bool detectShitEnable(void);
void tri_score(void);
void player_died(void);
void level_up(void);
void reinforcement_handler(void);
void player_shots_handler(void);

extern RT_MUTEX mutex_ennemi, mutex_shots, mutex_score;
extern RT_TASK move_task, ennemi_task, shots_impacts_task, switch_events_task, refresh_task, missile_ennemi_task, gift_task;



void shots_impacts(void * cookie);


void hp_update_leds(void);


#endif /* GAME_H_ */
