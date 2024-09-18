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

    /* Pure SPI write function, takes register and data as input and send it to the stepper controller via SPI.
	 * Returns 0 on success and 1 on failure.
	 */
int drv_ctrl_write_cmd(uint8_t adress, uint16_t data) {
	
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
	
uint16_t drv_ctrl_read_cmd(uint8_t adress) {
	
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

int drv_ctrl_init() {
	/* Initialize the stepper driver, drv_config_struct has to be defined in this file and exported in header. */
}

int drv_ctrl_enable(){
	/* Enable the stepper driver. */
}

int drv_ctrl_home() {
	/* Moves drive up until the top switch is reached. Then stops and set Position to 0. Returns 1 on success and 0 on failure.*/	
}

int drv_ctrl_moveto() {
	/* Move to position, takes target position as input in mm from top. Check also for out of range position.
	 * when move is complete set new position. Returns 1 on success and 0 on failure.*/
}

int drv_ctrl_move_till_force() {
	/* Move till defined force is reached. Check for maximum force and maximum moving range.
	 * when defined force is reached the drive will stop. Returns 1 on success and 0 an failure*/
}

int drv_ctrl_getpos() {
	/*Returns the actual position of the Z-Axis in mm from top.*/
}
