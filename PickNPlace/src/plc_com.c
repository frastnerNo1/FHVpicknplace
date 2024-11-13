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
/* F --> Force, init force transmition       RX Only                    */
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
/* f --> failed, command could not be fullfiled                         */
/* e1 --> error 1, system not initialized                               */
/* e2 --> error 2, wrong tool is active for this command                */
/*                                                                      */
/* X --> End of transmition                                             */
/************************************************************************/

#include "plc_com.h"

const char cTerminator = 'X';

typedef enum states {
	s_acknowledge   = 'a',
	s_busy          = 'b',
	s_unknown       = 'u',
	s_success       = 's',
	s_failed        = 'f',
	s_error         = 'e'
	} Plc_State_t;
	
typedef enum commands {
	c_init = 'I',
	c_move = 'M',
	c_tool = 'T',
	c_force = 'F'
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
	
	int acknowledge = 0;
	
	switch(command){
		case(c_init):
			acknowledge = set_state(init);
			break;
		case(c_force):
			acknowledge = set_state(get_force);
			break;
		case(c_tool):
		    acknowledge = set_state(change_tool);
			break;
		case(c_move):
		    switch(specifier) {
				case('u'):
				    acknowledge = set_state(pick);
				    break;
				case('d'):
				    acknowledge = set_state(place);
					break;
				case('c'):
				    acknowledge = set_state(close_lid);
					break;
				case('s'):
				    acknowledge = set_state(stamp);
				    break;
				case('i'):
				    acknowledge = set_state(soak);
					break;
				case('m'):
				    acknowledge = set_state(music);
					break;
			}
	}
	
	if(acknowledge == EXIT_SUCCESS) {
		plc_com_transmit_status(s_acknowledge, 0);
	} else {
		plc_com_transmit_status(s_error, e_not_init);
		set_state(idle);
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
		
	usart_write_buffer_wait(&gUsartInstance, txBuffer, len);
}

	/*
	 * @brief: send success state to PLC and set state machine to idle
	 */
void plc_com_success() {
	
	plc_com_transmit_status(s_success, 0);
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
void plc_com_transmit_force(int16_t force) {
	
	uint8_t txBuffer[8];

    plc_com_itoa(force, txBuffer);
	
	txBuffer[0] = 'F';
	txBuffer[7] = cTerminator;
	
	usart_write_buffer_wait(&gUsartInstance, txBuffer, 8);
	
	set_state(idle);
}

void plc_com_arm_receiver() {
	
	usart_read_job(&gUsartInstance, &sRxBuffer);
}

    /*
	 * @brief: callback function is called every time one character is received from the PLC
	 */
void plc_com_receive_callback() {
	
	static uint8_t sSymbolCounter = 0;
	static Plc_Command_t command;
	static uint8_t specifier;
	static bool awaitTerminator = false;
	
    /* Check if PLC is ready to take command */
	if(get_state() != idle && get_state() != start) {
		plc_com_transmit_status(s_busy, 0);
		plc_com_arm_receiver();
		return;
	}
	
    /*Reset the variables*/
	if(sSymbolCounter == 0) {
		command = 0;
		specifier = 0;
	}
	
    /*If terminator is not received or unexpected terminator is received
      or received symbols exceed the max. length an error is thrown. */
	if(((sRxBuffer == cTerminator) != awaitTerminator) || sSymbolCounter > 2 ) {
		plc_com_transmit_status(s_unknown, 0);
		sSymbolCounter = 0;
		plc_com_arm_receiver();
		return;
	}
	
    /*When terminator is received, the programm sets the new state.*/
	if(sRxBuffer == cTerminator) {
		set_state(busy);
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
			case('F'):
			    command = c_force;
				awaitTerminator = true;
				break;
			default:
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
			case('m'):
			    specifier = sRxBuffer;
			    break;
			default:
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
        buffer[1] = '-';
    } else {
        buffer[1] = '+';
    }

    buffer[2] = '0' + (number / 10000);
    number %= 10000;
    buffer[3] = '0' + (number / 1000);
    number %= 1000;
    buffer[4] = '0' + (number / 100);
    number %= 100;
    buffer[5] = '0' + (number / 10);
    number %= 10;
    buffer[6] = '0' + number;
}

