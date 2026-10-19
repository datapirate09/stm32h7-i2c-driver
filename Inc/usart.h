#include <stdint.h>
#include "dma.h"
#include "stm32h7a3xxq.h"


typedef enum {
	USART_INSTANCE_1 = 0,
	USART_INSTANCE_2 = 1,
    USART_INSTANCE_3 = 2,
} usart_instance;

typedef enum {
	USART_DATA_BITS_8 = 0,
	USART_DATA_BITS_9 = 1,
	USART_DATA_BITS_7 = 2,
	USART_DATA_BITS_LAST = 3
} usart_word_length;

typedef enum {
	USART_EVEN_PARITY = 0,
	USART_ODD_PARITY = 1,
	USART_LAST_PARITY = 2
} usart_parity_bits;

typedef enum {
	USART_TRANSMIT = 0,
	USART_RECEIVE = 1,
	TRANSMISSION_COMPLETED = 2,
	USART_MODE_LAST = 3
} usart_mode;

typedef enum {
	USART_STOP_BITS_1 = 0,
	USART_STOP_BITS_0_5 = 1,
	USART_STOP_BITS_2 = 2,
	USART_STOP_BITS_1_5 = 3,
	USART_STOP_BITS_LAST = 4
} usart_stop_bits;

typedef enum {
	USART_BAUD_RATE_9600 = 9600,
	USART_BAUD_RATE_115200 = 115200,
	USART_BAUD_RATE_LAST = 2
} usart_baud_rate;

typedef enum {
	USART_NOISE_DETECTION_ENABLE = 0,
	USART_NOISE_DETECTION_DISABLE = 1,
	USART_NOISE_DETECTION_LAST
} usart_noise_config;

typedef enum {
	RCC_PCLK = 0,
	PLL2_Q_CK = 1,
	PLL3_Q_CK = 2,
	HSI_KER_CK = 3,
	CSI_KER_CK = 4,
	LSE_CK = 5,
	USART_CLOCK_SOURCE_LAST
} usart_clock_source;

typedef enum {
	PRESCALE_BY_1 = 0,
	PRESCALE_BY_2 = 1,
	PRESCALE_BY_4 = 2,
	PRESCALE_BY_6 = 3,
	PRESCALE_BY_8 = 4,
	PRESCALE_BY_10 = 5,
	PRESCALE_BY_12 = 6,
	PRESCALE_BY_16 = 7,
	PRESCALE_BY_32 = 8,
	PRESCALE_BY_64 = 9,
	PRESCALE_BY_128 = 10,
	PRESCALE_BY_256 = 11,
	PRESCALE_LAST_VALUE = 12
} clock_prescaler;

typedef enum {
	STATUS_OK,
	STATUS_USART_OVERRUN_ERROR,
	STATUS_USART_NOISE_ERROR,
	STATUS_USART_FRAMING_ERROR,
} usart_status;

typedef enum {
	USART_POLLING_MODE,
	USART_INTERRUPT_MODE,
	USART_DMA_MODE,
} usart_transfer_mode;

struct usart_config {
    usart_instance instance;
    int is_fifo_en;
    int is_parity_enable;
    usart_word_length word_length;
    usart_parity_bits parity;
    usart_stop_bits stop_bits;
    usart_baud_rate baud_rate;
    usart_noise_config noise_config;
};

struct usart_clock_config {
	usart_clock_source clock_source;
	clock_prescaler prescaler;
};

typedef void (*usart_callback)(void);

USART_TypeDef* usart_get_instance_handle(usart_instance instance);
void usart_init(usart_instance usart_instance, struct usart_clock_config *clock_config);
void usart_config(struct usart_config* config, struct usart_clock_config* clock_config, USART_TypeDef* instance);
void usart_register_callback(usart_callback app_callback);
void usart_transmit_data(uint8_t *usart_data_out, uint16_t buffer_size, USART_TypeDef* instance, usart_transfer_mode transfer_type);
void usart_transmit_data_non_blocking(uint8_t *usart_data_out, uint16_t buffer_size, USART_TypeDef* instance, dma_stream stream);
usart_status usart_receive_data(uint8_t *usart_data_in, uint16_t buffer_size, USART_TypeDef* instance);
