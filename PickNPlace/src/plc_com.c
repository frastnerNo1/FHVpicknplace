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
/* f --> failde, command could not be fullfiled                         */
/*                                                                      */
/* ETX (03h) --> End of transmition                                     */
/************************************************************************/

#include "plc_com.h"

const char EOT = 0x03;

enum states {
	s_acknowledge   = 'a',
	s_busy          = 'b',
	s_unknown       = 'u',
	s_success       = 's',
	s_failed        = 'f',
	s_error         = 'e'
	};
	
enum commands {
	c_init = 'I',
	c_move = 'M',
	c_tool = 'T',
	c_force = 'F'
	};

static uint16_t sRxBuffer;

static void plc_com_transmit_status(enum states, enum errorCodes);
static void plc_com_plc_to_state(enum commands, uint8_t);


    /* 
	 *Converts the PLC commands to predefined states.
	 */
static void plc_com_plc_to_state(enum commands command, uint8_t specifier) {
	
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
			}
	}
	
	if(acknowledge == EXIT_SUCCESS) {
		plc_com_transmit_status(s_acknowledge, 0);
	} else {
		plc_com_transmit_status(s_error, 1);
		set_state(idle);
	}
}
    

static void plc_com_transmit_status(enum states status, enum errorCodes code) {

    uint8_t txBuffer[4];
	
	txBuffer[0] = 'S';
	txBuffer[1] = status;
	(status == s_error) ? (txBuffer[2] = code) : (txBuffer[2] = EOT);
	txBuffer[3] = EOT;
		
	usart_write_buffer_wait(&gUsartInstance, txBuffer, 3);
}

	/*
	 * Returns success message to PLC.
	 */
void plc_com_success() {
	
	plc_com_transmit_status(s_success, 0);
	set_state(idle);
	
}

	/*
	 * Returns error message to PLC.
	 */
void plc_com_error(enum errorCodes code) {
	
	plc_com_transmit_status(s_error, code);
	set_state(idle);
}

    /*
	 * Returns the force value to the PLC.
	 */
void plc_com_transmit_force(int16_t force) {
	
	uint8_t txBuffer[4];
	
	txBuffer[0] = 'F';
	txBuffer[1] = (force >> 8);
	txBuffer[2] = (force & 0xFF);
	txBuffer[3] = EOT;
	
	usart_write_buffer_wait(&gUsartInstance, txBuffer, 4);
}

void plc_com_arm_receiver() {
	
	usart_read_job(&gUsartInstance, &sRxBuffer);
}

void plc_com_receive_callback() {
	
	static uint8_t sSymbolCounter = 0;
	static enum commands command;
	static uint8_t specifier;
	static bool awaitEot = false;
	
	if(get_state() != idle && get_state() != start) {
		plc_com_transmit_status(s_busy, 0);
		plc_com_arm_receiver();
		return;
	}
	
	if(sSymbolCounter == 0) {
		command = 0;
		specifier = 0;
	}
	
	if(((sRxBuffer == EOT) != awaitEot) || sSymbolCounter > 2 ) {
		plc_com_transmit_status(s_unknown, 0);
		sSymbolCounter = 0;
		plc_com_arm_receiver();
		return;
	}
	
	if(sRxBuffer == EOT) {
		set_state(busy);
		plc_com_plc_to_state(command, specifier);
		sSymbolCounter = 0;
		awaitEot = false;
		plc_com_arm_receiver();
		return;
	}
	
	if(sSymbolCounter == 0) {
	    switch(sRxBuffer) {
		    case('I'):
			    command = c_init;
				awaitEot = true;
				break;
			case('M'):
			    command = c_move;
				awaitEot = false;
				break;
			case('T'):
			    command = c_tool;
				awaitEot = true;
				break;
			case('F'):
			    command = c_force;
				awaitEot = true;
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
				plc_com_transmit_status(s_unknown, 0);
				sSymbolCounter = 0;
				plc_com_arm_receiver();
				return;
		}
		sSymbolCounter++;
		awaitEot = true;
		plc_com_arm_receiver();
		return;
	}
}

