

#include "lcdlib.h"



int init()
{
	fb_init();

	return 0;
}

int menu()
{

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


	return 0;

}
