#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>

#if defined(GPIO_LIB_C)
#include "bcm2835.h"
#elif defined(GPIO_SYSTEM)
//
#endif

#include "typgen.h"
#include "gpio.h"


// Acceso al Driver GPIO
int fd_GPIO;

int errgpio;

char gpio_buf[256];

char _gpio_initialized = 0;



/******************************************************************************/


int init_gpio()
{
	if (_gpio_initialized)
		return GPIO_OK;
#if defined(GPIO_LIB_C)
	return !bcm2835_init();
#elif defined(GPIO_SYSTEM)
		//
#endif
	_gpio_initialized = 1;
	return GPIO_OK;
}

void end_gpio()
{	
#if defined(CM3) || defined(CM3_PLUS)
	//
#endif
	// Cierra descriptor
	_gpio_initialized = 0;
#if defined(GPIO_LIB_C)
	bcm2835_close();
#elif defined(GPIO_SYSTEM)
	
#endif
}

/******************************************************************************/
int gpio_write(int port, BYTE value)
/******************************************************************************/
/* Escribe 1/0 en un puerto de GPIO de salida								  */
{

#if (defined(RPI3) || defined(CM3) || defined(CM3_PLUS))
	if (port < 0 || port > 45)
		return GPIO_ERROR;
#endif

#if defined(GPIO_LIB_C)
	bcm2835_gpio_write(port, value);
#elif defined(GPIO_SYSTEM)
	sprintf(gpio_buf, "/sys/class/gpio/gpio%u/value", port);

	if ((fd_GPIO = open(gpio_buf, O_WRONLY)) == -1)
	{
		USERLOG(LOG_ERR, "init_gpio -> open %s error:%i[%s]\n", gpio_buf, errno, strerror(errno));
		return GPIO_ERROR;
	}

	write(fd_GPIO, value ? "1" : "0", 1);

	close(fd_GPIO);
#endif
	return GPIO_OK;
} /* gpio_write */

/******************************************************************************/
int gpio_read(int port)
/******************************************************************************/
/* Lee un puerto de GPIO de entrada											  */
{
	int value;
#if (defined(RPI3) || defined(CM3) || defined(CM3_PLUS))
	if (port < 0 || port > 45)
		return GPIO_ERROR;
#endif

#if defined(GPIO_LIB_C)

	value = bcm2835_gpio_lev(port);
#elif defined(GPIO_SYSTEM)

	// Valor
	sprintf(gpio_buf, "/sys/class/gpio/gpio%d/value", port);

	if ((fd_GPIO = open(gpio_buf, O_RDONLY)) == -1)
	{
		USERLOG(LOG_ERR, "gpio_read -> open %s error:%i[%s]\n", gpio_buf, errno, strerror(errno));
		return GPIO_ERROR;
	}

	if (read(fd_GPIO, &value, 1) > 0)
	{
		// Si hace mas lecturas consecutivas se debe volver al inicio del fichero
		// lseek(fd, 0, SEEK_SET);

		close(fd_GPIO);

		return (value == '1') ? 1 : 0;
	}

	USERLOG(LOG_ERR, "gpio_read -> read error:%i[%s]\n", errno, strerror(errno));

	return GPIO_ERROR;
#endif
	return value;
} /* gpio_read */

int gpio_set(int port, BYTE value)
{

#if (defined(RPI3) || defined(CM3) || defined(CM3_PLUS))
	if (port < 0 || port > 45)
		return GPIO_ERROR;
#endif
	_USERLOG(LOG_DEBUG, "gpio_set - %u \n", port);

#if defined(GPIO_LIB_C)
	// int pinVal = gpio2pin(port);

	if (value == GPIO_IN)
		bcm2835_gpio_fsel(port, BCM2835_GPIO_FSEL_INPT);
	if (value == GPIO_IN_PU)
		bcm2835_gpio_fsel(port, BCM2835_GPIO_PUD_UP);
	if (value == GPIO_IN_PD)
		bcm2835_gpio_fsel(port, BCM2835_GPIO_PUD_DOWN);
	if (value == GPIO_OUT_0) // or GPIO_OUT
		bcm2835_gpio_fsel(port, BCM2835_GPIO_FSEL_OUTP);
	if (value == GPIO_OUT_1)
		bcm2835_gpio_fsel(port, BCM2835_GPIO_FSEL_OUTP);

#elif defined(GPIO_SYSTEM)
	// Exporta puerto
	if ((fd_GPIO = open("/sys/class/gpio/export", O_WRONLY)) == -1)
	{
		USERLOG(LOG_ERR, "init_gpio -> open /sys/class/gpio/export error:%i[%s]\n", errno, strerror(errno));
		return GPIO_ERROR;
	}
	sprintf(gpio_buf, "%d", port);

	if (write(fd_GPIO, gpio_buf, strlen(gpio_buf)) != strlen(gpio_buf))
	{
		// EBUSY -> NO ERROR
		if (errno != EBUSY)
		{
			USERLOG(LOG_ERR, "init_gpio -> export %u port error:%i[%s]\n", port, errno, strerror(errno));

			close(fd_GPIO);
			return GPIO_ERROR;
		}

		USERLOG(LOG_DEBUG, "\tready\n");
	}

	close(fd_GPIO);
	USERLOG(LOG_DEBUG, "exported \n\r");

	// Direccion del puerto
	sprintf(gpio_buf, "/sys/class/gpio/gpio%u/direction", port);

	if ((fd_GPIO = open(gpio_buf, O_WRONLY)) == -1)
	{
		USERLOG(LOG_ERR, "gpio_input -> open %s error:%i[%s]\n", gpio_buf, errno, strerror(errno));

		close(fd_GPIO);

		return GPIO_ERROR;
	}

	if (value == GPIO_OUT || value == GPIO_OUT_0 || value == GPIO_OUT_1)
		sprintf(gpio_buf, "out");
	else
		sprintf(gpio_buf, "in");

	if (write(fd_GPIO, gpio_buf, strlen(gpio_buf)) != strlen(gpio_buf))
	{
		USERLOG(LOG_ERR, "gpio_input -> direction write [%s] port%u error:%i[%s]\n",
				gpio_buf, port, errno, strerror(errno));

		close(fd_GPIO);

		return GPIO_ERROR;
	}

	close(fd_GPIO);

	fd_GPIO = -1;

	if (value == GPIO_IN_PU)
	{
		sprintf(gpio_buf, "gpio -g mode %u up", port);

		system(gpio_buf);
	}
	if (value == GPIO_IN_PD)
	{
		sprintf(gpio_buf, "gpio -g mode %u down", port);

		system(gpio_buf);
	}

#endif

	if (value == GPIO_OUT_0)
		gpio_write(port, 0);
	if (value == GPIO_OUT_1)
		gpio_write(port, 1);

	return GPIO_OK;
}

/******************************************************************************/
int gpio_input(int port)
/******************************************************************************/
/* Configura un puerto de GPIO como entrada									  */
{
	return gpio_set(port, GPIO_IN);
} /* gpio_input */

/******************************************************************************/
int gpio_input_pullup(int port)
/******************************************************************************/
{
	return gpio_set(port, GPIO_IN_PU);
} /* gpio_input_pullup */

/******************************************************************************/
int gpio_input_pulldown(int port)
/******************************************************************************/
{
	return gpio_set(port, GPIO_IN_PD);
} /* gpio_input_pulldown */

/******************************************************************************/
int gpio_output(int port)
/******************************************************************************/
/* Configura un puerto de GPIO como salida									  */
{
	return gpio_set(port, GPIO_OUT);
} /* gpio_output */

/******************************************************************************/
int gpio_output_value(int port, int value)
/******************************************************************************/
/* Configura un puerto de GPIO como salida con un valor 					  */
{
	if (value == 0)
		return gpio_set(port, GPIO_OUT_0);
	else if (value == 1)
		return gpio_set(port, GPIO_OUT_1);
	else
		;
	return GPIO_ERROR;
} /* gpio_output_value */

//TODO hacer
int gpio_unset(int port){

	return GPIO_OK;
}