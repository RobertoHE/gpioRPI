
#ifndef _GPIO_H_
#define _GPIO_H_


#if defined (GPIO_LIB_C)
#include "bcm2835.h"
#elif defined (GPIO_SYSTEM)
//#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#else
#error Choose between GPIO_LIB_C or GPIO_SYSTEM
#endif


#include "typgen.h"
#include "userlog.h"


#if !(defined(RPI3) || defined(CM3) || defined(CM3_PLUS))
#error No esta definida el tipo de placa
#endif

// Cfg GPIOs
#define GPIO_IN 0
#define GPIO_IN_PU 1
#define GPIO_IN_PD 2
#define GPIO_OUT 3
#define GPIO_OUT_0 3
#define GPIO_OUT_1 4

#ifndef HIGH
#define HIGH 0x1U
#endif

#ifndef LOW
#define LOW 0x0U
#endif

typedef enum gpioError{
	GPIO_ERROR=-1,
	GPIO_OK = 0
}gpioError_t;

typedef struct
{	
	BYTE port;
	BYTE dir;
}STR_GPIO_PORT;

//set up
int init_gpio();
void end_gpio();

//set gpio
int gpio_input(int port);
int gpio_input_pullup(int port);
int gpio_input_pulldown(int port);
int gpio_output(int port);
int gpio_set(int port, BYTE value); // see Cfg GPIOs defines

//release to tri-state
int gpio_unset(int port);

//Write
int gpio_output_value(int port, int value);

//Read
int gpio_read(int port);
int gpio_write(int port, BYTE value);

#endif // _GPIO_H_
