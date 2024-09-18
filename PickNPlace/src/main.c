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

struct spi_module spi_master_instance;
struct spi_slave_inst spi_motor_controller;

void configure_spi_master(){
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

int main (void)
{
	system_init();
    configure_spi_master();
	
	while (1) {
		
		
		
	}
}
