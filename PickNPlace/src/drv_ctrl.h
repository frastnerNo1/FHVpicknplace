/*
 * drv_ctrl.h
 *
 * Created: 18.09.2024 09:33:37
 *  Author: floro
 */ 


#ifndef DRV_CTRL_H_
#define DRV_CTRL_H_

#include "main.h"

#define DIRECTION_UP                    true
#define DIRECTION_DOWN                  !DIRECTION_UP

#define STEPPER_PULSE_PERIOD_us         50
#define STEPPER_PULSE_SLOW_PERIOD_us    1000

#define Z_AXIS_MAX_TRAVEL               300
#define Z_AXIS_MM_PER_REV               10

#define Z_AXIS_STEPS_PER_REV            200
#define Z_AXIS_STEPS_PER_MM             (uint16_t)(Z_AXIS_STEPS_PER_REV / Z_AXIS_MM_PER_REV)


/* Registers of DRV8711 */

#define CTRL_REG        0x00
#define TORQUE_REG      0x01
#define OFF_REG         0x02
#define BLANK_REG       0x03
#define DECAY_REG       0x04
#define STALL_REG       0x05
#define DRIVE_REG       0x06
#define STATUS_REG      0x07

	enum drv_en {
		DRV_ENABLE             = (1 << 0),
		DRV_DISABLE            = (0 << 0)
		};

	enum drv_rdir {
		/** Direction by DIR Pin */
		DRV_DIRPIN             = (0 << 1),
		/** Direction by inverse DIR Pin */
		DRV_INVDIRPIN          = (1 << 1),
	};
	
	enum drv_mode {
		/** Full-step*/
		DRV_MODE_1             = (0b0000 << 3),
		/** Half-step*/
		DRV_MODE_1_2           = (0b0001 << 3),
		/** 1/4 step*/
		DRV_MODE_1_4           = (0b0010 << 3),
		/** 1/8 step*/
		DRV_MODE_1_8           = (0b0011 << 3),
		/** 1/16 step*/
		DRV_MODE_1_16          = (0b0100 << 3),
		/** 1/32 step*/
		DRV_MODE_1_32          = (0b0101 << 3),
		/** 1/64 step*/
		DRV_MODE_1_64          = (0b0110 << 3),
		/** 1/128 step*/
		DRV_MODE_1_128         = (0b0111 << 3),
		/** 1/256 step*/
		DRV_MODE_1_256         = (0b1000 << 3)
	};
	
	enum drv_exstall {
		/** Internal stall detect*/
		DRV_EXSTALL_INTERNAL   = 0 << 7,
		/** External stall detect*/
		DRV_EXSTALL_EXTERNAL   = 1 << 7
	};
	
	enum drv_isgain {
		DRV_ISGAIN_5           = 0b00 << 8,
		DRV_ISGAIN_10          = 0b01 << 8,
		DRV_ISGAIN_20          = 0b10 << 8,
		DRV_ISGAIN_40          = 0b11 << 8
	};
	
	enum drv_dtime {
		DRV_DTIME_400ns        = 0b00 << 10,
		DRV_DTIME_450ns        = 0b01 << 10,
		DRV_DTIME_650ns        = 0b10 << 10,
		DRV_DTIME_850ns        = 0b11 << 10
	};
	
	enum drv_smplth {
		DRV_SMPLTH_50us        = 0b000 << 8,
		DRV_SMPLTH_100us       = 0b001 << 8,
		DRV_SMPLTH_200us       = 0b010 << 8,
		DRV_SMPLTH_300us       = 0b011 << 8,
		DRV_SMPLTH_400us       = 0b100 << 8,
		DRV_SMPLTH_600us       = 0b101 << 8,
		DRV_SMPLTH_800us       = 0b110 << 8,
		DRV_SMPLTH_1000us      = 0b111 << 8
	};
	
	enum drv_pwmmode {
		DRV_PWMMODE_INTERNAL   = 0 << 8,
		DRV_PWMMODE_EXTERNAL   = 1 << 8
	};
	
	enum drv_abt {
		DRV_ABT_DISABLE        = 0 << 8,
		DRV_ABT_ENABLE         = 1 << 8
	};
	
	enum drv_decmod {
		DRV_DECMOD_FORCE_SLOW             = 0b000 << 0,
		DRV_DECMOD_FORCE_SLOWINC_FASTDEC  = 0b001 << 0,
		DRV_DECMOD_FORCE_FAST             = 0b010 << 0,
		DRV_DECMOD_FORCE_MIXED            = 0b011 << 0,
		DRV_DECMOD_FORCE_SLOWINC_AUTODEC  = 0b100 << 0,
		DRV_DECMOD_FORCE_AUTOMIXED        = 0b101 << 0
	};
	
	enum drv_sdcnt {
		DRV_SDCNT_1        = 0b00 << 8,
		DRV_SDCNT_2        = 0b01 << 8,
		DRV_SDCNT_4        = 0b10 << 8,
		DRV_SDCNT_8        = 0b11 << 8
	};
	
	enum drv_vdiv {
		DRV_VDIV_32        = 0b00 << 10,
		DRV_VDIV_16        = 0b01 << 10,
		DRV_VDIV_8         = 0b10 << 10,
		DRV_VDIV_4         = 0b11 << 10
	};
	
	enum drv_ocpth {
		DRV_OCPTH_250mV    = 0b00 << 0,
		DRV_OCPTH_500mV    = 0b01 << 0,
		DRV_OCPTH_750mV    = 0b10 << 0,
		DRV_OCPTH_1000mV   = 0b11 << 0
	};
	
	enum drv_ocpdeg {
		DRV_OCPDEG_1us     = 0b00 << 2,
		DRV_OCPDEG_2us     = 0b01 << 2,
		DRV_OCPDEG_4us     = 0b10 << 2,
		DRV_OCPDEG_8us     = 0b11 << 2
	};
	
	enum drv_tdriven {
		DRV_TDRIVEN_250ns  = 0b00 << 4,
		DRV_TDRIVEN_500ns  = 0b01 << 4,
		DRV_TDRIVEN_1us    = 0b10 << 4,
		DRV_TDRIVEN_2us    = 0b11 << 4
	};
	
	enum drv_tdrivep {
		DRV_TDRIVEP_250ns  = 0b00 << 6,
		DRV_TDRIVEP_500ns  = 0b01 << 6,
		DRV_TDRIVEP_1us    = 0b10 << 6,
		DRV_TDRIVEP_2us    = 0b11 << 6
	};
	
	enum drv_idriven {
		DRV_IDRIVEN_100mA  = 0b00 << 8,
		DRV_IDRIVEN_200mA  = 0b01 << 8,
		DRV_IDRIVEN_300mA  = 0b10 << 8,
		DRV_IDRIVEN_400mA  = 0b11 << 8
		
	};
	
	enum drv_idrivep {
		DRV_IDRIVEP_100mA  = 0b00 << 10,
		DRV_IDRIVEP_200mA  = 0b01 << 10,
		DRV_IDRIVEP_300mA  = 0b10 << 10,
		DRV_IDRIVEP_400mA  = 0b11 << 10
	};

typedef struct drv_config_struct {
	
	enum drv_en enable;
	
	bool drv_step;
	
	/*CTRL_RDIR*/
	enum drv_rdir direction_set;
	/*CTRL_MODE*/
	enum drv_mode step_mode;
	/*CTRL_EXSTALL*/
	enum drv_exstall stall_detect;
	/*CTRL_ISGAIN*/
	enum drv_isgain isense_gain;
	/*CTRL_DTIME*/
	enum drv_dtime dead_time_insert;
	
	/*TORQUE_TORQUE*/
	uint8_t drv_torque;
	/*TORQUE_SMPLTH*/
	enum drv_smplth backemf_sample_th;
	
	/*OFF_TOFF*/
	uint8_t drv_toff;
	/*OFF_PWMMODE*/
	enum drv_pwmmode pwm_mode;
	
    /*BLANK_TBLANK*/
	uint8_t drv_tblank;
	/*BLANK_ABT*/
	enum drv_abt adaptive_blanking_time;
	
	/*DECAY_TDECAY*/
	uint8_t drv_tdecay;
	/*DECAY_DECMOD*/
	enum drv_decmod decay_mode;
	
	/*STALL_SDTHR*/
	uint8_t drv_sdthr;
	/*STALL_SDCNT*/
	enum drv_sdcnt stall_count;
	/*STALL_VDIV*/
	enum drv_vdiv back_emf_div;
	
	/*DRIVE_OCPTH*/
	enum drv_ocpth ocp_threshold;
	/*DRIVE_OCPDEG*/
	enum drv_ocpdeg ocp_deglitch_time;
	/*DRIVE_TDRIVEN*/
	enum drv_tdriven ls_drive_time;
	/*DRIVE_TDRIVEP*/
	enum drv_tdrivep hs_drive_time;
	/*DRIVE_IDRIVEN*/
	enum drv_idriven ls_current;
	/*DRIVE_IDRIVEP*/
	enum drv_idrivep hs_current;
	
	} Driver_Instance_t;
	
void drv_ctrl_init(Driver_Instance_t *);

void drv_ctrl_enable(void);

void drv_ctrl_disable(void);

void drv_ctrl_set_microsteps(enum drv_mode);

void drv_ctrl_home(void);

void drv_ctrl_moveto(uint16_t);

void drv_ctrl_move_till_force(uint16_t);

#endif /* DRV_CTRL_H_ */