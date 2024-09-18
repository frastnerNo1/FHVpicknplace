/*
 * force_sense.c
 *
 * Created: 18.09.2024 09:33:52
 *  Author: floro
 */ 

/* In this file the forcesensor readout is implemented. */

/* Todo: implement the following functions:
- read force via ADC and to conversion
- return the actual forcevalue in newton
- return the actual forcebalue in gramm
*/

int force_sense_read_sense() {
	/* Read the voltage from the INA via ADC, save the raw value in memory. Returns 1 on success and 0 on error*/
}

int force_sense_calibrate() {
	/* Read the force sensor and calibrates it to the input value. Returns 1 on success and 0 on failure.*/
}

int force_sense_get_newton() {
	/* Trigger a force sensor reading and converts the value to newton and returns it. */
}

int force_sense_get_gramm() {
	/* Trigger a force sensor reading and converts the value to gram then returns it. */
}