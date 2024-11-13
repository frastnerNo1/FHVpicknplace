/*
 * plc_com.h
 *
 * Created: 18.09.2024 09:34:34
 *  Author: floro
 */ 


#ifndef SPS_COM_H_
#define SPS_COM_H_

#include "main.h"

typedef enum errorCodes {
	e_not_init      = '1',
	e_wrong_tool    = '2',
	e_general_error = '0'
} Error_Code_t;

void plc_com_success(void);

void plc_com_error(Error_Code_t);

void plc_com_transmit_force(int16_t);

void plc_com_receive_callback(void);

void plc_com_arm_receiver(void);

#endif /* PLC_COM_H_ */