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
#include "main.h"
#include "rprintf.h"

const char EOT = 0x03;

enum states {
	s_acknowledge   = 'a',
	s_busy          = 'b',
	s_unknown       = 'u',
	s_success       = 's',
	s_failed        = 'f'
	};
	
enum commands {
	c_init = 'I',
	c_move = 'M',
	c_tool = 'T',
	c_force = 'F'
	};

static uint16_t rx_buffer;

static void plc_com_send_cmd(bool, enum states, uint16_t);
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
		plc_com_send_cmd(false, s_acknowledge, 0);
	} else {
		plc_com_send_cmd(false, s_failed, 0);
	}
}
    

static void plc_com_send_cmd(bool force_tx, enum states state, uint16_t force) {
	
	uint8_t tx_buffer[4];
	uint16_t len;
	
	if(force_tx) {
		len = 4;
		tx_buffer[0] = 'F';
		tx_buffer[1] = (force >> 8);
		tx_buffer[2] = (force & 0xFF);
		tx_buffer[3] = EOT;
	} else {
		len = 3;
		tx_buffer[0] = 'S';
		tx_buffer[1] = state;
		tx_buffer[2] = EOT;
	}
		
	usart_write_buffer_wait(&usart_instance, tx_buffer, len);
}

	/*
	 * Returns success message to PLC.
	 */
void plc_com_success() {
	
	plc_com_send_cmd(false, s_success, 0);
	
}

    /*
	 * Returns the force value to the PLC.
	 */
void plc_com_transmit_force(int16_t force) {
	plc_com_send_cmd(true, s_success, force);
}

void plc_com_arm_receiver() {
	
	usart_read_job(&usart_instance, &rx_buffer);
}

void plc_com_receive_callback() {
	
	static uint8_t symbol_counter = 0;
	static enum commands command;
	static uint8_t specifier;
	static bool await_eot = false;
	
	if(get_state() != idle || get_state() != start) {
		plc_com_send_cmd(false, s_busy, 0);
		plc_com_arm_receiver();
		return;
	}
	
	if(symbol_counter == 0) {
		command = 0;
		specifier = 0;
	}
	
	if(((rx_buffer == EOT) != await_eot) || symbol_counter > 2 ) {
		plc_com_send_cmd(false, s_unknown, 0);
		symbol_counter = 0;
		plc_com_arm_receiver();
		return;
	}
	
	if(rx_buffer == EOT) {
		set_state(busy);
		plc_com_plc_to_state(command, specifier);
		symbol_counter = 0;
		await_eot = false;
		plc_com_arm_receiver();
		return;
	}
	
	if(symbol_counter == 0) {
	    switch(rx_buffer) {
		    case('I'):
			    command = c_init;
				await_eot = true;
				break;
			case('M'):
			    command = c_move;
				await_eot = false;
				break;
			case('T'):
			    command = c_tool;
				await_eot = true;
				break;
			case('F'):
			    command = c_force;
				await_eot = true;
				break;
			default:
			    plc_com_send_cmd(false, s_unknown, 0);
				symbol_counter = 0;
				plc_com_arm_receiver();
				return;
	    }
		symbol_counter++;
		plc_com_arm_receiver();
		return;
	}
	
	if(symbol_counter == 1){
		switch(rx_buffer) {
			case('u'):
			case('d'):
			case('c'):
			case('s'):
			case('i'):
			    specifier = rx_buffer;
			    break;
			default:
				plc_com_send_cmd(false, s_unknown, 0);
				symbol_counter = 0;
				plc_com_arm_receiver();
				return;
		}
		symbol_counter++;
		await_eot = true;
		return;
	}
}

