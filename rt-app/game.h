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
#define BORD_EST LCD_MAX_X
#define BORD_OUEST 0

#define PERIOD_TASK_ENNEMI 40

RT_TASK ennemi_task;


typedef struct t_ennemi {
	int				enable;
	int				x;
	int				y;
	unsigned int	pv;
} t_ennemi_;

typedef struct t_player {
	int				enable;
	int				x;
	int				y;
	unsigned int	lifes;
} t_player_;

typedef struct t_shot {
	int		enable;
	int		x;
	int		y;
	int		direction; // Up = -1, down = 1
} t_shot_;

extern t_ennemi_ ennemi[nbEnnemis];
extern t_player_ player[3];
extern t_shot_ shot[nbShotsMax];
extern unsigned int difficulty;



void game_main(void);
void ennemi_init(void);

#endif /* GAME_H_ */
