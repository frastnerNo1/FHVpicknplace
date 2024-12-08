/*
 * z_axis.c
 *
 * Created: 18.09.2024 09:32:43
 *  Author: floro
 */ 

/* 
 * All functions for z-axis movement are defined here. This includes movement, triggering actuators like the magnet and checking the force value.
 * Therefore other functions from drv_ctrl and force_sense are used. No hardware interaction is used in this functions.
 */

#include "z_axis.h"
#include "force_sense.h"
#include "drv_ctrl.h"
#include "plc_com.h"

typedef enum tools{
	pick_tool,
	stamp_tool
	} Tool_t;
	
static Tool_t sTool;

/* Private FPT for tool changes. */
static uint8_t z_axis_grab_tool(void);
static uint8_t z_axis_drop_tool(void);

     /*
	  * @brief: Initialize the Z_axis: move to home position and calibrate the force sensor. Tool is set to pick tool.
	  */
void z_axis_home() {

    #if LOGS >= 1
    rprintf("LOG: Z_AXIS_HOMING\r\n");
    #endif
	
	drv_ctrl_home();
    delay_ms(WAIT_TIME_ms);
	force_sense_calibrate();
	sTool = pick_tool;
	set_state(success);
}

    /*
	 * @brief: Move down to pick position, switch on magnet and move up to travel position, then send the success message to the PLC.
     * If out of position error occurred, init homing and report error.
	 */
void z_axis_pick_sample(){

    uint8_t status = 0;

    #if LOGS >= 1
    rprintf("LOG: Z_PICK_SAMPLE\r\n");
    #endif
	
	if(sTool == stamp_tool) {
		plc_com_error(e_wrong_tool);
		return;
	}
	
	status = drv_ctrl_moveto(PICK_HEIGHT_STEPS);
	port_pin_set_output_level(MAGNET_SWITCH_PIN, true);
	delay_ms(WAIT_TIME_ms);
	status = drv_ctrl_moveto(TRAVEL_HEIGHT_STEPS);

    if(status == 1){
    	drv_ctrl_home();
    	plc_com_error(e_position);
    } else {
        set_state(success);
    }
}

    /*
	 * @brief: Move down to place position, switch of magnet and move up to travel position, then send the success message to the PLC.
     * If out of position error occurred, init homing and report error.
	 */
void z_axis_place_sample(){

    uint8_t status = 0;

    #if LOGS >= 1
    rprintf("LOG: Z_PLACE_SAMPLE\r\n");
    #endif
	
	if(sTool == stamp_tool) {
		plc_com_error(e_wrong_tool);
		return;
	}
	
	status = drv_ctrl_moveto(PLACE_HEIGHT_STEPS);
	port_pin_set_output_level(MAGNET_SWITCH_PIN, false);
	delay_ms(WAIT_TIME_ms);
	status = drv_ctrl_moveto(TRAVEL_HEIGHT_STEPS);

    if(status == 1){
        drv_ctrl_home();
        plc_com_error(e_position);
    } else {
        set_state(success);
    }
}

    /* 
	 * @brief: Move down to the ink pad till the required force is reached. Then move to the travel position.
     * If out of position error occurred, init homing and report error.
	 */
void z_axis_soak_stamp() {

    uint8_t forceStatus = 0;
    uint8_t posStatus = 0;

    #if LOGS >= 1
    rprintf("LOG: Z_SOAK_STAMP\r\n");
    #endif
	
	if(sTool == pick_tool) {
		plc_com_error(e_wrong_tool);
		return;
	}
	
	posStatus = drv_ctrl_moveto(SOAK_HEIGHT_STEPS);
	forceStatus = drv_ctrl_move_till_force(SOAK_FORCE_mN);
	delay_ms(WAIT_TIME_ms);
	posStatus = drv_ctrl_moveto(TRAVEL_HEIGHT_STEPS);

    if(forceStatus == 0 && posStatus == 0){
	    set_state(success);
    } else if (posStatus == 1){
        drv_ctrl_home();
        plc_com_error(e_position);
    } else {
        plc_com_error(e_force);
    }

}

    /* 
	 * @brief: Move down to the box till the required force is reached. Then move to the travel position.
     * If out of position error occurred, init homing and report error.
	 */
void z_axis_stamp() {

    uint8_t forceStatus = 0;
    uint8_t posStatus = 0;

    #if LOGS >= 1
    rprintf("LOG: Z_STAMP\r\n");
    #endif
	
	if(sTool == pick_tool) {
		plc_com_error(e_wrong_tool);
		return;
	}
	
	posStatus = drv_ctrl_moveto(STAMP_HEIGHT_STEPS);
	forceStatus = drv_ctrl_move_till_force(STAMP_FORCE_mN);
	delay_ms(WAIT_TIME_ms);
	posStatus = drv_ctrl_moveto(TRAVEL_HEIGHT_STEPS);
	
    if(forceStatus == 0 && posStatus == 0){
        set_state(success);
    } else if (posStatus == 1){
        drv_ctrl_home();
        plc_com_error(e_position);
    } else {
        plc_com_error(e_force);
    }
}

    /* 
	 * @brief: Here the sequence for closing the lid is implemented. This sequence has to be defined, based on the mechanical aspects of the machine.
     * If out of position error occurred, init homing and report error.
	 */
void z_axis_close_lid() {

    uint8_t status = 0;

    #if LOGS >= 1
    rprintf("LOG: Z_CLOSE_LID\r\n");
    #endif
	
	if(sTool == stamp_tool) {
		plc_com_error(e_wrong_tool);
		return;
	}
	
	status = drv_ctrl_moveto(CLOSE_HEIGHT_STEPS);
	delay_ms(WAIT_TIME_ms);
	status = drv_ctrl_moveto(TRAVEL_HEIGHT_STEPS);

    if(status == 1){
        drv_ctrl_home();
        plc_com_error(e_position);
        } else {
        set_state(success);
    }
}

    /* 
	 * @brief: Sequence for tool changing. This sequence calls static function drop_tool and grab_tool based on the actual tool.
     * If out of position error occurred, init homing and report error.
	 */
void z_axis_change_tool() {

    uint8_t status = 0;

    #if LOGS >= 1
    rprintf("LOG: Z_CHANGE_TOOL\r\n");
    #endif
	
	if(sTool == pick_tool){
		status = z_axis_grab_tool();
		sTool = stamp_tool;
	} else {
		status = z_axis_drop_tool();
		sTool = pick_tool;
	}

    if(status == 1){
        drv_ctrl_home();
        plc_com_error(e_position);
        } else {
        set_state(success);
    }
}

    /*
	 * @brief: Move down to the tool, switch on magnet, move to travel position and calibrate the force sensor.
     * If out of position error occurred, set state to 1.
	 */
static uint8_t z_axis_grab_tool(void) {

    uint8_t status = 0;

    #if LOGS >= 1
    rprintf("LOG: Z_GRAB_TOOL\r\n");
    #endif
	
	port_pin_set_output_level(MAGNET_SWITCH_PIN, true);
	status = drv_ctrl_moveto(TOOL_GRAB_HEIGHT_STEPS);
	delay_ms(WAIT_TIME_ms);
	status = drv_ctrl_moveto(TRAVEL_HEIGHT_STEPS);
	force_sense_calibrate();

    return status;
}

    /*
	 * @brief: Move down to the tool holder, switch off magnet, move to travel position and calibrate the force sensor.
     * If out of position error occurred, set state to 1.
	 */
static uint8_t z_axis_drop_tool(void) {
    
    uint8_t status = 0;

    #if LOGS >= 1
    rprintf("LOG: Z_DROP_TOOL\r\n");
    #endif
	
	status = drv_ctrl_moveto(TOOL_DROP_HEIGHT_STEPS);
	port_pin_set_output_level(MAGNET_SWITCH_PIN, false);
	delay_ms(WAIT_TIME_ms);
	status = drv_ctrl_moveto(TRAVEL_HEIGHT_STEPS);
	force_sense_calibrate();

    return status;
}