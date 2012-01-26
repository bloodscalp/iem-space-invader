/*
 * Nom : menu.c
 *
 * Auteur : Sylvain Villet & Duployer Florent
 *
 * But : Contient les différentes fonctions propres au menu, tels
 * 		 que l'affichage, la gestion de la saisie, et les affichages
 * 		 des highscores et du About.
 *
 */

#include <linux/module.h>

#include <native/task.h>
#include <native/intr.h>
#include <native/event.h>
#include <native/alarm.h>
#include <native/timer.h>
#include <native/mutex.h>

#include "lcdlib.h"
#include "rt-app-m.h"
#include "xeno-ts.h"
#include "menu.h"
#include "game.h"

#define PERIOD_TASK_MENU 200

RT_TASK menu_task;

const int WinSizeY = 50;
const int WinSizeX = 130;
const int Ystart = 40;
const int Xstart = 15;
int Xaffichage = 15;
int espacement = 15;

/*
 * Auteur : Sylvain Villet & Duployer Florent
 *
 * But : Affichage du menu, en fonction d'une liste de menu en argument,
 * 		 du nombre de menu à afficher, et d'un titre de menu
 */
void menu_display(char ** menu, const int nbMenu, char* title)
{
	int Xaffichage = Xstart;
	int Yaffichage = Ystart;
	int tailleDegrade = 5;

	int i, j;

	// Background Black
	fb_rect_fill(0, LCD_MAX_Y-1, 0, LCD_MAX_X-1, 0);

	// Affichage du titre en haut de l'écran
	fb_print_string(0xFFFF, 0, title, 60, 15);

	// Affichage du menu en fonction du nombre de menu désiré
	for(i = 0; i < nbMenu; i++)
	{
		for(j = 0x1F; j > 0; j--)
		{
			fb_rect_fill(Yaffichage, Yaffichage+WinSizeY, Xaffichage, Xaffichage + tailleDegrade, BLUE(j));
			Xaffichage += tailleDegrade;
		}

		Xaffichage = Xstart;
		fb_print_string(0xFFFF, BLUE(0x1F), menu[i], Xstart+10, Yaffichage + (WinSizeY/2) - 4);
		Yaffichage += WinSizeY + espacement;
	}
	return;
}

/*
 * Auteur : Sylvain Villet
 *
 * But : Fonction qui retourne le menu sélectionner avec le touchscreen
 */
int menu_select(int nbMenu)
{
	int err;
	int touch = 0;
	int i;
	struct ts_sample touch_info;

	// Configuration de la tÃ¢che pÃ©riodique
	if (TIMER_PERIODIC)
	{
		err = rt_task_set_periodic(&menu_task, TM_NOW, PERIOD_TASK_MENU);
		if (err != 0) {
			printk("Menu task set periodic failed: %d\n", err);
			return -1;
		}
	}
	else
	{
		err = rt_task_set_periodic(&menu_task, TM_NOW, PERIOD_TASK_MENU*MS);
		if (err != 0) {
			printk("Menu task set periodic failed: %d\n", err);
			return -1;
		}
	}

    while (1){
		// Attend que l'utilisateur touche l'Ã©cran
		while(touch == 0)
		{
			rt_task_wait_period(NULL);

			if(xeno_ts_read(&touch_info, 1, O_NONBLOCK) > 0)
			{
				printk("x = %d, y = %d\n", touch_info.x, touch_info.y);
				touch = 1;

				while(xeno_ts_read(&touch_info, 1, O_NONBLOCK) > 0);
			}
		}

		if((touch_info.x > Xstart) && (touch_info.x < Xstart+WinSizeX))
		{
			for(i = 0; i < nbMenu; i++)
			{

				if((touch_info.y > (Ystart + i*(WinSizeY+espacement)))
					&& (touch_info.y < (Ystart + WinSizeY*(1+i) + espacement*i)))
				{
					return i;
				}
			}
		}
		touch = 0;
    }

	return -1;
}

/*
 * Auteur : Sylvain Villet
 *
 * But : Affiche du menu des difficultés, et lancement du jeu dès qu'il
 * 		 y'a acquisition de la difficulté
 */
void new_game(void)
{
	char *menu[3] = {"Easy", "Medium", "Hard"};
	char *titre = "DIFFICULTY";

	difficulty = 0;

	// Affichage du menu
	menu_display(menu, 3, titre);

	rt_task_wait_period(NULL);

	while(difficulty == 0){

		// Attend que l'utilisateur touche l'Ã©cran et lance la fonction
		// correspondante Ã  son choix
		switch(menu_select(3))
		{
			case 0:	printk("Easy\n");
					difficulty = 1;
					speed = 1;
					break;
			case 1: printk("Medium\n");
					difficulty = 2;
					speed = 5;
					break;
			case 2: printk("Hard\n");
					difficulty = 3;
					speed = 10;
					break;
			default:
					printk("error menu_select\n");
					break;
		}
	}

	game_main();

	rt_task_wait_period(NULL);
}

/*
 * Auteur : Duployer Florent
 *
 * But : Affichage des highscores (stockés en RAM), avec option de retour
 * 		 au menu principal
 */
void highscore(void)
{
	char *menu[1] = {"Return to main menu"};
	char *titre = "HIGH SCORES";

	int retour = 0;
	int i;
	int x = Xstart;
	int y = Ystart + WinSizeY + 10;

	int sizeWinNo = 18;

	char affichage[64];

	// Affichage du menu
	menu_display(menu, 1, titre);

	// Affichage des scores
	for(i = 0; i < 5; i++)
	{
		// Affichage du NumÃ©ro du score
		fb_rect_fill(y, y+sizeWinNo, x+1, x + sizeWinNo, 0xFFFF);
		sprintf(affichage, "%d", i+1);
		fb_print_string(0, 0xFFFF, affichage, x + 6, y + 6);

		// Affichage du scores
		sprintf(affichage, "%d", highScore[i]);
		fb_print_string(RED(0x1F), 0, affichage, x + sizeWinNo + 6, y + 6);

		y += sizeWinNo + 8;
	}

	while(retour == 0){

		// Attend que l'utilisateur touche l'Ã©cran et lance la fonction
		// correspondante Ã  son choix
		switch(menu_select(1))
		{
			case 0:	retour = 1;
					break;
			default:
					printk("error menu_select\n");
					break;
		}
	}

	return;
}
/*
 * Auteur : Duployer Florent
 *
 * But : Affichage de quelques informations concernant le projet
 *		 Option de retour au menu principale.
 */
void about(void)
{
	char *menu[1] = {"Return to main menu"};
	char *titre = "ABOUT";

	int retour = 0;
	int x = Xstart;
	int y = Ystart + WinSizeY + 15;

	// Affichage du menu
	menu_display(menu, 1, titre);

	fb_print_string(0xFFFF, 0, "Developpers :", x, y);
	x += 20; y += 20;
	fb_print_string(RED(0x1F), 0,"Christian Muller", x, y);
    y += 10;
	fb_print_string(RED(0x1F), 0,"Florent Duployer", x, y);
    y += 10;
	fb_print_string(RED(0x1F), 0,"Romain Failletaz", x, y);
    y += 10;
	fb_print_string(RED(0x1F), 0,"Sylvain Villet", x, y);

	x = Xstart;
	y += 20;

	fb_print_string(0xFFFF, 0,"Project made for", x, y);
	y += 10;
	fb_print_string(0xFFFF, 0,"the IEM course", x, y);
	y += 20;
	fb_print_string(0xFFFF, 0, "Professor :", x, y);
	x += 20; y += 20;
	fb_print_string(RED(0x1F), 0,"Daniel Rossier", x, y);
	x = Xstart;
    y += 20;
	fb_print_string(0xFFFF, 0, "Assist :", x, y);
	x += 20; y += 20;
	fb_print_string(RED(0x1F), 0,"Lionel Sambuc", x, y);

	while(retour == 0){

		// Attend que l'utilisateur touche l'Ã©cran et lance la fonction
		// correspondante Ã  son choix
		switch(menu_select(1))
		{
			case 0:	retour = 1;
					break;
			default:
					printk("error menu_select\n");
					break;
		}
	}

	return;
}

/*
 * Auteur : Sylvain Villet & Duployer Florent
 *
 * But : Fonction du menu principale
 */
void menu(void* cookie)
{
	char *menu[3] = {"New Game", "High Scores", "About"};
	char *titre = "SPACE INVADERS";

	// Reset des highscores au dÃ©marrage
	int i;
	for(i = 0; i < 10; i++)
	{
		highScore[i] = 0;
	}

	//	printk("Start menu\n");

	while(1){

		// Affichage du menu
		menu_display(menu, 3, titre);

		// Attend que l'utilisateur touche l'Ã©cran et lance la fonction
		// correspondante Ã  son choix
		switch(menu_select(3))
		{
			case 0:	// printk("new_game()\n");
					new_game();
					break;
			case 1: // printk("highscore()\n");
					highscore();
					break;
			case 2: // printk("About()\n");
					about();
					break;
			default:
					//printk("error menu_select\n");
					break;
		}

	}

	return;

}

