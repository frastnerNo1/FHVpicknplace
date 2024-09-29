/*
 * z_axis.c
 *
 * Created: 18.09.2024 09:32:43
 *  Author: floro
 */ 

/* 
 * Here do all commands regarding the z-axis. This includes movement, triggering actuators like the magnet and checking the force value.
 * Therefore other functions from drv_ctrl and force_sense are used.
 */

#include "z_axis.h"
#include "force_sense.h"
#include "drv_ctrl.h"
#include "plc_com.h"

typedef enum {
	pick_tool,
	stamp_tool
	} tools;
	
static tools sTool;

static void z_axis_grab_tool(void);
static void z_axis_drop_tool(void);

     /*
	  * Initialize the Z_axis: move to homeposition and calibrate the force sensor.
	  */
void z_axis_home() {
	
	drv_ctrl_home();
	force_sense_calibrate();
	sTool = pick_tool;
	set_state(success);
}

    /*
	 * Move down to pick position, switch on magnet and move up to travel position, then give the success message to the PLC.
	 */
void z_axis_pick_sample(){
	
	if(sTool == stamp_tool) {
		plc_com_error(e_wrong_tool);
		return;
	}
	
	drv_ctrl_moveto(PICK_HEIGHT_mm);
	port_pin_set_output_level(MAGNET_SWITCH_PIN, true);
	delay_ms(WAIT_TIME_ms);
	drv_ctrl_moveto(TRAVEL_HEIGHT_mm);
	set_state(success);
}

    /*
	 * Move down to place position, switch of magnet and move up to travel position, then give the success message to the PLC.
	 */
void z_axis_place_sample(){
	
	if(sTool == stamp_tool) {
		plc_com_error(e_wrong_tool);
		return;
	}
	
	drv_ctrl_moveto(PLACE_HEIGHT_mm);
	port_pin_set_output_level(MAGNET_SWITCH_PIN, false);
	delay_ms(WAIT_TIME_ms);
	drv_ctrl_moveto(TRAVEL_HEIGHT_mm);
	set_state(success);
}

    /* 
	 * Move down to the pad till the required force is reached. Then move to the travel position.
	 */
void z_axis_soak_stamp() {
	
	if(sTool == pick_tool) {
		plc_com_error(e_wrong_tool);
		return;
	}
	
	drv_ctrl_moveto(SOAK_HEIGHT_mm);
	drv_ctrl_move_till_force(SOAK_FORCE_mN);
	delay_ms(WAIT_TIME_ms);
	drv_ctrl_moveto(TRAVEL_HEIGHT_mm);
	set_state(success);
}

    /* 
	 * Move down to the box till the required force is reached. Then move to the travel position.
	 */
void z_axis_stamp() {
	
	if(sTool == pick_tool) {
		plc_com_error(e_wrong_tool);
		return;
	}
	
	drv_ctrl_moveto(STAMP_HEIGHT_mm);
	drv_ctrl_move_till_force(STAMP_FORCE_mN);
	delay_ms(WAIT_TIME_ms);
	drv_ctrl_moveto(TRAVEL_HEIGHT_mm);
	set_state(success);
}

    /*
	 * Move down to the tool, switch on magnet, move to travel position and calibrate the force sensor.
	 */
static void z_axis_grab_tool(void) {
	
	port_pin_set_output_level(MAGNET_SWITCH_PIN, true);
	drv_ctrl_moveto(TOOL_GRAB_HEIGHT_mm);
	delay_ms(WAIT_TIME_ms);
	drv_ctrl_moveto(TRAVEL_HEIGHT_mm);
	force_sense_calibrate();
}

    /*
	 * Move down to the tool holder, switch off magnet, move to travel position and calibrate the force sensor.
	 */
static void z_axis_drop_tool(void) {
	
	drv_ctrl_moveto(TOOL_DROP_HEIGHT_mm);
	port_pin_set_output_level(MAGNET_SWITCH_PIN, false);
	delay_ms(WAIT_TIME_ms);
	drv_ctrl_moveto(TRAVEL_HEIGHT_mm);
	force_sense_calibrate();
}

    /* 
	 * Here the sequence for closing the lid is implementer. This sequence has to be defined, based on the mechanical aspects of the machine.
	 */
void z_axis_close_lid() {
	
	if(sTool == stamp_tool) {
		plc_com_error(e_wrong_tool);
		return;
	}
	
	drv_ctrl_moveto(CLOSE_HEIGHT_mm);
	delay_ms(WAIT_TIME_ms);
	drv_ctrl_moveto(TRAVEL_HEIGHT_mm);
	set_state(success);
}

void z_axis_change_tool() {
	
	if(sTool == pick_tool){
		z_axis_grab_tool();
		sTool = stamp_tool;
	} else {
		z_axis_drop_tool();
		sTool = pick_tool;
	}
	set_state(success);
}
