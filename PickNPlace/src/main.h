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
#define PLC_COM_RX_PIN                  EXT1_PIN_9
#define PLC_COM_TX_PIN                  EXT1_PIN_10
#define STEPPER_SPI_CLK_PIN             EXT1_PIN_18
#define STEPPER_SPI_MOSI_PIN            EXT1_PIN_16
#define STEPPER_SPI_MISO_PIN            EXT1_PIN_17

#define MOTOR_CONTROLLER_DIR_PIN        EXT2_PIN_5
#define MOTOR_CONTROLLER_STP_PIN        EXT2_PIN_6
#define MAGNET_SWITCH_PIN               EXT2_PIN_7
#define Z_AXIS_ZERO_SWITCH_PIN          EXT2_PIN_8

#define FORCE_SENSE_INPUT_PIN           EXT3_PIN_3
#define ADC_REFERENCE_PIN				EXT3_PIN_4

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

extern struct spi_module gSpiMasterInstance;
extern struct spi_slave_inst gSpiMotorController;
extern struct adc_module gAdcInstance;
extern struct usart_module gUsartInstance;


int set_state(enum system_states);

enum system_states get_state(void);

#endif /* MAIN_H_ */