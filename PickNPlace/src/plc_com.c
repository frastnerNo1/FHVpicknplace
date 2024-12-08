/*
 * plc_com.c
 *
 * Created: 18.09.2024 09:34:19
 *  Author: floro
 */ 

/* In this file the communication via usart with the PLC is handled. Callback registration for incoming com is implemented in main.c*/

/************************************************************************/
/* COMMANDTABLE:                                                        */
/* I --> System Init                         RX Only                    */
/* M --> Move, followed by specifier         RX Only                    */
/* T --> Toolchange                          RX Only                    */
/*                                                                      */
/* Move specifiers: Mx                                                  */
/* u --> pick up                                                        */
/* d --> place                                                          */
/* c --> close lid                                                      */
/* s --> stamp                                                          */
/* i --> ink stamp                                                      */
/*                                                                      */                
/*                                                                      */
/* f --> Force in mN, followed by a number of 5 digits     TX Only      */
/*                                                                      */
/* S --> State, followed by specifier                      TX Only      */
/*                                                                      */
/* State specifiers: Sx                                                 */
/* a --> acknowledge command                                            */
/* b --> busy, can't process command                                    */
/* u --> unknown, command is not known                                  */
/* s --> success, command successfully fullfiled                        */
/* i --> idle, MCU returned to idle state                               */
/* f --> failed, command could not be fullfiled                         */
/* e1 --> error 1, system not initialized                               */
/* e2 --> error 2, wrong tool is active for this command                */
/* e3 --> error 3, controller not ready to process command              */
/* e4 --> error 4, target force not reached                             */
/* e5 --> error 5, out of position
/*                                                                      */
/* X --> End of transmition                                             */
/************************************************************************/

#include "plc_com.h"
#include "force_sense.h"

#define SEND_DELAY_MS   100

const char cTerminator = 'X';
	
typedef enum commands {
	c_init = 'I',
	c_move = 'M',
	c_tool = 'T'
	} Plc_Command_t;

static uint16_t sRxBuffer;

static void plc_com_transmit_status(Plc_State_t, Error_Code_t);
static void plc_com_plc_to_state(Plc_Command_t, uint8_t);
static void plc_com_itoa(int16_t, uint8_t *);


    /* 
	 * @brief: Take the received command from UART and set the state of the state machine.
     * @param: command as Plc_Command_t
     * @param: specifier for move commands as uint8_t character
	 */
static void plc_com_plc_to_state(Plc_Command_t command, uint8_t specifier) {
	
	uint8_t statusCode = 0;
	
	switch(command){
		case(c_init):
			statusCode = set_state(init);
			break;
		case(c_tool):
		    statusCode = set_state(change_tool);
			break;
		case(c_move):
		    switch(specifier) {
				case('u'):
				    statusCode = set_state(pick);
				    break;
				case('d'):
				    statusCode = set_state(place);
					break;
				case('c'):
				    statusCode = set_state(close_lid);
					break;
				case('s'):
				    statusCode = set_state(stamp);
				    break;
				case('i'):
				    statusCode = set_state(soak);
					break;
			}
	}
	
	if(statusCode == 0) {
		plc_com_transmit_status(s_acknowledge, 0);
	} else if(statusCode == 1) {
		plc_com_error(e_not_init);
	} else {
        plc_com_error(e_not_ready);
    }
}
    
    /*
     * @brief: initiate UART transmission to PLC
     * @param: status to transmit as Plc_State_t
     * @param: error code as Error_Code_t, this is just evaluated if status = s_error
     */
static void plc_com_transmit_status(Plc_State_t status, Error_Code_t code) {

    uint8_t txBuffer[4];
    uint8_t len = 3;

    if(status == s_error) len = 4;
	
	txBuffer[0] = 'S';
	txBuffer[1] = status;
	(status == s_error) ? (txBuffer[2] = code) : (txBuffer[2] = cTerminator);
	txBuffer[3] = cTerminator;
	
    #if LOGS == 2
    rprintf("UART send to PLC: %s\r\n", txBuffer);
    #endif
	
    tc_stop_counter(&int_timer);    //Stop the force transmission interrupt
    port_pin_set_output_level(PLC_COM_CMD_PIN, true);   //Switch PLC to command mode

    delay_ms(SEND_DELAY_MS); // Delay to ensure PLC finished at least one cycle	
	usart_write_buffer_wait(&gUsartInstance, txBuffer, len);
    delay_ms(SEND_DELAY_MS); // Delay to ensure PLC finished at least one cycle

    port_pin_set_output_level(PLC_COM_CMD_PIN, false);  //Switch PLC to force mode
    tc_start_counter(&int_timer);   //Start force transmission
}

	/*
	 * @brief: send success state to PLC and set state machine to idle
	 */
void plc_com_success() {
	
	plc_com_transmit_status(s_success, 0);
    plc_com_transmit_status(s_idle, 0);
	set_state(idle);
	
}

	/*
	 * @brief: send error message to PLC and set state machine to idle
     * @param: error code as Error_Code_t
	 */
void plc_com_error(Error_Code_t code) {
	
	plc_com_transmit_status(s_error, code);
	set_state(idle);
}

    /*
	 * @brief: convert force value to ascii string an send it to the PLC via UART
     * @param: force as signed int16_t value
	 */
void plc_com_transmit_force(struct tc_module* const tc_instance) {
	
	uint8_t txBuffer[8];

    plc_com_itoa(force_sense_get_millinewton(), txBuffer);
	
	txBuffer[0] = 'S';
	txBuffer[7] = cTerminator;

    #if LOGS == 2
    rprintf("LOG: force send to PLC: %s\n\r", txBuffer);
    #endif
	
	usart_write_buffer_wait(&gUsartInstance, txBuffer, 8);
	
}

void plc_com_arm_receiver() {
	
    #if LOGS == 2
    rprintf("UART armed!");
    #endif

	usart_read_job(&gUsartInstance, &sRxBuffer);
}

    /*
	 * @brief: callback function is called every time one character is received from the PLC
	 */
void plc_com_receive_callback(struct usart_module* const usart_instance) {

    #if LOGS == 2
    rprintf("UART Callback fired! Symbol: %c\r\n", sRxBuffer);
    #endif
	
	static uint8_t sSymbolCounter = 0;
	static Plc_Command_t command;
	static uint8_t specifier;
	static bool awaitTerminator = false;
	
    /*Reset the variables if symbol counter is in reset state*/
    if(sSymbolCounter == 0) {
        command = 0;
        specifier = 0;
    }
	
    /*If terminator is not received or unexpected terminator is received
      or received symbols exceed the max. length an error is thrown. */
	if(((sRxBuffer == cTerminator) != awaitTerminator) || sSymbolCounter > 2 ) {
        
        #if LOGS == 2
        rprintf("Invalid command!\r\n");
        #endif

		plc_com_transmit_status(s_unknown, 0);
		sSymbolCounter = 0;
		plc_com_arm_receiver();
		return;
	}
	
    /*When terminator is received, the program sets the new state.*/
	if(sRxBuffer == cTerminator) {

        #if LOGS == 2
        rprintf("Valid command received. %c,%c\r\n", command, specifier);
        #endif

		plc_com_plc_to_state(command, specifier);
		sSymbolCounter = 0;
		awaitTerminator = false;
		plc_com_arm_receiver();
		return;
	}
	
    /*Check first symbol received. */
	if(sSymbolCounter == 0) {
	    switch(sRxBuffer) {
		    case('I'):
			    command = c_init;
				awaitTerminator = true;
				break;
			case('M'):
			    command = c_move;
				awaitTerminator = false;
				break;
			case('T'):
			    command = c_tool;
				awaitTerminator = true;
				break;
			default:
                #if LOGS == 2
                rprintf("Invalid command! \r\n");
                #endif
			    plc_com_transmit_status(s_unknown, 0);
				sSymbolCounter = 0;
				plc_com_arm_receiver();
				return;
	    }
		sSymbolCounter++;
		plc_com_arm_receiver();
		return;
	}
	
    /*Check second symbol received. */
	if(sSymbolCounter == 1){
		switch(sRxBuffer) {
			case('u'):
			case('d'):
			case('c'):
			case('s'):
			case('i'):
			    specifier = sRxBuffer;
			    break;
			default:
                #if LOGS == 2
                rprintf("Invalid command!\r\n");
                #endif
				plc_com_transmit_status(s_unknown, 0);
				sSymbolCounter = 0;
				plc_com_arm_receiver();
				return;
		}
		sSymbolCounter++;
		awaitTerminator = true;
		plc_com_arm_receiver();
		return;
	}
}


    /*
     * @brief: convert force from int to ascii, this includes the sign
     * @param: signed number to convert
     * @param: pointer to buffer for transmission
     */
static void plc_com_itoa(int16_t number, uint8_t * buffer) {
    
    if(number < 0) {
        *(buffer + 1) = '-';
    } else {
        *(buffer + 1) = '+';
    }

    number = abs(number);

    *(buffer + 2) = '0' + (number / 10000);
    number %= 10000;
    *(buffer + 3) = '0' + (number / 1000);
    number %= 1000;
    *(buffer + 4) = '0' + (number / 100);
    number %= 100;
    *(buffer + 5) = '0' + (number / 10);
    number %= 10;
    *(buffer + 6) = '0' + number;
}

