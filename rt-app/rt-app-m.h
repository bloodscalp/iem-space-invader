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

#define STEP_MOVE_ENNEMI 4

#define NB_MAX_SHOTS 256

void __exit cleanup_module(void);

#endif /* __RT_APP_M_H__ */
