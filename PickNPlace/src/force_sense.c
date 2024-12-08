/*
 * force_sense.c
 *
 * Created: 18.09.2024 09:33:52
 *  Author: floro
 */ 

/* In this file the force sensor readout is implemented. */

#include "force_sense.h"

static uint32_t sForceSenseLastReadout;
static uint16_t sForceSenseZeroForce;

static void force_sense_read_sense(void);

    /* 
	 * @brief: Reads the voltage from the INA via ADC, save the raw value in memory.
	 */
static void force_sense_read_sense() {

    uint16_t forceBuffer = 0;
    sForceSenseLastReadout = 0;
	
    for(uint8_t i = 0; i <= AVERAGE_NUMBER; i++){
	    adc_start_conversion(&gAdcInstance);
	
	    while(adc_read(&gAdcInstance, &forceBuffer) == STATUS_BUSY){
		    //Wait till conversion is finished
	    }
        sForceSenseLastReadout = AVG(sForceSenseLastReadout, forceBuffer);
    }

    #if LOGS == 2
    rprintf("LOG: new RAW ADC value: %d\r\n", sForceSenseLastReadout);
    #endif
}

    /* 
	 * @brief: Read the force sensor and set the value as new zero value.
	 */
void force_sense_calibrate() {
	
	force_sense_read_sense();
	sForceSenseZeroForce = sForceSenseLastReadout * FORCE_SENSE_mN_PER_BIT;
}

    /* 
	 * @brief: Trigger a force sensor readout.
     * @returns: force value as signed number in mN
	 */
int16_t force_sense_get_millinewton() {
	
	force_sense_read_sense();
	uint16_t absForce_mN = sForceSenseLastReadout * FORCE_SENSE_mN_PER_BIT;
	
	return absForce_mN - sForceSenseZeroForce;
}


    /*
	 * @brief: Trigger a force sensor readout and converts the value to gram then returns it.
     * @brief: force value as signed number in gramm
	 */
int16_t force_sense_get_gramm() {
	
	return force_sense_get_millinewton() * 10;
}

/* This function is only used in testmode 1 to get the value of the ADC in micro volts. */
#if TESTMODE == 1
uint32_t force_sense_get_uV() {
    force_sense_read_sense();
    
    return sForceSenseLastReadout * 244;
}
#endif