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
#define MAGNET_SWITCH_PIN
#define Z_AXIS_ZERO_SWITCH_PIN

#define DIRECTION_UP                    true
#define DIRECTION_DOWN                  !DIRECTION_UP

#define STEPPER_PULSE_PERIOD_us         500

#define Z_AXIS_MAX_TRAVEL               100
#define Z_AXIS_MM_PER_REV               10
#define Z_AXIS_MICROSTEPS               2

#define Z_AXIS_STEPS_PER_REV            (200 * Z_AXIS_MICROSTEPS)
#define Z_AXIS_STEPS_PER_MM             (uint16_t)(Z_AXIS_STEPS_PER_REV / Z_AXIS_MM_PER_REV)

#define FORCE_SENSE_mN_PER_COUNT        10
#define FORCE_SENSE_g_PER_COUNT         (uint16_t)(FORCE_SENSE_mN_PER_COUNT * 9.81)

enum system_states {
	idle,
	busy,
	init,
	pick,
	place,
	get_tool,
	drop_tool,
	stamp,
	soak,
	close_lid,
	get_force
	};    

extern struct spi_module spi_master_instance;
extern struct spi_slave_inst spi_motor_controller;
extern struct adc_module adc_instance;
extern struct usart_module usart_instance;

void set_state(enum system_states);

enum system_states get_state(void);

#endif /* MAIN_H_ */