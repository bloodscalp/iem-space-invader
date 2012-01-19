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

void menu_display(char ** menu, const int nbMenu, char* title)
{

	int Xaffichage = Xstart;
	int Yaffichage = Ystart;
	int tailleDegrade = 5;

	int i, j;

	// Background Black
	fb_rect_fill(0, LCD_MAX_Y-1, 0, LCD_MAX_X-1, 0);

	fb_print_string(0xFFFF, 0, title, 60, 15);

	// Affichage du menu
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


int menu_select(int nbMenu)
{
	int err;
	int touch = 0;
	int i;
	struct ts_sample touch_info;

	// Configuration de la tâche périodique
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
		// Attend que l'utilisateur touche l'écran
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

void new_game(void)
{
	char *menu[3] = {"Easy", "Medium", "Hard"};
	char *titre = "DIFFICULTY";


	difficulty = 0;

	// Affichage du menu
	menu_display(menu, 3, titre);

	rt_task_wait_period(NULL);

	while(difficulty == 0){

		// Attend que l'utilisateur touche l'écran et lance la fonction
		// correspondante à son choix
		switch(menu_select(3))
		{
			case 0:	printk("Easy\n");
					difficulty = 1;
					break;
			case 1: printk("Medium\n");
					difficulty = 2;
					break;
			case 2: printk("Hard\n");
					difficulty = 3;
					break;
			default:
					printk("error menu_select\n");
					break;
		}
	}

	game_main();

	rt_task_wait_period(NULL);


}


void top10(void)
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
	for(i = 0; i < 10; i++)
	{
		// Affichage du Numéro du score
		fb_rect_fill(y, y+sizeWinNo, x+1, x + sizeWinNo, 0xFFFF);
		sprintf(affichage, "%d", i+1);
		fb_print_string(0, 0xFFFF, affichage, x + 6, y + 6);

		// Affichage du scores
		sprintf(affichage, "%d", highScore[i]);
		fb_print_string(RED(0x1F), 0, affichage, x + sizeWinNo + 6, y + 6);

		y += sizeWinNo + 3;

	}


	while(retour == 0){

		// Attend que l'utilisateur touche l'écran et lance la fonction
		// correspondante à son choix
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


void menu(void* cookie)
{
	char *menu[3] = {"New Game", "Top 10", "About"};
	char *titre = "SPACE INVADERS";

	// Reset des highscores au démarrage
	int i;
	for(i = 0; i < 10; i++)
	{
		highScore[i] = 0;
	}


	printk("Start menu\n");

	while(1){

		// Affichage du menu
		menu_display(menu, 3, titre);

		// Attend que l'utilisateur touche l'écran et lance la fonction
		// correspondante à son choix
		switch(menu_select(3))
		{
			case 0:	printk("new_game()\n");
					new_game();
					break;
			case 1: printk("top10()\n");
					top10();
					break;
			case 2: printk("About()\n");
					break;
			default:
					printk("error menu_select\n");
					break;
		}

	}

	return;

}

