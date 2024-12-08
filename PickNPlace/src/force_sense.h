/*
 * force_sense.h
 *
 * Created: 18.09.2024 09:34:04
 *  Author: floro
 */ 


#ifndef FORCE_SENSE_H_
#define FORCE_SENSE_H_

#include "main.h"

/* Used bridge circuit: 0 = MAIN PCB, 1 = independent bridge PCB*/
#define INA_BRIDGE                  0

#if INA_BRIDGE == 0
#define FORCE_SENSE_mN_PER_BIT      2
#endif

#if INA_BRIDGE == 1
#define FORCE_SENSE_mN_PER_BIT      6.26f
#endif

#define FORCE_SENSE_g_PER_BIT       (FORCE_SENSE_mN_PER_BIT * 0.1f)

#define AVERAGE_NUMBER              10
#define AVG(a, b)                   ((a >> 1) + (b >> 1) + (a & b & 0x1))

void force_sense_calibrate(void);

int16_t force_sense_get_millinewton(void);

int16_t force_sense_get_gramm(void);

uint32_t force_sense_get_uV(void);

#endif /* FORCE_SENSE_H_ */