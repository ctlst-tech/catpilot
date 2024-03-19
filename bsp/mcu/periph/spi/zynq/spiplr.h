#ifndef SPI_POLLER_H
#define SPI_POLLER_H

#include <stdint.h>

#define SPIPLR_BASE_ADDR                    0x43C80000  // TODO TBD
#define SPIPLR_IRQ                          68          // TODO TBD

#define SPIPLR_CNT_OFF                      0x0000      /// Control register offset
#define SPIPLR_CNT_ENABLE                   (1 << 0)    /// Start and continue polling of current command set
#define SPIPLR_CNT_DISABLE                  (1 << 1)    /// Stop polling
#define SPIPLR_CNT_RESET                    (1 << 2)    /// Reset SPIPLR peripheral
#define SPIPLR_CNT_CR_START                 (1 << 3)    /// custom transaction start

#define SPIPLR_POLL_PERIOD_OFF              0x0004      /// Poll period register offset (reset threshold for irq_counter clocked by main clock)

#define SPIPLR_STAT_OFF                     0x0008      /// Status register offset
#define SPIPLR_STAT_POLL_EN                 (1 << 0)    /// Polling loop is enabled
#define SPIPLR_STAT_POLL_RUN                (1 << 1)    /// Polling cycle is running right now
#define SPIPLR_STAT_POLL_RDY                (1 << 2)    /// Poll data is ready and new poll cycle is not started yet
#define SPIPLR_STAT_CR_RDY                  (1 << 3)    /// Custom request result is ready
#define SPIPLR_STAT_POLL2BIG                (1 << 4)    /// Polling cycle is too long for current period

#define SPIPLR_IRQ_OFF                      0x000C      /// IRQ control register
#define SPIPLR_IRQ_POLL_RDY_EN              (1 << 0)    /// Enable IRQ on poll data ready
#define SPIPLR_IRQ_POLL_RDY_DIS             (1 << 1)    /// Disable IRQ on poll data ready
#define SPIPLR_IRQ_CR_RDY_EN                (1 << 2)    /// Enable IRQ on custom request ready
#define SPIPLR_IRQ_CR_RDY_DIS               (1 << 3)    /// Disable IRQ on custom request ready

#define SPIPLR_IRQ_COUNTER_OFF              0x0010      /// IRQ irq_counter register
#define SPIPLR_IRQ_PROC_TIME_OFF            0x0014      /// IRQ processing time register
#define SPIPLR_POLL_CYCLE_TIME_OFF          0x0018      /// Polling cycle time

#define SPIPLR_CFG_REQ_VECTOR_OFF           0x0080
#define SPIPLR_CFG_VECTOR_SIZE 				32

#define SPIPLR_CR_REQ_OFF                   0x0500      /// Custom SPI transaction request register. Transfer is triggered upon register write (16 registers)
#define SPIPLR_CR_RESP_OFF                  0x0900      /// Custom SPI transaction reply register (16 registers)

#define SPIPLR_DATA_MASK                    0xFFFFFF
#define SPIPLR_DEVN_MASK                    ((1 << 6) - 1)

/// Macros for constructing SPI command, either for Custom Request or for poll vector
#define SPIPLR_COMPOSE_REQ(__id, __req, __cons)             \
            ( (((__cons) & 0x01) << 30) | (((__id) & SPIPLR_DEVN_MASK) << 24) | ((__req) & SPIPLR_DATA_MASK))

#define SPIPLR_COMPOSE_REQ_TERMINATION()    SPIPLR_COMPOSE_REQ(0x00, 0x00, 0)

#define SPIPLR_RESP_EXTRACT_DEV_ID(__reg)   (((__reg) >> 24 ) & SPIPLR_DEVN_MASK)

#define SPIPLR_POLL_VECTOR_SIZE              256
#define SPIPLR_POLL_REQ_VECTOR_OFF           0x0100      /// Request vector start

#define SPIPLR_POLL_RESP_VECTOR_OFF          0x0900      /// Response vector start

#define SPIPLR_REG_MAP_SIZE                 (SPIPLR_POLL_RESP_VECTOR_OFF + ((SPIPLR_POLL_VECTOR_SIZE) << 2))

#define SPIPLR_CR_MAX_TRANS					16

/**
 * Request register (custom request or request vector)
 * 31   30   29         24          16          8           0
 * res cons  | device # | -------data to request------------|
 *
 * res - reserved
 * cons - CS of the devices is not go hi after transaction, consequent requests this way
 *
 * === custom request example ===
 *
 * #define DEVICE_ID    0
 * write_reg(SPIPLR_CR_REQ_OFF, SPIPLR_REQ(DEVICE_ID, 0x1234))
 * ... wait SPIPLR_STAT_CR_RDY
 * uint32_t result = read_reg(SPIPLR_CR_RESP_OFF) & SPIPLR_RESP_DATA_MASK
 *
 *
 * === req vector programming example ===
 * int i = 0;
 * write_reg(SPIPLR_POLL_REQ_VECTOR_START_OFF + (i++ << 2), SPIPLR_REQ(DEVICE_ID_1, 0x123456))
 * write_reg(SPIPLR_POLL_REQ_VECTOR_START_OFF + (i++ << 2), SPIPLR_REQ(DEVICE_ID_2, 0x123456))
 * write_reg(SPIPLR_POLL_REQ_VECTOR_START_OFF + (i++ << 2), SPIPLR_REQ(DEVICE_ID_3, 0x123456))
 */

typedef struct {
    uintptr_t base;
    uintptr_t phys_base;
} spiplr_instance_t;

typedef struct __attribute__((packed)) {
    unsigned int sclk_divider : 8;
    unsigned int tr_byte_num : 2;
    unsigned int cs_vector : 8;
    unsigned int lsb_first : 1;
    unsigned int clk_cpha : 1;
    unsigned int clk_cpol : 1;
    unsigned int clk_delay : 5;
    unsigned int reserved : 6;
} spiplr_cfg_word_t;

int spiplr_init(spiplr_instance_t *i, uintptr_t phys_base);
int spiplr_poll_set_period(spiplr_instance_t *i, uint32_t period_us, uint32_t mck);
int spiplr_poll_start(spiplr_instance_t *i);
int spiplr_poll_stop(spiplr_instance_t *i);
int spiplr_reset(spiplr_instance_t *i);
int spiplr_get_status(spiplr_instance_t *i, uint32_t *status);
int spiplr_reset_status(spiplr_instance_t *i);
int spiplr_req_vector_set(spiplr_instance_t *i, uint32_t *v, int size);
int spiplr_resp_vector_get(spiplr_instance_t *i, uint32_t *v, int *size);
int spiplr_cfg_vector_set(spiplr_instance_t *i, uint32_t *v, int size);
int spiplr_irq_poll_rdy_enable(spiplr_instance_t *i);
int spiplr_irq_poll_rdy_disable(spiplr_instance_t *i);
void spiplr_print_req_vector(uint32_t *v, int size);
int spiplr_dump_reg_map(spiplr_instance_t *i);
int spi_plr_custom_trans (spiplr_instance_t *i, uint32_t *inb, int n, uint32_t *outb );
uint32_t spi_plr_get_irq_counter(spiplr_instance_t *i);
uint32_t spi_plr_get_irq_proc_time(spiplr_instance_t *i);
uint32_t spi_plr_get_poll_cycle_time(spiplr_instance_t *i);

#endif //SPI_POLLER_H
