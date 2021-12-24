#include "icm20602.h"

gpio_cfg_t icm20602_mosi = GPIO_SPI1_MOSI;
gpio_cfg_t icm20602_miso = GPIO_SPI1_MISO;
gpio_cfg_t icm20602_sck  = GPIO_SPI1_SCK;
gpio_cfg_t icm20602_cs   = GPIO_SPI1_CS2;
gpio_cfg_t icm20602_drdy = GPIO_SPI1_DRDY2;

icm20602_cfg_t icm20602_cfg = {
                         {SPI1, 
                          &icm20602_mosi, 
                          &icm20602_miso, 
                          &icm20602_sck,
                          &icm20602_cs,
                          20, 6, NULL},
                          0};

icm20602_cfg_t *cfg;
spi_cfg_t *spi;

// Others sensors on this SPI bus
gpio_cfg_t cs1 = GPIO_SPI1_CS1;
gpio_cfg_t cs3 = GPIO_SPI1_CS3;
gpio_cfg_t cs4 = GPIO_SPI1_CS4;

int ICM20602_Init() {
    int rv;

    // Disable others sensors
    GPIO_Init(&cs1);
    GPIO_Init(&cs3);
    GPIO_Init(&cs4);
    GPIO_Set(&cs1);
    GPIO_Set(&cs3);
    GPIO_Set(&cs4);

    cfg = &icm20602_cfg;
    spi = &cfg->spi;

    rv = SPI_Init(&cfg->spi);

    return rv;
}

uint8_t ICM20602_ReadReg(uint8_t reg) {
    uint8_t reg_ = reg | 0x80;

    // Chip selection
    GPIO_Reset(spi->cs_cfg);

    SPI_Transmit(spi, &reg_, 1);
    SPI_Receive(spi, &cfg->data, 1);

    GPIO_Set(spi->cs_cfg);

    return (cfg->data);
}
