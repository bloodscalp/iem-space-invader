/*
 * game.h
 *
 *  Created on: 10 janv. 2012
 *      Author: redsuser
 */

#ifndef GAME_H_
#define GAME_H_

#define nbEnnemis 24		// doit être un multiple du nombre de vagues ennemies
#define nbShotsMax 512
#define nbVagueEnnemis 4
#define DIRECTION_EST 1
#define DIRECTION_OUEST -1
#define EDGE_EAST LCD_MAX_X
#define EDGE_WEST 0
#define SHIT_SIZE 16
#define DEFAULT_PV_ENNEMI 20
#define X_SPACE 4
#define Y_SPACE 4
#define xStart 10
#define yStart 10

typedef enum {
	false = 0, true = 1
} bool;

#define STEP_MOVE_ENNEMI 4

#define PERIOD_TASK_ENNEMI 40


#define PERIOD_TASK_REFRESH 40

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
	int direction; // Up = -1, down = 1
} t_shot_;

extern t_ennemi_ ennemi[nbEnnemis];
extern t_player_ player[3];
extern t_shot_ shot[nbShotsMax];
extern unsigned int difficulty;

void game_main(void);
void init_ennemi_init(void);
void move_player(void *cookie);
void move_ennemi(void* cookie);
void show_ennemi(void);
bool detectShitEnable(void);

extern RT_MUTEX mutex_ennemi;

void shots_impacts(void * cookie);


#endif /* GAME_H_ */
