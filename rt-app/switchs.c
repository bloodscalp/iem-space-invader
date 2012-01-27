/*
 * switchs.c
 *
 *  Created on: 19 janv. 2012
 *      Author: Christian Muller
 */

#include <native/task.h>

#include "pca9554-m.h"
#include "rt-app-m.h"
#include "game.h"
#include "switchs.h"

int SW2_event, SW3_event, SW4_event, SW5_event;
int SW2_up, SW2_up_cpt;

/*
void i2c_module_init() {
	mknod("/var/dev/i2c", S_IFCHR|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH, makedev(10,0));

	i2c_fd = open("/var/dev/i2c", O_RDWR);
}
*/

/**
 * Auteur : Christian Muller
 *
 * Tâche qui va lire l'état des switchs et lancer les fonctions associées
 */
void switch_events_handler(void *cookie) {

	int i;
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

	/* Compteur indiquant depuis combien de temps le switch 2 est appuyé */
	SW2_up_cpt = 0;

	while(1) {
		rt_task_wait_period(NULL);

		i = 0;


		/* Vérifie l'état des switchs */
		check_switch_events_once();

		/* This is a cheat! Ca ajoute une vie si on presse 3 et 5 en même temps :p */
		if(SW5_event && SW3_event) {
			SW5_event = 0;
			SW3_event = 0;

			if(player[0].lifes < 4)
			{
				player[0].lifes++;
				hp_update_leds();
			}
		}

		/* Tir automatique avec une période inférieure au tir manuel */
		if(SW2_up) {
			SW2_up = 0;

			SW2_up_cpt = (SW2_up_cpt+1) % FREQENCE_TIR_AUTO;

			if(SW2_up_cpt == 0)
				player_shots_handler();
		}

		/* Nouveau tir */
		if(SW2_event) {
			SW2_event = 0;

			player_shots_handler();
		}

		/* Switchs 3 et 4 ne font rien */
		if(SW3_event) {
			SW3_event = 0;
			//reinforcement_handler();
		}

		if(SW4_event) {
			SW4_event = 0;
		}
	}
}


void check_switch_events_once(void) {

	char buf;
	static char lastBuf;
	int err;

	char switch_change, switch_change_up;

	/* Lis la valeur des switchs */
	if((err = pca9554_read(NULL, &buf, 1, NULL)) < 0) {
		printk("i2c read error : %d\n", err);
	} else {

		/* Indique si le switch 2 est pressé */
		if((buf >> 4) & 0x8) {
			SW2_up = 1;
		}

		/* S'il y a eu un changement */
		if(buf != lastBuf) {

			/* Détécte les changements (rise) */
			switch_change = (buf ^ lastBuf) >> 4;
			switch_change_up = switch_change & (buf >> 4);

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
			lastBuf = buf;
		}
	}
}
