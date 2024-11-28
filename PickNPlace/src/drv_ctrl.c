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

static uint16_t sActualPositionSteps;
static volatile uint16_t sStepcounter;
static volatile uint16_t sPulsePeriod;
static volatile uint16_t sTargetPeriod;

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
static void drv_ctrl_set_ramp_params(uint16_t, uint16_t);
static inline void drv_ctrl_set_period(void);

    /* 
	 * @brief: Pure SPI write function.
     * @param: register adress size = one byte
     * @param: data size = two byte
	 */
static void drv_ctrl_write_cmd(uint8_t adress, uint16_t data) {

    #if LOGS == 2
    rprintf("LOG: transmit: %d to adress: %d\r\n", data, adress);
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
    drv_ctrl_set_microsteps(new_config->step_mode);
}

    /* 
     * @brief: Enable the motor driver. This has to be done prior to all move fuctions!
     */
void drv_ctrl_enable(){

    #if LOGS == 2
    rprintf("LOG: enable driver\r\n");
    #endif
	
	sDrvConfig.enable = DRV_ENABLE;
	drv_ctrl_write_ctrl();
}

    /*
     * @brief: Disable the motor driver.
     */
void drv_ctrl_disable(){

    #if LOGS == 2
    rprintf("LOG: disable driver\r\n");
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
        rprintf("LOG: set micro steps to: %d\r\n", sStepDivider);
        #endif
	}
}

    /*
     * @brief: Set the torque of the motor in percent of the maximum torque.
     * @param: Torque in percent
     */
void drv_ctrl_set_torque(uint8_t torquePercent){

    sDrvConfig.drv_torque = (150 * torquePercent) / 100;
    drv_ctrl_write_torque();

}

    /*
     * @brief:  Turn on motor in upward direction until the top switch is reached.
     *          Then stop and set Position to 0. This function is blocking!!
     */
void drv_ctrl_home() {

    #if LOGS == 2
    rprintf("LOG: home sequence called\r\n");
    #endif
	
    //Switch to more micro steps = slower movement
	//drv_ctrl_set_microsteps(DRV_MODE_1_64);

    drv_ctrl_set_torque(20);
	drv_ctrl_set_ramp_params(STEPPER_PULSE_SLOW_PERIOD_ms, STEPPER_PULSE_SLOW_PERIOD_ms);
	port_pin_set_output_level(MOTOR_CONTROLLER_DIR_PIN, up);
    tc_start_counter(&pwm_timer);
	while (port_pin_get_input_level(Z_AXIS_ZERO_SWITCH_PIN))
	{
		//wait till home pos is reached
	}
	tc_stop_counter(&pwm_timer);
	sActualPositionSteps = 0;

    drv_ctrl_set_torque(10);
		
}

    /* 
     * @brief:  Move to position, check for out of range error.
     *          When movement is complete set new position. This function is blocking!!
     * @param:  Target positon in mm
	 */
void drv_ctrl_moveto(uint16_t position_mm) {

    #if LOGS == 2
    rprintf("LOG: move to %d mm\r\n", position_mm);
    #endif
	
	if(position_mm > Z_AXIS_MAX_TRAVEL) {
		    #if LOGS == 2
		    rprintf("Exceed max travel!\r\n", position_mm);
		    #endif
		return;
	}
	
    //Switch to less micro steps = faster movement
	//drv_ctrl_set_microsteps(DRV_MODE_1_8);

    drv_ctrl_set_torque(90);
	
	uint32_t target_steps = position_mm * Z_AXIS_STEPS_PER_MM ;
	
	enum direction dir = (sActualPositionSteps > target_steps) ? up : down;
	sStepcounter = abs(sActualPositionSteps - target_steps)*sStepDivider;
	    #if LOGS == 2
	    rprintf("LOG: travel %d steps in dir: %d\r\n", sStepcounter, dir);
	    #endif
	
	port_pin_set_output_level(MOTOR_CONTROLLER_DIR_PIN, dir);
    drv_ctrl_set_ramp_params(STEPPER_PULSE_SLOW_PERIOD_ms, STEPPER_PULSE_PERIOD_ms);
	tc_start_counter(&pwm_timer);

    while(sStepcounter > 0){
	    #if LOGS == 2
	    rprintf("LOG: travel %d steps.\r\n", sStepcounter);
	    #endif        
        //Wait till drive finished movement
    }
	
	sActualPositionSteps = target_steps;
    
    drv_ctrl_set_torque(10);
}


    /*
	 * @brief:  Move till defined force is reached. When defined force is reached the drive will stop and retract.
     * @param:  Target force in mN
	 */
uint8_t drv_ctrl_move_till_force(uint16_t force_mN) {

    #if LOGS == 2
    rprintf("LOG: move till %d mN\r\n", force_mN);
    #endif

    uint8_t status = 0;
	
    //Switch to more micro steps = slower movement
	//drv_ctrl_set_microsteps(DRV_MODE_1_64);
	
	drv_ctrl_set_torque(20);
	
	port_pin_set_output_level(MOTOR_CONTROLLER_DIR_PIN, down);
	
    sStepcounter = Z_AXIS_MAX_STAMP_DISTANCE * Z_AXIS_STEPS_PER_MM;
    drv_ctrl_set_ramp_params(STEPPER_PULSE_SLOW_PERIOD_ms, STEPPER_PULSE_SLOW_PERIOD_ms);
    tc_start_counter(&pwm_timer);
	while(force_sense_get_millinewton() < force_mN && sStepcounter > 0) {
        // Wait till desired force is reached or distance exceeds max stamp distance
        delay_ms(1);
	}
    tc_stop_counter(&pwm_timer);
    if(sStepcounter == 0) status = 1;
	
    //Retract same amount of steps which were counted during downward movement
	port_pin_set_output_level(MOTOR_CONTROLLER_DIR_PIN, up);
    sStepcounter = (Z_AXIS_MAX_STAMP_DISTANCE * Z_AXIS_STEPS_PER_MM) - sStepcounter;
    tc_start_counter(&pwm_timer);
	while(sStepcounter > 0) {
		// Wait till retracted
	}
	
	drv_ctrl_set_torque(10);

    return status;
}

void drv_ctrl_pwm_callback(struct tc_module *const module_inst){

    if(get_state() == init) return;
    
    if(sStepcounter == 0){
        tc_stop_counter(module_inst);
        return;
    };

    sStepcounter--;

    if(sPulsePeriod > sTargetPeriod){
        sPulsePeriod -= 5;
        drv_ctrl_set_period();
    }
}

static void drv_ctrl_set_ramp_params(uint16_t startPeriod, uint16_t targetPeriod){
    sPulsePeriod = startPeriod;
    sTargetPeriod = targetPeriod;
    drv_ctrl_set_period();
}

static inline void drv_ctrl_set_period(){
    
    tc_set_compare_value(&pwm_timer, TC_COMPARE_CAPTURE_CHANNEL_0, PERIOD_TO_CCVAL(sPulsePeriod));
    tc_set_compare_value(&pwm_timer, TC_COMPARE_CAPTURE_CHANNEL_1, PERIOD_TO_CCVAL(sPulsePeriod)/PWM_START_DUTY);
}
