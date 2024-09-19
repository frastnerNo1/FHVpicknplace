/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * Main function with loop. Here just the state machine is implemented, all the other functionality is handled in dedicated functions.
 * Also the callback for incoming usart com from the PLC is register in this file.
 *
 */

#include <asf.h>
#include "main.h"
#include "drv_ctrl.h"

struct spi_module spi_master_instance;
struct spi_slave_inst spi_motor_controller;

static void configure_spi_master(void){
	struct spi_config config_spi_master;
	struct spi_slave_inst_config motor_controller_config;
	
	spi_slave_inst_get_config_defaults(&motor_controller_config);
	motor_controller_config.ss_pin = MOTOR_CONTROLLER_SS_PIN;
	spi_attach_slave(&spi_motor_controller, &motor_controller_config);
	
	spi_get_config_defaults(&config_spi_master);
	config_spi_master.transfer_mode = SPI_TRANSFER_MODE_0;
	config_spi_master.data_order = SPI_DATA_ORDER_MSB;
	config_spi_master.mux_setting = EXT1_SPI_SERCOM_MUX_SETTING;
	config_spi_master.pinmux_pad0 = EXT1_SPI_SERCOM_PINMUX_PAD0;
	config_spi_master.pinmux_pad1 = PINMUX_UNUSED;
	config_spi_master.pinmux_pad2 = EXT1_SPI_SERCOM_PINMUX_PAD2;
	config_spi_master.pinmux_pad3 = EXT1_SPI_SERCOM_PINMUX_PAD3;
	spi_init(&spi_master_instance, EXT1_SPI_MODULE, &config_spi_master);
	
	spi_enable(&spi_master_instance);
}

static void configure_stepper_motor(void) {
	struct drv_config_struct stepper_motor_config;
	
	stepper_motor_config.direction_set = DRV_DIRPIN;
	stepper_motor_config.step_mode = DRV_MODE_1_4;
	stepper_motor_config.stall_detect = DRV_EXSTALL_INTERNAL;
	stepper_motor_config.isense_gain = DRV_ISGAIN_40;
	stepper_motor_config.dead_time_insert = DRV_DTIME_850ns; //Check if change is needed
	stepper_motor_config.drv_torque = 0x96;
	stepper_motor_config.backemf_sample_th = DRV_SMPLTH_200us; //Check if change is needed
	stepper_motor_config.drv_toff = 0x30; // Check if change is needed
	stepper_motor_config.pwm_mode = DRV_PWMMODE_INTERNAL;
	stepper_motor_config.drv_tblank = 0x80; //Check if change is needed
	stepper_motor_config.adaptive_blanking_time= DRV_ABT_DISABLE;
	stepper_motor_config.drv_tdecay = 0x10; //Check if check is needed
	stepper_motor_config.decay_mode = DRV_DECMOD_FORCE_SLOWINC_FASTDEC; //Check if change is needed
	stepper_motor_config.ocp_threshold = DRV_OCPTH_250mV;
	stepper_motor_config.ocp_deglitch_time = DRV_OCPDEG_4us;
	stepper_motor_config.ls_drive_time = DRV_TDRIVEN_1us;
	stepper_motor_config.hs_drive_time = DRV_TDRIVEP_1us;
	stepper_motor_config.ls_current = DRV_IDRIVEN_100mA;
	stepper_motor_config.hs_current = DRV_IDRIVEP_100mA;
	
	/* Following lines are for stall detection which is not in use*/
	stepper_motor_config.drv_sdthr = 0x40; //Check if change is needed
	stepper_motor_config.stall_count = DRV_SDCNT_4; //Check if change is needed
	stepper_motor_config.back_emf_div = DRV_VDIV_32;
	
	drv_ctrl_init(&stepper_motor_config);
	drv_ctrl_enable();
	
}

static void configure_port_pins(void)
{
	struct port_config config_port_pin;
	port_get_config_defaults(&config_port_pin);
	config_port_pin.direction  = PORT_PIN_DIR_OUTPUT;
	config_port_pin.input_pull = PORT_PIN_PULL_DOWN;
	port_pin_set_config(MOTOR_CONTROLLER_DIR_PIN, &config_port_pin);
	port_pin_set_config(MOTOR_CONTROLLER_STP_PIN, &config_port_pin);
}


int main (void)
{
	system_init();
	delay_init();
    configure_spi_master();
	configure_stepper_motor();
	configure_port_pins();
	
	while (1) {
		
		drv_ctrl_moveto(10);
		delay_ms(2000);
		drv_ctrl_moveto(0);
		delay_ms(2000);
		
	}
}
