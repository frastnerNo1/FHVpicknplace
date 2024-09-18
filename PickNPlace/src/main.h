/*
 * main.h
 *
 * Created: 18.09.2024 09:31:51
 *  Author: floro
 */ 


#ifndef MAIN_H_
#define MAIN_H_

#define MOTOR_CONTROLLER_SS_PIN         EXT1_PIN_5

extern struct spi_module spi_master_instance;
extern struct spi_slave_inst spi_motor_controller;

#endif /* MAIN_H_ */