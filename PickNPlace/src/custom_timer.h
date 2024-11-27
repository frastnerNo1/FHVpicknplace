/*
 * custom_timer.h
 *
 * Created: 27.11.2024 09:38:31
 *  Author: floro
 */ 

 #ifndef MAIN_H_
 #define MAIN_H_

 #define TC_BASE_REGISTER 0x42002000

 #define TC_CTRLA_REGISTER (*(volatile uint16_t *)(TC2_BASE + 0x00))
 #define TC_CTRLBCLEAR_REGISTER (*(volatile uint8_t *)(TC2_BASE + 0x04))
 #define TC_CTRLBSET_REGISTER (*(volatile uint8_t *)(TC2_BASE + 0x05))
 #define TC_DBGCTRL_REGISTER (*(volatile uint8_t *)(TC2_BASE + 0x08))
 #define TC_EVENTCTRL_REGISTER (*(volatile uint16_t *)(TC2_BASE + 0x0A))
 #define TC_INTENCLR_REGISTER (*(volatile uint8_t *)(TC2_BASE + 0x0C))
 #define TC_INTENSET_REGISTER (*(volatile uint8_t *)(TC2_BASE + 0x0D))
 #define TC_INTFLAG_REGISTER (*(volatile uint8_t *)(TC2_BASE + 0x0E))

 #endif
