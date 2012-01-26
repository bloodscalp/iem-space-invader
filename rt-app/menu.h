/*
 * Nom : menu.h
 *
 * Auteurs : Sylvain Villet & Florent Duployer
 *
 */
extern RT_TASK menu_task;

int menu_select(int nbMenu);
void new_game(void);
void highscore(void);
void about(void);

extern void menu(void* cookie);
