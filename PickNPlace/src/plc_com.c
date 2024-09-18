/*
 * plc_com.c
 *
 * Created: 18.09.2024 09:34:19
 *  Author: floro
 */ 

/* In this file the communication via usart with the PLC is handled. Callback for incoming com is implemented in main.c*/

/* Todo: implement the following functions:
- convert telegram from PLC to state
- convert result to telegram for PLC
- write to PLC via usart
*/

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

int plc_com_receive_cmd() {
	/* Callback function for receiving messages from the PLC. Takes the message and converts it via plc_com_plc_to_state to predefined state which is then used to switch the state of the machine.
	 * Returns the state*/
}

