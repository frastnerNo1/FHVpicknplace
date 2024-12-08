/*
 * z_axis.h
 *
 * Created: 18.09.2024 09:32:58
 *  Author: floro
 */ 


#ifndef Z_AXIS_H_
#define Z_AXIS_H_

#include "main.h"

/* The following defines specify the heights for every position in steps from upper dead end position. */

/* One millimeter is ~ 123 steps */
#define OFFSET                          0

#define PICK_HEIGHT_STEPS                  6800 + OFFSET
#define PLACE_HEIGHT_STEPS                 4300 + OFFSET
#define TOOL_GRAB_HEIGHT_STEPS             11400 + OFFSET
#define TOOL_DROP_HEIGHT_STEPS             10000 + OFFSET
#define SOAK_HEIGHT_STEPS                  10300 + OFFSET
#define STAMP_HEIGHT_STEPS                 3700 + OFFSET
#define CLOSE_HEIGHT_STEPS                 7200 + OFFSET
#define TRAVEL_HEIGHT_STEPS                700 + OFFSET

/* Z axis remains for the wait time at each position. */
#define WAIT_TIME_ms                       1000

/* Target force in millinewton for the stamp. */
#define SOAK_FORCE_mN                   (int16_t)5000
#define STAMP_FORCE_mN                  (int16_t)5000

void z_axis_home(void);

void z_axis_pick_sample(void);

void z_axis_place_sample(void);

void z_axis_stamp(void);

void z_axis_soak_stamp(void);

void z_axis_close_lid(void);

void z_axis_change_tool(void);


#endif /* Z_AXIS_H_ */