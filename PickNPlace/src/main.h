/*
 * main.h
 *
 * Created: 18.09.2024 09:31:51
 *  Author: floro
 */ 


#ifndef MAIN_H_
#define MAIN_H_

/*
 * Define the test mode at compile time:
 * 0 = OFF, normal program mode
 * 1 = FORCE test, ADC output is printed to console every second
 * 2 = COM test, UART inputs are plotted on the console
 */
#define TESTMODE    2

/*Global enable for console logs: 0 = OFF, 1 = LOG, 2 = VERBOSE LOG*/
#define LOGS        1

#include <asf.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#if LOGS != 0 || TESTMODE != 0
#include "rprintf.h"
#endif

#define FORCE_SENSE_INPUT_PIN           EXT1_PIN_3
#define MOTOR_CONTROLLER_SS_PIN         EXT1_PIN_5
#define PLC_COM_RX_PIN                  EXT1_PIN_13
#define PLC_COM_TX_PIN                  EXT1_PIN_14
#define STEPPER_SPI_MOSI_PIN            EXT1_PIN_16
#define STEPPER_SPI_MISO_PIN            EXT1_PIN_17
#define STEPPER_SPI_CLK_PIN             EXT1_PIN_18

#define MOTOR_CONTROLLER_DIR_PIN        EXT2_PIN_5
#define MOTOR_CONTROLLER_STP_PIN        EXT2_PIN_6
#define MAGNET_SWITCH_PIN               EXT2_PIN_7
#define Z_AXIS_ZERO_SWITCH_PIN          EXT2_PIN_8

#define ADC_REFERENCE_PIN				EXT3_PIN_4  //NOT IN USE

typedef enum system_states {
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
	music,
	success
	} System_State_t;    

extern struct spi_module gSpiMasterInstance;
extern struct spi_slave_inst gSpiMotorController;
extern struct adc_module gAdcInstance;
extern struct usart_module gUsartInstance;

/* Setter function for the system state */
int set_state(System_State_t);

/* Getter function for the system state */
System_State_t get_state(void);

#endif /* MAIN_H_ */