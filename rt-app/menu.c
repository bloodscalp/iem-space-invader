#include "lcdlib.h"
#include <linux/module.h>


int menu_select(void)
{
	fb_rect_fill(0, LCD_MAX_Y-1, 0, LCD_MAX_X-1, 0);
	fb_print_string(255, 0, "SPACE INVADERS", 50, 150);



	return 0;
}


void menu(void* cookie)
{
	printk("Start menu\n");


	switch(menu_select())
	{
		case 0:	//new_game();
				break;
		case 1: //about();
				break;
		default:
				break;
	}

	printk("End menu\n");




	return;

}

