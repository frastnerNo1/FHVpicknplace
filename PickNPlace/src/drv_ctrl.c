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

int drv_ctrl_write_cmd() {
	/* Pure SPI write function, takes register and date as input and send it to the stepper controller via SPI.
	 * Returns 1 on success and 0 on failure. */
}

int drv_ctrl_read_cmd() {
	/* Pure SPI read function, takes register and number of bytes to read as input and returns the date on success or
	 * 0 on failure. */
}

int drv_ctrl_init() {
	/* Initialize the stepper driver, drv_config_struct has to be defined in this file and exported in header. */
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
