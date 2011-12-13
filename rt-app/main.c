

#include "lcdlib.h"



int init()
{
	fb_init();

	return 0;
}

int menu()
{
	fb_rect_fill(0, 319, 0, 239, 0);
	fb_print_string(255, 0, "SPACE INVADERS", 50, 140);

	return 0;
}


int main(int argc, char **argv) {

	if(init()<0)
	{
		printf("Initialisation error.");
		return -1;
	}

	switch(menu())
	{
		case 0:	//new_game();
				break;
		case 1: //about();
				break;
		others:
	}

	while(1);


	return 0;

}

