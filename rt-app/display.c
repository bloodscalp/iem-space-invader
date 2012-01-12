/*
 * display.c
 *
 *  Created on: 12 janv. 2012
 *      Author: redsuser
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

#define PERIOD_TASK_REFRESH 40

RT_TASK refresh_task;

void refresh(void)
{

	int err;
	int i;

	// Configuration de la tâche périodique
	if (TIMER_PERIODIC)
	{
		err = rt_task_set_periodic(&refresh_task, TM_NOW, PERIOD_TASK_REFRESH);
		if (err != 0) {
			printk("Menu task set periodic failed: %d\n", err);
			return;
		}

	}
	else
	{
		err = rt_task_set_periodic(&refresh_task, TM_NOW, PERIOD_TASK_REFRESH*MS);
		if (err != 0) {
			printk("Menu task set periodic failed: %d\n", err);
			return;
		}
	}

    while (1){

    }


}
