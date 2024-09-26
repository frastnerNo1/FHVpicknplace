/*
 * plc_com.h
 *
 * Created: 18.09.2024 09:34:34
 *  Author: floro
 */ 


#ifndef SPS_COM_H_
#define SPS_COM_H_

#include "main.h"

void plc_com_success(void);

void plc_com_transmit_force(int16_t);

void plc_com_receive_callback(void);

void plc_com_arm_receiver(void);

#endif /* PLC_COM_H_ */