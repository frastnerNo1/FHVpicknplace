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

#include <asf.h>
#include <stdlib.h>
#include "main.h"
#include "drv_ctrl.h"

static struct drv_config_struct drv_config;
static enum direction {up = DIRECTION_UP, down = DIRECTION_DOWN};
static uint16_t actual_position_mm;

    /* Pure SPI write function, takes register and data as input and send it to the stepper controller via SPI.
	 * Returns 0 on success and 1 on failure.
	 */
static int drv_ctrl_write_cmd(uint8_t adress, uint16_t data) {
	
	uint8_t transfer_data_buffer[] = {((adress << 4)|(data >> 8)), (data & 0xFF)};
		
	spi_select_slave(&spi_master_instance, &spi_motor_controller, true);
	enum status_code response = spi_write_buffer_wait(&spi_master_instance, transfer_data_buffer, 2);
	spi_select_slave(&spi_master_instance, &spi_motor_controller, false);
	
	if(response == 0) {
		return EXIT_SUCCESS;
	} else {
		return EXIT_FAILURE;
	}
}


    /* Pure SPI read function, takes register adress as input and reads 16 bit of data then returns 
	 * the date on success or 1 on failure. The first 4 bits are not relevant.
	 */	
static uint16_t drv_ctrl_read_cmd(uint8_t adress) {
	
	uint16_t dummy = ((adress << 4) | (1 << 7));
	uint8_t data[2];
	
	spi_select_slave(&spi_master_instance, &spi_motor_controller, true);
	enum status_code response = spi_read_buffer_wait(&spi_master_instance, data, 2, dummy);
	spi_select_slave(&spi_master_instance, &spi_motor_controller, false);
	
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
	drv_config.direction_set |
	drv_config.enable |
	drv_config.step_mode |
	drv_config.stall_detect |
	drv_config.isense_gain |
	drv_config.dead_time_insert);
	
}

static void drv_ctrl_write_torque(void){
	
	drv_ctrl_write_cmd(TORQUE_REG,
	drv_config.drv_torque |
	drv_config.backemf_sample_th);
	
}

static void drv_ctrl_write_off(void){
	
    drv_ctrl_write_cmd(OFF_REG,
    drv_config.drv_toff |
    drv_config.pwm_mode);
	
}

static void drv_ctrl_write_blank(void){
	
	drv_ctrl_write_cmd(BLANK_REG,
	drv_config.drv_tblank |
	drv_config.adaptive_blanking_time);
	
}

static void drv_ctrl_write_decay(void){
	
	drv_ctrl_write_cmd(DECAY_REG,
	drv_config.drv_tdecay |
	drv_config.decay_mode);
	
}

static void drv_ctrl_write_stall(void){
	
	drv_ctrl_write_cmd(STALL_REG,
	drv_config.drv_sdthr |
	drv_config.stall_count |
	drv_config.back_emf_div);
	
}

static void drv_ctrl_write_drive(void){
	
	drv_ctrl_write_cmd(DRIVE_REG,
	drv_config.ocp_threshold |
	drv_config.ocp_threshold |
	drv_config.ls_drive_time |
	drv_config.hs_drive_time |
	drv_config.ls_current |
	drv_config.hs_current);
	
}

    /* Initialize the stepper driver, drv_config_struct is defined in this file and exported in header. */
int drv_ctrl_init(struct drv_config_struct * const config) {
	
	drv_config = *config;
	
	drv_ctrl_write_ctrl();
			
	drv_ctrl_write_torque();
			
    drv_ctrl_write_off();
			
	drv_ctrl_write_blank();
			
	drv_ctrl_write_decay();
			
	drv_ctrl_write_stall();
			
	drv_ctrl_write_drive();
	
			
	return EXIT_SUCCESS;
}

int drv_ctrl_enable(){
	
	drv_config.enable = DRV_ENABLE;
	drv_ctrl_write_ctrl();
	
	return EXIT_SUCCESS;
}

int drv_ctrl_disable(){
	
	drv_config.enable = DRV_DISABLE;
	drv_ctrl_write_ctrl();
	
	return EXIT_SUCCESS;
}

int drv_ctrl_home() {
	/* Moves drive up until the top switch is reached. Then stops and set Position to 0. Returns 1 on success and 0 on failure.*/	
}

    /* Move to position, takes target position as input in mm from top. Check also for out of range position.
	 * when move is complete set new position. Returns 1 on success and 0 on failure.
	 */
int drv_ctrl_moveto(uint16_t position_mm) {
	
	if(position_mm > Z_AXIS_MAX_TRAVEL) {
		return EXIT_FAILURE;
	}
	
	enum direction dir = (actual_position_mm > position_mm) ? up : down;
	uint16_t steps = abs(actual_position_mm - position_mm) * Z_AXIS_STEPS_PER_MM;
	
	port_pin_set_output_level(MOTOR_CONTROLLER_DIR_PIN, dir);
	for(int i = 0; i <= steps; i++){
		port_pin_set_output_level(MOTOR_CONTROLLER_STP_PIN, true);
		delay_ms(10);
		port_pin_set_output_level(MOTOR_CONTROLLER_STP_PIN, false);
		delay_ms(20);
	}
	
	actual_position_mm = position_mm;
	
	return EXIT_SUCCESS;
	
}

int drv_ctrl_move_till_force(uint8_t force_mN) {
	/* Move till defined force is reached. Check for maximum force and maximum moving range.
	 * when defined force is reached the drive will stop. Returns 1 on success and 0 an failure*/
}

uint16_t drv_ctrl_getpos() {
	/*Returns the actual position of the Z-Axis in mm from top.*/
}
