/*
 * stepper_music.c
 *
 * Created: 24.09.2024 13:57:40
 *  Author: floro
 */ 

#include <math.h>
#include "stepper_music.h"
#include "main.h"

const uint8_t oct = 1;
const uint8_t tempo = 50;
const uint16_t duration = 100;

void play_music(uint16_t * notes){
	
	int del = 10;
	int count = 0;
	
	for(uint16_t i = 0; i < 13; i++){
		
		uint16_t note = *notes;
	
		del=(note*oct)/10;
		count=floor((duration*5*tempo)/del);
		for(int x = 0; x < count; x++) {
			port_pin_set_output_level(MOTOR_CONTROLLER_STP_PIN, true);
			delay_us(del);
			port_pin_set_output_level(MOTOR_CONTROLLER_STP_PIN, false);
			delay_us(del);
		}
		
		notes++;
	}
}
