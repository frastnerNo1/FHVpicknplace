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

#include "force_sense.h"

static uint16_t force_sense_last_readout;
static uint16_t force_sense_zero_value;

    /* Reads the voltage from the INA via ADC, save the raw value in memory. Returns 1 on success and 0 on error*/
static int force_sense_read_sense() {
	
	adc_start_conversion(&adc_instance);
	
	while(adc_read(&adc_instance, &force_sense_last_readout) == STATUS_BUSY){
		//Wait till conversion is finished
	}
	
	return EXIT_SUCCESS;
}

    /* Read the force sensor and set the value as new zero value.
	 * Returns 1 on success and 0 on failure
	 */
int force_sense_calibrate() {
	
	force_sense_read_sense();
	force_sense_zero_value = force_sense_last_readout;
	
	return EXIT_SUCCESS;
}

int force_sense_get_newton() {
	/* Trigger a force sensor reading and converts the value to newton and returns it. */
}

int force_sense_get_gramm() {
	/* Trigger a force sensor reading and converts the value to gram then returns it. */
}