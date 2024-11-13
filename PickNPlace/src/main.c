/**
 * TITLE: FHVPickNPlace
 * FOCUSPROJECT MECHATRONIC FHV
 *
 * AUTOR: Florian Ronacher
 * CREATED: 18.09.2024
 * VERSION: 1.0
 *
 */

/**
 * Description:
 * Config of the MCU peripherals is done here.
 * State machine is implemented here, the switch is done based on the system_state, which is maintained by the setter function.
 * Also the callback for incoming UART communication from the PLC is registered in this file.
 *
 */

#include "main.h"
#include "drv_ctrl.h"
#include "plc_com.h"
#include "force_sense.h"
#include "z_axis.h"
#include "stepper_music.h"

/* Function prototype for the loop to check the PLC com. */
void test_loop(void);

/* Global instances of the peripherals. */
struct spi_module gSpiMasterInstance;
struct spi_slave_inst gSpiMotorController;
struct adc_module gAdcInstance;
struct usart_module gUsartInstance;

static System_State_t sSystemState;

static void configure_spi_master(void){
	struct spi_config config_spi_master;
	struct spi_slave_inst_config motor_controller_config;
	
	spi_slave_inst_get_config_defaults(&motor_controller_config);
	motor_controller_config.ss_pin = MOTOR_CONTROLLER_SS_PIN;
	spi_attach_slave(&gSpiMotorController, &motor_controller_config);
	
	spi_get_config_defaults(&config_spi_master);
	config_spi_master.transfer_mode = SPI_TRANSFER_MODE_3;    //Acording to datasheet of the board
	config_spi_master.data_order = SPI_DATA_ORDER_MSB;
	config_spi_master.mux_setting = EXT1_SPI_SERCOM_MUX_SETTING;
	config_spi_master.pinmux_pad0 = EXT1_SPI_SERCOM_PINMUX_PAD0;
	config_spi_master.pinmux_pad1 = PINMUX_UNUSED;
	config_spi_master.pinmux_pad2 = EXT1_SPI_SERCOM_PINMUX_PAD2;
	config_spi_master.pinmux_pad3 = EXT1_SPI_SERCOM_PINMUX_PAD3;
	spi_init(&gSpiMasterInstance, EXT1_SPI_MODULE, &config_spi_master);
	
	spi_enable(&gSpiMasterInstance);
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
	stepper_motor_config.drv_toff = 0x80; // Check if change is needed
	stepper_motor_config.pwm_mode = DRV_PWMMODE_INTERNAL;
	stepper_motor_config.drv_tblank = 0xFF; //Check if change is needed
	stepper_motor_config.adaptive_blanking_time= DRV_ABT_DISABLE;
	stepper_motor_config.drv_tdecay = 0x10; //Check if check is needed
	stepper_motor_config.decay_mode = DRV_DECMOD_FORCE_AUTOMIXED; //Check if change is needed
	stepper_motor_config.ocp_threshold = DRV_OCPTH_250mV;
	stepper_motor_config.ocp_deglitch_time = DRV_OCPDEG_4us;
	stepper_motor_config.ls_drive_time = DRV_TDRIVEN_1us;
	stepper_motor_config.hs_drive_time = DRV_TDRIVEP_1us;
	stepper_motor_config.ls_current = DRV_IDRIVEN_100mA;
	stepper_motor_config.hs_current = DRV_IDRIVEP_100mA;
	
	/* Following 3 lines are for stall detection which is currently not in use*/
	stepper_motor_config.drv_sdthr = 0x40;
	stepper_motor_config.stall_count = DRV_SDCNT_4;
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
	port_pin_set_config(MOTOR_CONTROLLER_SS_PIN, &config_port_pin);
	port_pin_set_config(MAGNET_SWITCH_PIN, &config_port_pin);
	config_port_pin.direction = PORT_PIN_DIR_INPUT;
	config_port_pin.input_pull = PORT_PIN_PULL_UP;
	port_pin_set_config(Z_AXIS_ZERO_SWITCH_PIN, &config_port_pin);
}


static void configure_adc(void) // TODO: Check  if calibration is needed
{
	struct adc_config config_adc;
	adc_get_config_defaults(&config_adc);
	config_adc.negative_input = ADC_NEGATIVE_INPUT_GND; //Can be muxed to external pin
	config_adc.positive_input = ADC_POSITIVE_INPUT_PIN0;
	config_adc.reference = ADC_REFERENCE_INT1V;
    config_adc.accumulate_samples = ADC_ACCUMULATE_SAMPLES_1024;
	adc_init(&gAdcInstance, ADC, &config_adc);
	adc_enable(&gAdcInstance);
}

static void configure_usart(void){
	struct usart_config config_usart;
	usart_get_config_defaults(&config_usart);
	
	config_usart.baudrate = 9600;
	config_usart.mux_setting = EXT1_UART_SERCOM_MUX_SETTING;
	config_usart.pinmux_pad0 = EXT1_UART_SERCOM_PINMUX_PAD0;
	config_usart.pinmux_pad1 = EXT1_UART_SERCOM_PINMUX_PAD1;
	config_usart.pinmux_pad2 = EXT1_UART_SERCOM_PINMUX_PAD2;
	config_usart.pinmux_pad3 = EXT1_UART_SERCOM_PINMUX_PAD3;
	
	while(usart_init(&gUsartInstance, EXT1_UART_MODULE, &config_usart) != STATUS_OK){
		delay_ms(1000);
	}
	
	usart_enable(&gUsartInstance);
	
	usart_enable_transceiver(&gUsartInstance, USART_TRANSCEIVER_TX);
	usart_enable_transceiver(&gUsartInstance, USART_TRANSCEIVER_RX);
}

/* UART callback is implemented in plc_com file*/
static void configure_usart_callbacks(void){
	
	usart_register_callback(&gUsartInstance,
	        plc_com_receive_callback, USART_CALLBACK_BUFFER_RECEIVED);
			
	usart_enable_callback(&gUsartInstance, USART_CALLBACK_BUFFER_RECEIVED);
}

/*
 * @brief: changing the state variable at one single point. If system is in start state, just init is allowed.
 * @param: new system state of type system_states.
 * @returns: 0 when state is successfully set, 1 if change is not allowed
 */
int set_state(System_State_t new_state) {
	
	if(sSystemState == start && new_state != init){
        #if LOGS > 0
        rprintf("LOG: new state failed, not init.");
        #endif
		return EXIT_FAILURE;
	} else {
		sSystemState = new_state;
        #if LOGS > 0
        rprintf("LOG: new state = %d", sSystemState);
        #endif
		return EXIT_SUCCESS;
	}
}

/*
 * @brief: get the actual system state at one single point.
 * @returns: current state of type system_states
 */
System_State_t get_state(void) {
	
	return sSystemState;
	
}

uint16_t notes1[15] = {7000, 6000, 7000, 3000, 3000, 0, 7000, 6000, 7000, 6000, 5500, 5500, 7000, 6000, 7000};


int main (void)
{
	system_init();
	delay_init();
	configure_port_pins();
    configure_spi_master();
	configure_stepper_motor();
	configure_adc();
	configure_usart();
	configure_usart_callbacks();
	system_interrupt_enable_global();
    #if LOGS != 0 || TESTMODE != 0
    rprintf_init();
    #endif
	
	sSystemState = start;
	
	plc_com_arm_receiver();
	
	while (1) {

    #if TESTMODE == 1
    rprintf("Voltage in uVolt: %d\r\n", force_sense_get_uV());
    delay_ms(1000);
    #endif

    #if TESTMODE == 0

		switch(sSystemState) {
			case(start):
			case(idle):
			case(busy):
			    break;
			case(init):
			    z_axis_home();
				break;
			case(pick):
			    z_axis_pick_sample();
				break;
			case(place):
			    z_axis_place_sample();
				break;
			case(change_tool):
			    z_axis_change_tool();
				break;
			case(stamp):
			    z_axis_stamp();
				break;
			case(soak):
			    z_axis_soak_stamp();
				break;
			case(close_lid):
			    z_axis_close_lid();
				break;
			case(get_force):
				plc_com_transmit_force(
				    force_sense_get_millinewton()
				);
				break;
			case(music):
			    stepper_music_play(notes1, 15);
				break;
			case(success):
			    plc_com_success();
				break;
		}
    #endif

    #if TESTMODE == 2
        test_loop();
    #endif
	}
}

void test_loop(){

        delay_ms(1000);
        
        if(sSystemState == init) {
            rprintf("INIT");
        }
        switch(sSystemState) {
            case(start):
            case(idle):
            case(busy):
                rprintf("IDLE\r\n");
                break;
            case(init):
                rprintf("INIT called\r\n");
                delay_ms(2000);
                set_state(success);
                break;
            case(pick):
                rprintf("PICK called\r\n");
                delay_ms(2000);
                set_state(success);
                break;
            case(place):
                rprintf("PLACE called\r\n");
                delay_ms(2000);
                set_state(success);
                break;
            case(change_tool):
                rprintf("TOOL called\r\n");
                delay_ms(2000);
                set_state(success);
                break;
            case(stamp):
                rprintf("STAMP called\r\n");
                delay_ms(2000);
                set_state(success);
                break;
            case(soak):
                rprintf("INK called\r\n");
                delay_ms(2000);
                set_state(success);
                break;
            case(close_lid):
                rprintf("CLOSE LID called\r\n");
                delay_ms(2000);
                set_state(success);
                break;
            case(get_force):
                rprintf("FORCE called\r\n");
                plc_com_transmit_force(
                force_sense_get_millinewton()
                );
                break;
            case(music):
                stepper_music_play(notes1, 15);
                break;
            case(success):
                plc_com_success();
            break;
        }
}
