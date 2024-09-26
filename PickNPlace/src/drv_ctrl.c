/*
 * drv_ctrl.c
 *
 * Created: 18.09.2024 09:33:24
 *  Author: floro
 */ 

/* This file manages the drive related stuff. Private functions handling bare SPÌ com functionality. Public functions handles move commands.*/

/* Todo: implement the following functions:
- bare SPI register write
- bare SPI register read
- move to home
- move to specific position
- return actual position
*/

#include "drv_ctrl.h"
#include "force_sense.h"

enum direction {up = DIRECTION_UP, down = DIRECTION_DOWN};

static struct drv_config_struct sDrvConfig;

static uint32_t sActualPositionSteps;

    /* Pure SPI write function, takes register and data as input and send it to the stepper controller via SPI.
	 * Returns 0 on success and 1 on failure.
	 */
static void drv_ctrl_write_cmd(uint8_t adress, uint16_t data) {
	
	uint8_t transfer_data_buffer[] = {((adress << 4)|(data >> 8)), (data & 0xFF)};
	
	port_pin_set_output_level(MOTOR_CONTROLLER_SS_PIN, true);
	spi_write_buffer_wait(&gSpiMasterInstance, transfer_data_buffer, 2);
	port_pin_set_output_level(MOTOR_CONTROLLER_SS_PIN, false);
}


    /* Pure SPI read function, takes register adress as input and reads 16 bit of data then returns 
	 * the date on success or 1 on failure. The first 4 bits are not relevant.
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

    /* Following functions writes to one designated register. The functions are using the local configuration struct
	 * to prevent overwriting of previous values.
	 */

static void drv_ctrl_write_ctrl(void){
	
	drv_ctrl_write_cmd(CTRL_REG,
	sDrvConfig.direction_set |
	sDrvConfig.enable |
	sDrvConfig.step_mode |
	sDrvConfig.stall_detect |
	sDrvConfig.isense_gain |
	sDrvConfig.dead_time_insert);
	
}

static void drv_ctrl_write_torque(void){
	
	drv_ctrl_write_cmd(TORQUE_REG,
	sDrvConfig.drv_torque |
	sDrvConfig.backemf_sample_th);
	
}

static void drv_ctrl_write_off(void){
	
    drv_ctrl_write_cmd(OFF_REG,
    sDrvConfig.drv_toff |
    sDrvConfig.pwm_mode);
	
}

static void drv_ctrl_write_blank(void){
	
	drv_ctrl_write_cmd(BLANK_REG,
	sDrvConfig.drv_tblank |
	sDrvConfig.adaptive_blanking_time);
	
}

static void drv_ctrl_write_decay(void){
	
	drv_ctrl_write_cmd(DECAY_REG,
	sDrvConfig.drv_tdecay |
	sDrvConfig.decay_mode);
	
}

static void drv_ctrl_write_stall(void){
	
	drv_ctrl_write_cmd(STALL_REG,
	sDrvConfig.drv_sdthr |
	sDrvConfig.stall_count |
	sDrvConfig.back_emf_div);
	
}

static void drv_ctrl_write_drive(void){
	
	drv_ctrl_write_cmd(DRIVE_REG,
	sDrvConfig.ocp_threshold |
	sDrvConfig.ocp_threshold |
	sDrvConfig.ls_drive_time |
	sDrvConfig.hs_drive_time |
	sDrvConfig.ls_current |
	sDrvConfig.hs_current);
	
}

    /* Initialize the stepper driver, drv_config_struct is defined in this file and exported in header. */
void drv_ctrl_init(struct drv_config_struct * const new_config) {
	
	sDrvConfig = *new_config;
	
	drv_ctrl_write_ctrl();			
	drv_ctrl_write_torque();			
    drv_ctrl_write_off();			
	drv_ctrl_write_blank();			
	drv_ctrl_write_decay();			
	drv_ctrl_write_stall();			
	drv_ctrl_write_drive();	
}

void drv_ctrl_enable(){
	
	sDrvConfig.enable = DRV_ENABLE;
	drv_ctrl_write_ctrl();
}

void drv_ctrl_disable(){
	
	sDrvConfig.enable = DRV_DISABLE;
	drv_ctrl_write_ctrl();
}

void drv_ctrl_set_microsteps(uint8_t steps) {
	
	sDrvConfig.step_mode = steps;
	drv_ctrl_write_ctrl();
}

/*
 *Moves drive up until the top switch is reached.
 *Then stops and set Position to 0. Returns 1 on success and 0 on failure.
 */
void drv_ctrl_home() {
	
	port_pin_set_output_level(MOTOR_CONTROLLER_DIR_PIN, up);
	while (port_pin_get_input_level(Z_AXIS_ZERO_SWITCH_PIN))
	{
		port_pin_set_output_level(MOTOR_CONTROLLER_STP_PIN, true);
		delay_us(STEPPER_PULSE_SLOW_PERIOD_us);
		port_pin_set_output_level(MOTOR_CONTROLLER_STP_PIN, false);
		delay_us(STEPPER_PULSE_SLOW_PERIOD_us);
	}
	
	sActualPositionSteps = 0;
		
}

    /* Move to position, takes target position as input in mm from top. Check also for out of range position.
	 * when move is complete set new position. Returns 1 on success and 0 on failure. This function is blocking!!
	 */
void drv_ctrl_moveto(uint16_t position_mm) {
	
	if(position_mm > Z_AXIS_MAX_TRAVEL) {
		return;
	}
	
	uint32_t target_steps = position_mm * Z_AXIS_STEPS_PER_MM;
	
	enum direction dir = (sActualPositionSteps > target_steps) ? up : down;
	uint32_t steps = abs(sActualPositionSteps - target_steps);
	
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
	 * Move till defined force is reached. Check for maximum force and maximum moving range.
	 * when defined force is reached the drive will stop.
	 */
void drv_ctrl_move_till_force(uint16_t force_mN) {
	
	uint16_t step_counter = 0;
	
	port_pin_set_output_level(MOTOR_CONTROLLER_DIR_PIN, down);
	
	while(force_sense_get_millinewton() < force_mN) {
		port_pin_set_output_level(MOTOR_CONTROLLER_STP_PIN, true);
		delay_us(STEPPER_PULSE_PERIOD_us);
		port_pin_set_output_level(MOTOR_CONTROLLER_STP_PIN, false);
		delay_us(STEPPER_PULSE_PERIOD_us);
		step_counter++;
	}
	
	port_pin_set_output_level(MOTOR_CONTROLLER_DIR_PIN, up);
	
	for( ;step_counter > 0; step_counter--) {
		port_pin_set_output_level(MOTOR_CONTROLLER_STP_PIN, true);
		delay_us(STEPPER_PULSE_PERIOD_us);
		port_pin_set_output_level(MOTOR_CONTROLLER_STP_PIN, false);
		delay_us(STEPPER_PULSE_PERIOD_us);
	}
}
