/*
 * menu.h
 *
 *  Created on: 20 déc. 2011
 *      Author: redsuser
 */




extern RT_TASK menu_task;

int menu_select(int nbMenu);
void new_game(void);
void highscore(void);
void about(void);

extern void menu(void* cookie);
