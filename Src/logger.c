#include "logger.h"
#include "usart.h"

USART_TypeDef* logger_instance;

struct usart_clock_config logger_clock_config = {
		.clock_source = RCC_PCLK,
		.prescaler = PRESCALE_BY_1,
};

struct usart_config logger_kernel_config = {
		.baud_rate = USART_BAUD_RATE_9600,
		.instance = USART_INSTANCE_3,
		.is_fifo_en = 1,
		.is_parity_enable = 0,
		.parity = USART_EVEN_PARITY,
		.stop_bits = USART_STOP_BITS_1,
		.word_length = USART_DATA_BITS_8,
		.noise_config = USART_NOISE_DETECTION_ENABLE,
};


void logger_init(void) {
	logger_instance = usart_get_instance_handle(logger_kernel_config.instance);
	usart_init(logger_kernel_config.instance, &logger_clock_config);
	usart_config(&logger_kernel_config, &logger_clock_config, logger_instance);
}

int _write(int file, char *ptr, int len) {
	(void)file;
    usart_transmit_data_non_blocking((uint8_t*)ptr, len, logger_instance, DMA_STREAM0);
    return len;
}

// approach 2 is to use the weak attribute version of write
//int __io_putchar(int ch) {
//	uint8_t byte = (uint8_t)ch;
//	usart_transmit_data_non_blocking(&byte, 1, logger_instance, DMA_STREAM0);
//	return ch;
//}
