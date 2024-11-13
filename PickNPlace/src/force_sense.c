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

static void force_sense_read_sense(void);

    /* 
	 * @brief: Reads the voltage from the INA via ADC, save the raw value in memory.
	 */
static void force_sense_read_sense() {
	
	adc_start_conversion(&gAdcInstance);
	
	while(adc_read(&gAdcInstance, &sForceSenseLastReadout) == STATUS_BUSY){
		//Wait till conversion is finished
	}

    #if LOGS == 2
    rprintf("LOG: new RAW ADC value: %d", sForceSenseLastReadout);
    #endif
}

    /* 
	 * @brief: Read the force sensor and set the value as new zero value.
	 */
void force_sense_calibrate() {
	
	force_sense_read_sense();
	sForceSenseZeroValue = sForceSenseLastReadout;
}

    /* 
	 * @brief: Trigger a force sensor readout.
     * @returns: force value as signed number in mN
	 */
int16_t force_sense_get_millinewton() {
	
	force_sense_read_sense();
	int16_t force_mN = (sForceSenseLastReadout - sForceSenseZeroValue) / FORCE_SENSE_mN_PER_COUNT;
	
	return force_mN;
}


    /*
	 * @brief: Trigger a force sensor readout and converts the value to gram then returns it.
     * @brief: force value as signed number in gramm
	 */
int16_t force_sense_get_gramm() {
	
	force_sense_read_sense();
	int16_t force_g = (sForceSenseLastReadout - sForceSenseZeroValue) / FORCE_SENSE_g_PER_COUNT;
	
	return force_g;
}

/* This function is only used in testmode 1 to get the value of the ADC in micro volts. */
#if TESTMODE == 1
uint32_t force_sense_get_uV() {
    force_sense_read_sense();
    
    return sForceSenseLastReadout * 244;
}
#endif