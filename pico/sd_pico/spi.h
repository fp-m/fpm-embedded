/* spi.h
Copyright 2021 Carl John Kugler III

Licensed under the Apache License, Version 2.0 (the License); you may not use
this file except in compliance with the License. You may obtain a copy of the
License at

   http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an AS IS BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied. See the License for the
specific language governing permissions and limitations under the License.
*/

#pragma once

#include <fpm/api.h>
#include <stdbool.h>

// Pico includes
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/spi.h"
#include "pico/mutex.h"
#include "pico/sem.h"
#include "pico/types.h"

#define SPI_FILL_CHAR (0xFF)

// "Class" representing SPIs
typedef struct _spi_t {
    // SPI HW
    spi_inst_t *hw_inst;
    uint miso_gpio; // SPI MISO GPIO number (not pin number)
    uint mosi_gpio;
    uint sck_gpio;
    uint baud_rate;

    // State variables:
    uint tx_dma;
    uint rx_dma;
    dma_channel_config tx_dma_cfg;
    dma_channel_config rx_dma_cfg;
    irq_handler_t dma_isr;
    bool initialized;
    semaphore_t sem;
    mutex_t mutex;
} spi_t;

#ifdef __cplusplus
extern "C" {
#endif

// SPI DMA interrupts
void __not_in_flash_func(spi_irq_handler)(spi_t *pSPI);

bool __not_in_flash_func(spi_transfer)(spi_t *pSPI, const uint8_t *tx, uint8_t *rx, size_t length);
void spi_lock(spi_t *pSPI);
void spi_unlock(spi_t *pSPI);
void spi_init_port(spi_t *pSPI);
void spi_deinit_port(spi_t *pSPI);
void set_spi_dma_irq_channel(bool useChannel1, bool shared);

#if 1
// No debug output
#define DBG_PRINTF(fmt, args...)
#else
// Enable debug output
#define DBG_PRINTF fpm_printf
#endif

//#define myASSERT(__e) ((__e) ? (void)0 : my_assert_func(__FILE__, __LINE__, __func__, #__e))
#define myASSERT(__e)

#ifdef __cplusplus
}
#endif

// Use LED at pin 25 to show SPI activity.
//#define SPI_LED_PIN 25

/* [] END OF FILE */
