/*
 * switchs.c
 *
 *  Created on: 19 janv. 2012
 *      Author: redsuser
 */

#include <native/task.h>

#include "pca9554-m.h"
#include "rt-app-m.h"
#include "game.h"
#include "switchs.h"

int SW2_event, SW3_event, SW4_event, SW5_event;

/*
void i2c_module_init() {
	mknod("/var/dev/i2c", S_IFCHR|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH, makedev(10,0));

	i2c_fd = open("/var/dev/i2c", O_RDWR);
}
*/

void switch_events_handler(void *cookie) {

	int i;
	int ctr;
	int err;

	/* Configuration de la tâche périodique */
	if (TIMER_PERIODIC) {
		err = rt_task_set_periodic(&switch_events_task, TM_NOW, PERIOD_TASK_SWITCHS);
		if (err != 0) {
			printk("Switch events task set periodic failed: %d\n", err);
			return;
		}

	} else {
		err = rt_task_set_periodic(&switch_events_task, TM_NOW, PERIOD_TASK_SWITCHS * MS);
		if (err != 0) {
			printk("Switch events task set periodic failed: %d\n", err);
			return;
		}
	}

	while(1) {
		rt_task_wait_period(NULL);

		i = 0;
		ctr = 0;

		/* Vérifie l'état des switchs */
		check_switch_events_once();

		/* This is a cheat :p */
		if(SW5_event && SW3_event) {
			SW5_event = 0;
			SW3_event = 0;

			player[0].lifes++;
			hp_update_leds();

		}

		/* Nouveau tir */
		if(SW2_event) {
			printk("Shot! \n");

			SW2_event = 0;

			/* Parcours le tableau des tirs */
			while(1) {
				ctr++;

				/* Si le tir courant est inactif */
				if(shot[i].enable == 0) {
					/* L'initialise et l'active */
					shot[i].x = player[1].x + SHIP_SIZE/2;
					shot[i].y = player[1].y;
					shot[i].direction = DIRECTION_UP; // Moves up
					shot[i].enable = 1;
					break;
				} else {
					/* Pase au tir suivant */
					i = ((i+1) % NB_MAX_SHOTS);

					/* Si on a parcouru plus de 2 fois le tableau */
					if(ctr > NB_MAX_SHOTS*2)
						/* Annule le tir pour éviter de planter les suivants */
						break;
				}
			}
		}

		if(SW3_event) {
			SW3_event = 0;

			if(player[0].lifes < 4) {
				player[0].lifes++;
				hp_update_leds();
			}
		}

		if(SW4_event) {
			SW4_event = 0;

			if(player[0].lifes > 0) {
				player[0].lifes--;
				hp_update_leds();
			}
		}
	}
}


void check_switch_events_once(void) {

	char buf[1];
	static char lastBuf[1];
	int err;

	char switch_change, switch_change_up;

	/* Lis la valeur des switchs */
	if((err = pca9554_read(NULL, buf, 1, NULL)) < 0) {
		printk("i2c read error : %d\n", err);
	} else if(buf[0] != lastBuf[0]) {

		/* Analyse s'il y a eu un changement (rise) */
		switch_change = (buf[0] ^ lastBuf[0]) >> 4;
		switch_change_up = switch_change & ~(buf[0] >> 4);


		/* Met à jour les event flags selon les switchs */
		if(switch_change_up & 0x1) {
			SW5_event = 1;
		}

		if(switch_change_up & 0x2) {
			SW4_event = 1;
		}

		if(switch_change_up & 0x4) {
			SW3_event = 1;
		}

		if(switch_change_up & 0x8) {
			SW2_event = 1;
		}

		/* Mémorise l'état présent */
		lastBuf[0] = buf[0];
	}
}
