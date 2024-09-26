/*
 * force_sense.h
 *
 * Created: 18.09.2024 09:34:04
 *  Author: floro
 */ 


#ifndef FORCE_SENSE_H_
#define FORCE_SENSE_H_

#include "main.h"

#define FORCE_SENSE_mN_PER_COUNT        10
#define FORCE_SENSE_g_PER_COUNT         (uint16_t)(FORCE_SENSE_mN_PER_COUNT * 9.81)

void force_sense_calibrate(void);

int16_t force_sense_get_millinewton(void);

int16_t force_sense_get_gramm(void);

#endif /* FORCE_SENSE_H_ */