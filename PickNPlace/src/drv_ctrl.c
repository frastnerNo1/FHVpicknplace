/*
 * drv_ctrl.c
 *
 * Created: 18.09.2024 09:33:24
 *  Author: floro
 */ 

/* Here the interconnect between the MCU and the motor driver is implemented.
 * 
 */

#include "drv_ctrl.h"
#include "force_sense.h"

enum direction {up = DIRECTION_UP, down = DIRECTION_DOWN};

static Driver_Instance_t sDrvConfig;

static uint32_t sActualPositionSteps;

static uint16_t sStepDivider;

static void drv_ctrl_write_cmd(uint8_t, uint16_t);
static uint16_t drv_ctrl_read_cmd(uint8_t);
static void drv_ctrl_write_ctrl(void);
static void drv_ctrl_write_torque(void);
static void drv_ctrl_write_off(void);
static void drv_ctrl_write_blank(void);
static void drv_ctrl_write_decay(void);
static void drv_ctrl_write_stall(void);
static void drv_ctrl_write_drive(void);

    /* 
	 * @brief: Pure SPI write function.
     * @param: register adress size = one byte
     * @param: data size = two byte
	 */
static void drv_ctrl_write_cmd(uint8_t adress, uint16_t data) {

    #if LOGS == 2
    rprintf("LOG: transmit: %x to adress: %x", data, adress);
    #endif
	
	uint8_t transfer_data_buffer[] = {((adress << 4)|(data >> 8)), (data & 0xFF)};
	
	port_pin_set_output_level(MOTOR_CONTROLLER_SS_PIN, true);
	spi_write_buffer_wait(&gSpiMasterInstance, transfer_data_buffer, 2);
	port_pin_set_output_level(MOTOR_CONTROLLER_SS_PIN, false);
}


    /* 
     * @brief: Pure SPI read function.
     * @param: adress to read size = one byte
     * @returns: two byte of data, the 4 MSB are not relevant
	 */	
static uint16_t drv_ctrl_read_cmd(uint8_t adress) {
	
	uint16_t dummy = ((adress << 4) | (1 << 7));
	uint8_t data[2] ={0x00, 0x00};
	
	port_pin_set_output_level(MOTOR_CONTROLLER_SS_PIN, true);
	enum status_code response = spi_read_buffer_wait(&gSpiMasterInstance, data, 2, dummy);
	port_pin_set_output_level(MOTOR_CONTROLLER_SS_PIN, false);
	
	if(response == 0) {
		return ((data[0] << 4) | data[1]);
	} else {
		return EXIT_FAILURE;
	}
}

    /*
	 * Following private functions writes to one designated register. The functions are using the local configuration struct
	 * to prevent overwriting of previous values.
	 */

static void drv_ctrl_write_ctrl(){
	
	drv_ctrl_write_cmd(CTRL_REG,
	sDrvConfig.direction_set |
	sDrvConfig.enable |
	sDrvConfig.step_mode |
	sDrvConfig.stall_detect |
	sDrvConfig.isense_gain |
	sDrvConfig.dead_time_insert);
	
}

static void drv_ctrl_write_torque(){
	
	drv_ctrl_write_cmd(TORQUE_REG,
	sDrvConfig.drv_torque |
	sDrvConfig.backemf_sample_th);
	
}

static void drv_ctrl_write_off(){
	
    drv_ctrl_write_cmd(OFF_REG,
    sDrvConfig.drv_toff |
    sDrvConfig.pwm_mode);
	
}

static void drv_ctrl_write_blank(){
	
	drv_ctrl_write_cmd(BLANK_REG,
	sDrvConfig.drv_tblank |
	sDrvConfig.adaptive_blanking_time);
	
}

static void drv_ctrl_write_decay(){
	
	drv_ctrl_write_cmd(DECAY_REG,
	sDrvConfig.drv_tdecay |
	sDrvConfig.decay_mode);
	
}

static void drv_ctrl_write_stall(){
	
	drv_ctrl_write_cmd(STALL_REG,
	sDrvConfig.drv_sdthr |
	sDrvConfig.stall_count |
	sDrvConfig.back_emf_div);
	
}

static void drv_ctrl_write_drive(){
	
	drv_ctrl_write_cmd(DRIVE_REG,
	sDrvConfig.ocp_threshold |
	sDrvConfig.ocp_threshold |
	sDrvConfig.ls_drive_time |
	sDrvConfig.hs_drive_time |
	sDrvConfig.ls_current |
	sDrvConfig.hs_current);
	
}

    /*
	 * @brief: Initialize the stepper driver
     * @param: new config struct as Driver_Instance_t
	 */
void drv_ctrl_init(Driver_Instance_t * const new_config) {
	
	sDrvConfig = *new_config;
	
	drv_ctrl_write_ctrl();			
	drv_ctrl_write_torque();			
    drv_ctrl_write_off();			
	drv_ctrl_write_blank();			
	drv_ctrl_write_decay();			
	drv_ctrl_write_stall();			
	drv_ctrl_write_drive();	
}

    /* 
     * @brief: Enable the motor driver. This has to be done prior to all move fuctions!
     */
void drv_ctrl_enable(){

    #if LOGS == 2
    rprintf("LOG: enable driver");
    #endif
	
	sDrvConfig.enable = DRV_ENABLE;
	drv_ctrl_write_ctrl();
}

    /*
     * @brief: Disable the motor driver.
     */
void drv_ctrl_disable(){

    #if LOGS == 2
    rprintf("LOG: disable driver");
    #endif
	
	sDrvConfig.enable = DRV_DISABLE;
	drv_ctrl_write_ctrl();
}

    /*
     * @brief: Set the microstep mode. Changes also the step divider for converting mm to steps.
     * @param: micro step mode as drv_mode
     */
void drv_ctrl_set_microsteps(enum drv_mode steps) {
	
	sDrvConfig.step_mode = steps;
	drv_ctrl_write_ctrl();
	
	switch(steps) {
		case(DRV_MODE_1):
		    sStepDivider = 1;
			break;
		case(DRV_MODE_1_2):
		    sStepDivider = 2;
		    break;
		case(DRV_MODE_1_4):
		    sStepDivider = 4;
			break;
		case(DRV_MODE_1_8):
		    sStepDivider = 8;
		    break;
		case(DRV_MODE_1_16):
		    sStepDivider = 16;
		    break;
		case(DRV_MODE_1_32):
		    sStepDivider = 32;
		    break;
		case(DRV_MODE_1_64):
		    sStepDivider = 64;
		    break;
		case(DRV_MODE_1_128):
		    sStepDivider = 128;
			break;
	    case(DRV_MODE_1_256):
		    sStepDivider = 256;
			break;

        #if LOGS == 2
        rprintf("LOG: set micro steps to: %d", sStepDivider);
        #endif
	}
}

    /*
     * @brief:  Turn on motor in upward direction until the top switch is reached.
     *          Then stop and set Position to 0. This function is blocking!!
     */
void drv_ctrl_home() {

    #if LOGS == 2
    rprintf("LOG: home sequence called");
    #endif
	
    //Switch to more micro steps = slower movement
	drv_ctrl_set_microsteps(DRV_MODE_1_64);
	
	port_pin_set_output_level(MOTOR_CONTROLLER_DIR_PIN, up);
	while (port_pin_get_input_level(Z_AXIS_ZERO_SWITCH_PIN))
	{
		port_pin_set_output_level(MOTOR_CONTROLLER_STP_PIN, true);
		delay_us(STEPPER_PULSE_PERIOD_us);
		port_pin_set_output_level(MOTOR_CONTROLLER_STP_PIN, false);
		delay_us(STEPPER_PULSE_PERIOD_us);
	}
	
	sActualPositionSteps = 0;
		
}

    /* 
     * @brief:  Move to position, check for out of range error.
     *          When movement is complete set new position. This function is blocking!!
     * @param:  Target positon in mm
	 */
void drv_ctrl_moveto(uint16_t position_mm) {

    #if LOGS == 2
    rprintf("LOG: move to %d mm", position_mm);
    #endif
	
	if(position_mm > Z_AXIS_MAX_TRAVEL) {
		return;
	}
	
    //Switch to less micro steps = faster movement
	drv_ctrl_set_microsteps(DRV_MODE_1_4);
	
	uint32_t target_steps = position_mm * Z_AXIS_STEPS_PER_MM ;
	
	enum direction dir = (sActualPositionSteps > target_steps) ? up : down;
	uint32_t steps = abs(sActualPositionSteps - target_steps)*sStepDivider;
	
	port_pin_set_output_level(MOTOR_CONTROLLER_DIR_PIN, dir);
	for(uint32_t i = 0; i <= steps; i++){
		port_pin_set_output_level(MOTOR_CONTROLLER_STP_PIN, true);
		delay_us(STEPPER_PULSE_PERIOD_us);
		port_pin_set_output_level(MOTOR_CONTROLLER_STP_PIN, false);
		delay_us(STEPPER_PULSE_PERIOD_us);
	}
	
	sActualPositionSteps = target_steps;	
}


    /*
	 * @brief:  Move till defined force is reached. When defined force is reached the drive will stop and retract.
     * @param:  Target force in mN
	 */
void drv_ctrl_move_till_force(uint16_t force_mN) {

    #if LOGS == 2
    rprintf("LOG: move till %d mN", force_mN);
    #endif
	
    //Step counter for retraction
	uint16_t step_counter = 0;
	
    //Switch to more micro steps = slower movement
	drv_ctrl_set_microsteps(DRV_MODE_1_64);
	
	port_pin_set_output_level(MOTOR_CONTROLLER_DIR_PIN, down);
	
	while(force_sense_get_millinewton() < force_mN) {
		port_pin_set_output_level(MOTOR_CONTROLLER_STP_PIN, true);
		delay_us(STEPPER_PULSE_PERIOD_us);
		port_pin_set_output_level(MOTOR_CONTROLLER_STP_PIN, false);
		delay_us(STEPPER_PULSE_PERIOD_us);
		step_counter ++; //Count steps in downward direction
	}
	
	port_pin_set_output_level(MOTOR_CONTROLLER_DIR_PIN, up);
	
    //Retract same amount of steps which were counted during downward movement
	for( ;step_counter > 0; step_counter-- ) {
		port_pin_set_output_level(MOTOR_CONTROLLER_STP_PIN, true);
		delay_us(STEPPER_PULSE_PERIOD_us);
		port_pin_set_output_level(MOTOR_CONTROLLER_STP_PIN, false);
		delay_us(STEPPER_PULSE_PERIOD_us);
	}
}
