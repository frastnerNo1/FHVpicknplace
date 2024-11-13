/*
 * z_axis.h
 *
 * Created: 18.09.2024 09:32:58
 *  Author: floro
 */ 


#ifndef Z_AXIS_H_
#define Z_AXIS_H_

#include "main.h"

/* The following defines specify the heights for every position in mm from upper dead end position. */
#define PICK_HEIGHT_mm                  150
#define PLACE_HEIGHT_mm                 100
#define TOOL_GRAB_HEIGHT_mm             100
#define TOOL_DROP_HEIGHT_mm             100
#define SOAK_HEIGHT_mm                  100
#define STAMP_HEIGHT_mm                 50
#define CLOSE_HEIGHT_mm                 50
#define TRAVEL_HEIGHT_mm                5

/* Z axis remains for the wait time at each position. */
#define WAIT_TIME_ms                    2000

/* Target force in millinewton for the stamp. */
#define SOAK_FORCE_mN                   (int16_t)1000
#define STAMP_FORCE_mN                  (int16_t)1000

void z_axis_home(void);

void z_axis_pick_sample(void);

void z_axis_place_sample(void);

void z_axis_stamp(void);

void z_axis_soak_stamp(void);

void z_axis_close_lid(void);

void z_axis_change_tool(void);


#endif /* Z_AXIS_H_ */