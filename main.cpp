#ifdef PIGPIO
#include <pigpio.h>
#endif

#include <cstdio>
#include <unistd.h>
#include <cstdlib>

#define PIN_RESET 5
#define PIN_BACKLIGHT 13
#define PIN_DATA_CONTROL 6

#define SPI_Command_Mode 0
#define SPI_Data_Mode 1

int mode = 0;

// int spiWrite(unsigned handle, char *buf, unsigned count);

void writeMode(int spi, int _mode, const char* buf, unsigned count)
{
    if (mode != _mode) {
#ifdef PIGPIO
        gpioWrite(PIN_DATA_CONTROL, _mode);
#else
        printf("set mode %d\n", _mode);
#endif
        mode = _mode;
    }
#ifdef PIGPIO
    spiWrite(spi, (char*)buf, count);
#else
    printf("write data: ");
    for (int i = 0; i < count; i++) {
        printf("%02x ", buf[i]);
    }
    printf("\n");
#endif
}

void writeCmd(int spi, char cmd)
{
    writeMode(spi, SPI_Command_Mode, &cmd, 1);
}

void writeData(int spi, const char* data, unsigned count)
{
    writeMode(spi, SPI_Data_Mode, data, count);
}

void writeData(int spi, char data)
{
    writeData(spi, &data, 1);
}

void writeAddr(int spi, int addr1, int addr2)
{
    char data[4];
    data[0] = (addr1 >> 8) & 0xFF;
    data[1] = addr1 & 0xFF;
    data[2] = (addr2 >> 8) & 0xFF;
    data[3] = addr2 & 0xFF;
    writeData(spi, data, 4);
}

// void drawFilledRect(int spi, int x, int y, int w, int h, int color)
// {
//     writeCmd(spi, 0x2A); // set column address
//     writeAddr(spi, x, x + w - 1);
//     writeCmd(spi, 0x2B); // set row address
//     writeAddr(spi, y, y + h - 1);

// }

void drawPixel(int spi, int x, int y, int color)
{
    writeCmd(spi, 0x2A); // set column address
    writeAddr(spi, x, x);
    writeCmd(spi, 0x2B); // set row address
    writeAddr(spi, y, y);

    char data[2];
    data[0] = (color >> 8) & 0xFF;
    data[1] = color & 0xFF;
    writeData(spi, data, 2);
}

int main(int argc, char** argv)
{
#ifdef PIGPIO
    if (gpioInitialise() < 0) {
        fprintf(stderr, "Failed to initialise GPIO\n");
        return 1;
    }
#endif

    // initialize SPI
#ifdef PIGPIO
    int spi = spiOpen(0, 1000000, 0);
#else
    int spi = 0;
#endif

    if (spi < 0) {
        fprintf(stderr, "spiOpen failed\n");
        return 1;
    }

#ifdef PIGPIO
    gpioSetMode(PIN_RESET, PI_OUTPUT);
    gpioSetMode(PIN_BACKLIGHT, PI_OUTPUT);
    gpioWrite(PIN_BACKLIGHT, 0);
    gpioSetMode(PIN_DATA_CONTROL, PI_OUTPUT);
    gpioWrite(PIN_DATA_CONTROL, SPI_Command_Mode);
#endif

    writeCmd(spi, 0x01); // reset
    usleep(150000); // sleep 150ms
    writeCmd(spi, 0x11); // sleep out
    usleep(255000); // sleep 255ms
    writeCmd(spi, 0x3A); // set pixel format
    writeCmd(spi, 0x55); // 16bit

    writeCmd(spi, 0x2A); // set column address
    char columnData[4] = { 0x00, 0x00, 0x00, 0x0F };
    writeData(spi, columnData, 4);

    // setCommandMode();
    writeCmd(spi, 0x2B); // set row address
    char rowData[4] = { 0x00, 0x00, 0x00, 0x0F };
    writeData(spi, rowData, 4);

    writeCmd(spi, 0x21); // Display Inversion On
    writeCmd(spi, 0x13); // Normal Display Mode On
    writeCmd(spi, 0x29); // display on
    usleep(255000); // sleep 255ms

#ifdef PIGPIO
    gpioWrite(PIN_BACKLIGHT, 1);
#endif

    usleep(150000); // sleep 150ms

    // draw some random pixel
    for (int i = 0; i < 100; i++) {
        int x = rand() % 16;
        int y = rand() % 16;
        int color = rand() % 256;
        drawPixel(spi, x, y, color);
    }

#ifdef PIGPIO
    spiClose(spi);
#endif
    return 0;
}