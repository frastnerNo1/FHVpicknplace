/*
 * z_axis.c
 *
 * Created: 18.09.2024 09:32:43
 *  Author: floro
 */ 

/* Here do all commands regarding the z-axis. This includes movement, triggering actuators like the magnet and checking the force value.
Therefore other functions from drv_ctrl and force_sense are used.*/


/*Todo: implement functions for the following:
- homing
- moving to specific points
- checking forge while moving
- turn the magnet on and of
*/

int z_axis_home() {
	/*Initialize homing sequence then sets axis state to home and ready.*/
}

int z_axis_pick_sample(){
	/* Move down to pick position, switch on magnet and move up to travel position, then give the success message to the PLC. */
}

int z_axis_place_sample(){
	/* Move down to place position, switch of magnet and move up to travel position, then give the success message to the PLC. */
}

int z_axis_soak_stamp() {
	/* Move down to the pad till the required force is reached. Then move to the travel position. Returns 1 on success and 0 on failure.*/
}

int z_axis_stamp() {
	/* Move down to the box till the required force is reached. Then move to the travel position. Returns 1 on success and 0 on failure.*/
}

int z_axis_grab_tool() {
	/* Move down to the tool, switch on magnet, move to travel position and calibrate the force sensor. Returns 1 on success and 0 on failure.*/
}

int z_axis_drop_tool() {
	/* Move down to the toolholder, switch of magnet, move to travel position and calibrate the force sensor. Returns 1 on success and 0 on failure. */
}

int z_axis_close_lid() {
	/* Here the sequence for closing the lid is implementer. This sequence has to be defined, based on the mechanical aspects of the machine.*/
}
