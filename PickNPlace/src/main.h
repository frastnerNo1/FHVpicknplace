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

#define STEPPER_PULSE_PERIOD_us         100

#define Z_AXIS_MAX_TRAVEL               100
#define Z_AXIS_MM_PER_REV               10
#define Z_AXIS_MICROSTEPS               8

#define Z_AXIS_STEPS_PER_REV            (200 * Z_AXIS_MICROSTEPS)
#define Z_AXIS_STEPS_PER_MM             (uint16_t)(Z_AXIS_STEPS_PER_REV / Z_AXIS_MM_PER_REV)

extern struct spi_module spi_master_instance;
extern struct spi_slave_inst spi_motor_controller;
extern struct adc_module adc_instance;

#endif /* MAIN_H_ */