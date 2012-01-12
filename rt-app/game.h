/*
 * game.h
 *
 *  Created on: 10 janv. 2012
 *      Author: redsuser
 */

#ifndef GAME_H_
#define GAME_H_

#define nbEnnemis 24
#define nbShotsMax 512

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
