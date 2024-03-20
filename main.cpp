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

int mode = -1;

void writeMode(int _mode, const char* buf, unsigned count)
{
    if (mode != _mode) {
#ifdef BCM2835
        bcm2835_gpio_write(PIN_DATA_CONTROL, _mode);
#else
        printf("set mode %d\n", _mode);
#endif
        mode = _mode;
    }
#ifdef BCM2835
    bcm2835_spi_transfern((char*)buf, count);
#else
    printf("write data: ");
    for (int i = 0; i < count; i++) {
        printf("%02x ", buf[i]);
    }
    printf("\n");
#endif
}

void writeCmd(char cmd)
{
    writeMode(SPI_Command_Mode, &cmd, 1);
}

void writeData(const char* data, unsigned count)
{
    writeMode(SPI_Data_Mode, data, count);
}

void writeData(char data)
{
    writeData(&data, 1);
}

void sleep(int ms)
{
#ifdef BCM2835
    bcm2835_delay(ms);
#else
    usleep(ms * 1000);
#endif
}

void writeAddr(int addr1, int addr2)
{
    char data[4];
    data[0] = (addr1 >> 8) & 0xFF;
    data[1] = addr1 & 0xFF;
    data[2] = (addr2 >> 8) & 0xFF;
    data[3] = addr2 & 0xFF;
    writeData(data, 4);
}

// void drawFilledRect(int spi, int x, int y, int w, int h, int color)
// {
//     writeCmd(0x2A); // set column address
//     writeAddr(x, x + w - 1);
//     writeCmd(0x2B); // set row address
//     writeAddr(y, y + h - 1);

// }

void drawPixel(int x, int y, uint16_t color)
{
    writeCmd(0x2A); // set column address
    writeAddr(x, x);
    writeCmd(0x2B); // set row address
    writeAddr(y, y);

    writeCmd(0x2C); // memory Write

    char data[2];
    data[0] = (color >> 8) & 0xFF;
    data[1] = color & 0xFF;
    writeData(data, 2);
}

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

    if (!bcm2835_spi_begin()) {
        fprintf(stderr, "Failed to initialise BCM2835 spi interface.\n");
        return 1;
    }

    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);      // The default
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);                   // The default
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_65536); // The default
    bcm2835_spi_chipSelect(BCM2835_SPI_CS0);                      // The default
    bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);      // the default

#endif

    writeCmd(0x01); // reset
    sleep(150); // sleep 150ms
    writeCmd(0x11); // sleep out
    sleep(255); // sleep 255ms
    writeCmd(0x3A); // set pixel format
    // writeCmd(0x55); // 16bit
    writeCmd(0x05); // 16bpp
    sleep(10); // sleep 10ms

    writeCmd(0x36); // set memory address
    sleep(10); // sleep 10ms

    writeCmd(0x2A); // set column address
    // char columnData[4] = { 0x00, 0x00, 0x00, 128 }; // 240 ???
    char columnData[4] = { 0x00, 0x00, 0x00, (char)240 };
    // char columnData[4] = { 0x00, 0x00, 240 >> 8, 240 & 0xFF };
    writeData(columnData, 4);

    writeCmd(0x2B); // set row address
    // char rowData[4] = { 0x00, 0x00, 0x00, 128 }; // 240 ???
    char rowData[4] = { 0x00, 0x00, 0x00, (char)240 };
    // char rowData[4] = { 0x00, 0x00, 240 >> 8, 240 & 0xFF };
    writeData(rowData, 4);

    writeCmd(0x21); // Display Inversion On
    writeCmd(0x13); // Normal Display Mode On
    writeCmd(0x29); // display on
    sleep(255); // sleep 255ms

#ifdef BCM2835
    bcm2835_gpio_write(PIN_BACKLIGHT, 1);
#endif

    sleep(150); // sleep 150ms

    // draw some random pixel
    for (int i = 0; i < 100; i++) {
        // int x = rand() % 120;
        // int y = rand() % 120;
        // uint16_t color = rand() % 0xFFFFFF;
        // drawPixel(x, y, color);

        drawPixel(i, i, 0xFF00);
    }

#ifdef BCM2835
    bcm2835_spi_end();
    bcm2835_close();
#endif
    return 0;
}