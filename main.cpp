#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>

#ifdef BCM2835
// sudo apt-get install libbcm2835-dev
#include <bcm2835.h>
#endif

#define PIN_RESET 5
#define PIN_BACKLIGHT 13
#define PIN_DATA_CONTROL 6

#define SPI_Command_Mode 0
#define SPI_Data_Mode 1

int main(int argc, char** argv)
{
#ifdef BCM2835
    if (!bcm2835_init()) {
        fprintf(stderr, "Failed to initialise BCM2835 rpi io interface.\n");
        return 1;
    }

    bcm2835_gpio_fsel(PIN_RESET, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_write(PIN_RESET, 1);
    bcm2835_delay(50); // sleep 50ms
    bcm2835_gpio_write(PIN_RESET, 0);
    bcm2835_delay(50); // sleep 50ms
    bcm2835_gpio_write(PIN_RESET, 1);
    bcm2835_delay(50); // sleep 50ms

    bcm2835_gpio_fsel(PIN_BACKLIGHT, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_write(PIN_BACKLIGHT, 0);
    bcm2835_gpio_fsel(PIN_DATA_CONTROL, BCM2835_GPIO_FSEL_OUTP);

    bcm2835_spi_begin();
#endif

#ifdef BCM2835
    bcm2835_spi_end();
#endif
    return 0;
}