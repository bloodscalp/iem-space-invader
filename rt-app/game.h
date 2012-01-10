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

struct t_ennemi {
	int				x;
	int				y;
	unsigned int	pv;
} ennemi[nbEnnemis];

struct t_player {
	int				enable;
	int				x;
	int				y;
	unsigned int	lifes;
} player[3];

struct t_shot {
	int		enable;
	int		x;
	int		y;
	int		direction; // Up = -1, down = 1
} shot[nbShotsMax];

unsigned int speed;

// 1 = easy, 2 = medium, 3 = hard
unsigned int difficulty;

unsigned int score;


#endif /* GAME_H_ */
