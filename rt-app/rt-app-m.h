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
#define PERIOD_TASK_MISSILE_ENNEMI 250
#define PERIOD_TASK_GIFT 100

#define nbEnnemis 24
#define nbVagueEnnemis 4
#define nbEnnemiParVague (nbEnnemis/nbVagueEnnemis)
#define SHOT_SPEED 2
#define DIRECTION_EST 1
#define DIRECTION_OUEST -1
#define DIRECTION_UP -SHOT_SPEED
#define DIRECTION_DOWN SHOT_SPEED
#define SHIP_SIZE 16
#define GIFT_SIZE 16
#define MISSILE_SIZE 2
#define DEFAULT_PV_ENNEMI 1
#define MOVE_MISSILE 1
#define MOVE_ENNEMI_Y 3
#define X_SPACE 4
#define Y_SPACE 4
#define xStart 10
#define yStart 10
#define ALLIED_SHIPS_SPACING 	20
#define ALLIED_SHIPS_HP 		1
#define NB_PLAYER 				3
#define STEP_GIFT 				2
#define GIFTEVERYLEVEL 2

#define CAPLVL_INITIALE			1
#define CAP_LEVEL_1				5
#define CAP_LEVEL_2				10
#define CAP_LEVEL_3				15
#define ENNEMY_HP_CAP_1			1
#define ENNEMY_HP_CAP_2			2
#define ENNEMY_HP_CAP_3			3
#define ENNEMY_HP_CAP_4			4

#define FREQENCE_TIR_AUTO		10

#define STEP_MOVE_ENNEMI 4

#define NB_MAX_SHOTS 256

#define MAX_HP 4

void __exit cleanup_module(void);

#endif /* __RT_APP_M_H__ */
