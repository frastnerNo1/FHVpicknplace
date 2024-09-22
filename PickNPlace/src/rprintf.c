/******************************************************************************
* This file is for rprintf()
* The rprintf() is a simple printf() and only can print string with %s,%d,%c,%x.
* It is based on a function supplied by Cypress Semiconductor 
* (now part of Infineon Technologies), see:
* https://community.infineon.com/t5/Knowledge-Base-Articles/Alternate-printf-Function-for-PSoC-4-KBA87093/ta-p/248174
*******************************************************************************/
#include <asf.h>
#include "rprintf.h"

struct usart_module my_usart_instance;

static void iputc(uint8_t ch)
{
	/*This function has to be replaced by user*/	
	usart_serial_putchar(&my_usart_instance, ch);
}

static uint8_t* change(uint32_t Index)
{
    return (uint8_t*)("0123456789abcdef"+Index);
}

void rprintf_init() {
    struct usart_config my_config_usart;

    usart_get_config_defaults(&my_config_usart);

    my_config_usart.baudrate    = 9600;
	my_config_usart.mux_setting = EDBG_CDC_SERCOM_MUX_SETTING;
    my_config_usart.pinmux_pad0 = EDBG_CDC_SERCOM_PINMUX_PAD0;
    my_config_usart.pinmux_pad1 = EDBG_CDC_SERCOM_PINMUX_PAD1;
    my_config_usart.pinmux_pad2 = EDBG_CDC_SERCOM_PINMUX_PAD2;
    my_config_usart.pinmux_pad3 = EDBG_CDC_SERCOM_PINMUX_PAD3;

    usart_serial_init(&my_usart_instance, EDBG_CDC_MODULE,&my_config_usart);
    usart_enable(&my_usart_instance);
}

void rprintf(const uint8_t *pszFmt,...)
{
    uint8_t *pszVal;
    uint32_t iVal, xVal, i = 0, buffer[12], index = 1;
    uint8_t cVal;
    uint32_t *pArg;
    pArg =(uint32_t *)&pszFmt;

    while(*pszFmt)
    {
        if('%' != *pszFmt)
        {
            iputc(*pszFmt);
            pszFmt++;
            continue;
        }
        pszFmt++;

        if(*pszFmt == 's')
        {
            pszVal = (uint8_t*)pArg[index++];
            for(; *pszVal != '\0'; pszVal++)
                iputc(*pszVal);
            pszFmt++;
            continue;
        }
        if(*pszFmt == 'd')
        {
            iVal = pArg[index++];
            i = 0;
            do{
                buffer[i++] = iVal % 10;
                iVal /= 10;
            }while(iVal);
            while(i > 0)
            {
                i--;
                iputc(*change(buffer[i]));
            }
            pszFmt++;
            continue;
        }
        if(*pszFmt == 'c')
        {
            cVal = (uint8_t)pArg[index++];
            iputc(cVal);
            pszFmt++;
            continue;
        }
        if(*pszFmt == 'x')
        {
            xVal = pArg[index++];
            i = 0;
            do{
                buffer[i++] = xVal % 16;
                xVal /= 16;
            }while(xVal);
            if(i%2!=0)
                buffer[i++]=0;
            if(i<2)
                buffer[i++]=0;

            while(i > 0)
            {
                i--;
                iputc(*change(buffer[i]));
            }
            pszFmt++;
            continue;
        }
        if(pszFmt == '\0')
        {
            break;
        }

    }
}