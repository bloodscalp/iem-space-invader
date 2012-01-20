#ifndef __RT_APP_M_H__
#define __RT_APP_M_H__
/******************************************************************************
 *                           Xenomai Definitions
 *****************************************************************************/
#define TIMER_PERIODIC	0           /**< 1:Periodic timer, 0:Aperiodic timer */

#define STACK_SIZE		8192        /**< Default stack size */
#define MS				1000000		/**< 1 ms in ns */


/******************************************************************************
 *                           Game parameters
 *****************************************************************************/
#define PERIOD_TASK_SWITCHS 50
#define PERIOD_TASK_ENNEMI 	40
#define PERIOD_TASK_REFRESH 40
#define PERIOD_TASK_MISSILE_ENNEMI 40

#define nbEnnemis 24
#define nbVagueEnnemis 4
#define DIRECTION_EST 1
#define DIRECTION_OUEST -1
#define DIRECTION_UP -1
#define DIRECTION_DOWN 1
#define SHIP_SIZE 16
#define DEFAULT_PV_ENNEMI 1
#define MOVE_MISSILE 1
#define X_SPACE 4
#define Y_SPACE 4
#define xStart 10
#define yStart 10

#define STEP_MOVE_ENNEMI 4

#define NB_MAX_SHOTS 256

#define MAX_HP 4

void __exit cleanup_module(void);

#endif /* __RT_APP_M_H__ */
