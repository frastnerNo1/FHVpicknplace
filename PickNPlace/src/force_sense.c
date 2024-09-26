/*
 * force_sense.c
 *
 * Created: 18.09.2024 09:33:52
 *  Author: floro
 */ 

/* In this file the force sensor readout is implemented. */

#include "force_sense.h"

static uint16_t sForceSenseLastReadout;
static uint16_t sForceSenseZeroValue;

    /* Reads the voltage from the INA via ADC, save the raw value in memory. Returns 1 on success and 0 on error*/
static void force_sense_read_sense(void) {
	
	adc_start_conversion(&gAdcInstance);
	
	while(adc_read(&gAdcInstance, &sForceSenseLastReadout) == STATUS_BUSY){
		//Wait till conversion is finished
	}
}

    /* Read the force sensor and set the value as new zero value.
	 * Returns 1 on success and 0 on failure
	 */
void force_sense_calibrate() {
	
	force_sense_read_sense();
	sForceSenseZeroValue = sForceSenseLastReadout;
}

    /* Trigger a force sensor reading and converts the value to millinewton and returns it. */
int16_t force_sense_get_millinewton() {
	
	force_sense_read_sense();
	int16_t force_mN = (sForceSenseLastReadout - sForceSenseZeroValue) / FORCE_SENSE_mN_PER_COUNT;
	
	return force_mN;
}


    /* Trigger a force sensor reading and converts the value to gram then returns it. */
int16_t force_sense_get_gramm() {
	
	force_sense_read_sense();
	int16_t force_g = (sForceSenseLastReadout - sForceSenseZeroValue) / FORCE_SENSE_g_PER_COUNT;
	
	return force_g;
}