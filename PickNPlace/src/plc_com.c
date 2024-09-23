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
/* T --> Tool, followed by specifier         RX Only                    */
/* F --> Force, init force transmition       RX Only                    */
/*                                                                      */
/* Move specifiers: Mx                                                  */
/* u --> pick up                                                        */
/* d --> place                                                          */
/* c --> close lid                                                      */
/* s --> stamp                                                          */
/* i --> ink stamp                                                      */
/*                                                                      */
/* Tool specifiers: Tx                                                  */
/* 1 --> pick n place tool                                              */
/* 2 --> stamp tool                                                     */
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
/*                                                                      */
/************************************************************************/

#include "plc_com.h"
#include "rprintf.h"

int plc_com_plc_to_state() {
	/* Converts the PLC commands to predefined states. If the command is unknown a unknown message is send to the PLC. */
}

int plc_com_state_to_plc() {
	/* Converts returned state from the MCU to status telegram and sends it to PLC with plc_com_send_cmd. Returns 1 on success and 0 on failure.*/
}

int plc_com_send_cmd() {
	/* Pure USART send command. Takes a string massage as input and sends it to the PLC.
	 * Returns 1 on success and 0 on error.*/
}

void plc_com_receive_callback() {
	
	rprintf("Callback fired!");
}

