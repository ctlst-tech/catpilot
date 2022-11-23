#include "ms5611.h"

#include "ms5611_reg.h"

// Private functions
void ms5611_thread(void *dev_ptr);
void ms5611_fsm(ms5611_t *dev);
static void ms5611_chip_selection(ms5611_t *dev);
static void ms5611_chip_deselection(ms5611_t *dev);
static void ms5611_exchange(ms5611_t *dev, uint8_t *tx_buf, uint8_t *rx_buf,
                            uint16_t length);
static void ms5611_reset(ms5611_t *dev);
uint16_t ms5611_read_prom(ms5611_t *dev, uint8_t word);
static int ms5611_read_calib(ms5611_t *dev);
static uint32_t ms5611_read_adc(ms5611_t *dev);
static int ms5611_read_meas(ms5611_t *dev);
static void ms5611_process_meas(ms5611_t *dev);

// Public functions
ms5611_t* ms5611_start(spi_t *spi, gpio_t *cs, uint32_t period,
                 uint32_t thread_priority) {
    if (spi == NULL || cs == NULL) {
        return NULL;
    }

    ms5611_t *dev = calloc(1, sizeof(ms5611_t));

    if (dev == NULL) {
        return NULL;
    }

    strcpy(dev->name, "MS5611");
    dev->interface.spi = spi;
    dev->interface.cs = cs;

    if (period < 2) {
        LOG_ERROR(dev->name, "Too high frequency");
        return NULL;
    }

    dev->os.period = period / portTICK_PERIOD_MS;
    dev->os.priority = thread_priority;

    dev->sync.measrdy_sem = xSemaphoreCreateBinary();
    if (dev->sync.measrdy_sem == NULL) {
        return NULL;
    }

    if (xTaskCreate(ms5611_thread, dev->name, 512, dev, dev->os.priority,
                    NULL) != pdTRUE) {
        LOG_ERROR(dev->name, "Thread start error");
        return NULL;
    }

    LOG_DEBUG(dev->name, "Start service, period = %u ms, priority = %u",
              dev->os.period, dev->os.priority);

    xSemaphoreTake(dev->sync.measrdy_sem, 0);

    return dev;
}

void ms5611_thread(void *dev_ptr) {
    ms5611_t *dev = dev_ptr;
    TickType_t last_wake_time;

    last_wake_time = xTaskGetTickCount();
    dev->state = MS5611_RESET;

    while (1) {
        ms5611_fsm(dev);
        xTaskDelayUntil(&last_wake_time, dev->os.period);
    }
}

void ms5611_fsm(ms5611_t *dev) {
    switch (dev->state) {
        case MS5611_RESET:
            vTaskDelay(5);
            ms5611_reset(dev);
            vTaskDelay(5);
            dev->state = MS5611_READ_CALIB;
            break;

        case MS5611_READ_CALIB:
            if (ms5611_read_calib(dev)) {
                LOG_ERROR(dev->name, "Failed to read calibration values");
                dev->attempt++;
            } else {
                vTaskDelay(10);
                ms5611_exchange(dev, (uint8_t *)&CMD_MS5611_CONVERT_D1_OSR1024,
                                NULL, 1);
                LOG_INFO(dev->name, "Initialization successful");
                dev->state = MS5611_READ_MEAS;
            }
            if (dev->attempt > 5) {
                LOG_ERROR(dev->name, "Fatal error");
                dev->state = MS5611_FAIL;
            }
            break;

        case MS5611_READ_MEAS:
            if (!ms5611_read_meas(dev)) {
                ms5611_process_meas(dev);
            }
            break;

        case MS5611_FAIL:
            vTaskDelete(NULL);
            break;
    }
}

void ms5611_stat(ms5611_t *dev) {
    if(dev == NULL || dev->state != MS5611_READ_MEAS) {
        return;
    }
    printf("\n");
    LOG_DEBUG(dev->name, "Statistics:");
    LOG_DEBUG(dev->name, "T = %.3f [C]", dev->meas.T);
    LOG_DEBUG(dev->name, "P = %.3f [Pa]", dev->meas.P);
}

// Private functions
static void ms5611_chip_selection(ms5611_t *dev) {
    spi_chip_select(dev->interface.spi, dev->interface.cs);
}

static void ms5611_chip_deselection(ms5611_t *dev) {
    spi_chip_deselect(dev->interface.spi, dev->interface.cs);
}

static void ms5611_exchange(ms5611_t *dev, uint8_t *tx_buf, uint8_t *rx_buf,
                            uint16_t length) {
    ms5611_chip_selection(dev);
    if (rx_buf == NULL) {
        spi_transmit(dev->interface.spi, tx_buf, length);
    } else {
        spi_transmit_receive(dev->interface.spi, tx_buf, rx_buf, length);
    }
    ms5611_chip_deselection(dev);
}

static void ms5611_reset(ms5611_t *dev) {
    uint8_t reg = CMD_MS5611_RESET;
    ms5611_exchange(dev, (uint8_t *)&CMD_MS5611_RESET, NULL, 1);
}

uint16_t ms5611_read_prom(ms5611_t *dev, uint8_t word) {
    uint8_t data[3];
    data[0] = CMD_MS5611_PROM_ADDR + (word << 1);
    ms5611_exchange(dev, data, data, sizeof(data));
    return (data[1] << 8 | data[2]);
}

static int ms5611_read_calib(ms5611_t *dev) {
    uint16_t prom[MS5611_PROM_SIZE] = {};

    for (int i = 0; i < MS5611_PROM_SIZE; i++) {
        prom[i] = ms5611_read_prom(dev, i);
        if (prom[i] == 0 && i > 1) {
            return -1;
        }
    }

    uint16_t crc_get = prom[7] & 0xF;
    prom[7] &= 0xFF00;
    uint16_t crc_calc = crc4(prom);
    if (crc_get != crc_calc) {
        return -1;
    }

    dev->calib.C1 = prom[1];
    dev->calib.C2 = prom[2];
    dev->calib.C3 = prom[3];
    dev->calib.C4 = prom[4];
    dev->calib.C5 = prom[5];
    dev->calib.C6 = prom[6];

    return 0;
}

static uint32_t ms5611_read_adc(ms5611_t *dev) {
    uint8_t data[4] = {};
    data[0] = CMD_MS5611_READ_ADC;
    ms5611_exchange(dev, data, data, sizeof(data));
    uint32_t reg = (data[1] << 16) | (data[2] << 8) | data[3];
    return reg;
}

// Read 4 samples for pressure and 1 sample for temperature
static int ms5611_read_meas(ms5611_t *dev) {
    uint8_t cmd;
    static int ignore_next = 0;
    static int meas_previous_state = MS5611_READ_TEMP;
    static int meas_current_state = MS5611_READ_TEMP;

    // Read ADC after previous command
    uint32_t reg = ms5611_read_adc(dev);

    if (reg != 0 && ignore_next == 0) {
        meas_current_state++;
    }

    if (meas_current_state > MS5611_READ_PRES_4) {
        meas_current_state = MS5611_READ_TEMP;
    }

    switch (meas_current_state) {
        case (MS5611_READ_TEMP):
            cmd = CMD_MS5611_CONVERT_D1_OSR1024;
            break;
        case (MS5611_READ_PRES_1):
        case (MS5611_READ_PRES_2):
        case (MS5611_READ_PRES_3):
        case (MS5611_READ_PRES_4):
            cmd = CMD_MS5611_CONVERT_D2_OSR1024;
            break;
    }

    ms5611_exchange(dev, &cmd, NULL, 1);

    if (reg == 0 || reg == 0xFFFFFF) {
        ignore_next = 1;
        return -1;
    }

    if (ignore_next) {
        ignore_next = 0;
        return -1;
    }

    switch (meas_previous_state) {
        case (MS5611_READ_TEMP):
            dev->raw.D2 = reg;
            break;
        case (MS5611_READ_PRES_1):
        case (MS5611_READ_PRES_2):
        case (MS5611_READ_PRES_3):
        case (MS5611_READ_PRES_4):
            dev->raw.D1 = reg;
            break;
    }

    meas_previous_state = meas_current_state;

    return 0;
}

static void ms5611_process_meas(ms5611_t *dev) {
    int32_t dT = 0, TEMP = 0, T2 = 0, P = 0;
    int64_t OFF = 0, SENS = 0, OFF2 = 0, SENS2 = 0;

    // Temperature
    dT = dev->raw.D2 - (uint32_t)(dev->calib.C5) * (1 << 8);
    TEMP = 2000 + dT * dev->calib.C6 / (1 << 23);

    // Pressure
    OFF = (uint32_t)dev->calib.C2 * (1 << 16) +
          (uint32_t)(dev->calib.C4 * dT) / (1 << 7);

    SENS = (uint32_t)dev->calib.C1 * (1 << 15) +
           (uint32_t)(dev->calib.C3 * dT) / (1 << 8);

    // Low temperature compensation
    if (TEMP < 2000) {
        T2 = (dT * dT) / (1 << 31);
        OFF2 = 5 * (TEMP - 2000) * (TEMP - 2000) / 2;
        SENS2 = 5 * (TEMP - 2000) * (TEMP - 2000) / 4;
        if (TEMP < -1500) {
            OFF2 = OFF2 + 7 * (TEMP + 1500) * (TEMP + 1500);
            SENS2 = SENS2 + 11 * (TEMP + 1500) * (TEMP + 1500) / 2;
        }
    }

    TEMP = TEMP - T2;
    OFF = OFF - OFF2;
    SENS = SENS - SENS2;

    P = (dev->raw.D1 * SENS / (1 << 21) - OFF) / (1 << 15);

    dev->meas.T = TEMP / 100.f;
    dev->meas.P = P;
}
