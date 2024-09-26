/*
 * stepper_music.c
 *
 * Created: 24.09.2024 13:57:40
 *  Author: floro
 */ 

#include <math.h>
#include "stepper_music.h"
#include "drv_ctrl.h"

const uint32_t cDuration_ms = 200;
const uint16_t cDurationPause_ms = 500;

void stepper_music_play(uint16_t * notes, uint16_t len){
	
	uint16_t del = 0;
	uint16_t count = 0;
	
	drv_ctrl_set_microsteps(DRV_MODE_1_2);
	
	for(uint16_t i = 0; i < len; i++){
		
		uint16_t note = *notes;
		
		if(note == 0) {
			delay_ms(cDurationPause_ms);
			notes ++;
			continue;
		}
		
		del = 1000000 / note;
		count = (cDuration_ms * 1000) / del;
		for(int x = 0; x < count; x++) {
			port_pin_toggle_output_level(MOTOR_CONTROLLER_DIR_PIN);
			port_pin_set_output_level(MOTOR_CONTROLLER_STP_PIN, true);
			delay_us(del/2);
			port_pin_set_output_level(MOTOR_CONTROLLER_STP_PIN, false);
			delay_us(del/2);
		}
		
		notes++;
	}
	set_state(success);
}