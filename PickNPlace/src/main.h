/*
 * main.h
 *
 * Created: 18.09.2024 09:31:51
 *  Author: floro
 */ 


#ifndef MAIN_H_
#define MAIN_H_

#include <asf.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define MOTOR_CONTROLLER_SS_PIN         EXT1_PIN_5
#define MOTOR_CONTROLLER_DIR_PIN        EXT2_PIN_5
#define MOTOR_CONTROLLER_STP_PIN        EXT2_PIN_6
#define FORCE_SENSE_INPUT_PIN           EXT3_PIN_3
#define ADC_REFERENCE_PIN				EXT3_PIN_4
#define PLC_COM_TX_PIN
#define PLC_COM_RX_PIN
#define MAGNET_SWITCH_PIN               EXT2_PIN_7
#define Z_AXIS_ZERO_SWITCH_PIN          EXT2_PIN_8

#define DIRECTION_UP                    true
#define DIRECTION_DOWN                  !DIRECTION_UP

#define PICK_HEIGHT_mm                  150
#define PLACE_HEIGHT_mm                 100
#define TOOL_GRAB_HEIGHT_mm             100
#define TOOL_DROP_HEIGHT_mm             100
#define SOAK_HEIGHT_mm                  100
#define STAMP_HEIGHT_mm                 50
#define CLOSE_HEIGHT_mm                 50
#define TRAVEL_HEIGHT_mm                5

#define WAIT_TIME_ms                    2000

#define SOAK_FORCE_mN                   (int16_t)1000
#define STAMP_FORCE_mN                  (int16_t)1000

#define STEPPER_PULSE_PERIOD_us         500
#define STEPPER_PULSE_SLOW_PERIOD_us    3000

#define Z_AXIS_MAX_TRAVEL               100
#define Z_AXIS_MM_PER_REV               10
#define Z_AXIS_MICROSTEPS               2

#define Z_AXIS_STEPS_PER_REV            (200 * Z_AXIS_MICROSTEPS)
#define Z_AXIS_STEPS_PER_MM             (uint16_t)(Z_AXIS_STEPS_PER_REV / Z_AXIS_MM_PER_REV)

#define FORCE_SENSE_mN_PER_COUNT        10
#define FORCE_SENSE_g_PER_COUNT         (uint16_t)(FORCE_SENSE_mN_PER_COUNT * 9.81)

enum system_states {
	start,
	idle,
	busy,
	init,
	pick,
	place,
	change_tool,
	stamp,
	soak,
	close_lid,
	get_force,
	success
	};    

extern struct spi_module spi_master_instance;
extern struct spi_slave_inst spi_motor_controller;
extern struct adc_module adc_instance;
extern struct usart_module usart_instance;

int set_state(enum system_states);

enum system_states get_state(void);

#endif /* MAIN_H_ */