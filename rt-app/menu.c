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

#define PERIOD_TASK_MENU 200

RT_TASK menu_task;

const int WinSizeY = 50;
const int WinSizeX = 130;
const int Ystart = 40;
const int Xstart = 15;
int Xaffichage = 15;
const int nbMenu = 3;
int espacement = 15;

void menu_display(void)
{

	int Xaffichage = Xstart;
	int Yaffichage = Ystart;
	int tailleDegrade = 5;

	const int nbMenu = 3;
	int i, j;

	char *menu[3] = {"New Game", "Top 10", "About"};

	// Background Black
	fb_rect_fill(0, LCD_MAX_Y-1, 0, LCD_MAX_X-1, 0);

	fb_print_string(0xFFFF, 0, "SPACE INVADERS", 60, 15);

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


int menu_select(void)
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


void menu(void* cookie)
{
	printk("Start menu\n");


	while(1){

		// Affichage du menu
		menu_display();

		// Attend que l'utilisateur touche l'écran et lance la fonction
		// correspondante à son choix
		switch(menu_select())
		{
			case 0:	printk("new_game()\n");
					break;
			case 1: printk("top10()\n");
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

