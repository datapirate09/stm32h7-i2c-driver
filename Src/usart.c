#include <usart.h>
#include <stddef.h>

static void usart_config_word_length(usart_word_length word_length, USART_TypeDef* instance);
static void usart_config_stop_bits(usart_stop_bits stop_bits, USART_TypeDef* instance);
static uint32_t get_clock_frequency(usart_clock_source clock_source,
                                    clock_prescaler prescaler);
static uint32_t get_pclk1_frequency(void);
static uint32_t get_hsi_ker_frequency(void);
static uint32_t get_csi_ker_frequency(void);
static uint32_t get_lse_frequency(void);
static void usart_config_baud_rate(usart_baud_rate baud_rate,struct usart_clock_config* clock_config, USART_TypeDef* instance);

uint8_t *tx_data = NULL;
uint32_t tx_buffer_size;

volatile uint32_t tx_index = 0;

static usart_callback application_callback = NULL;

USART_TypeDef* usart_get_instance_handle(usart_instance instance) {
	switch(instance) {
	case USART_INSTANCE_1:
		return USART1;
	case USART_INSTANCE_2:
		return USART2;
	case USART_INSTANCE_3:
		return USART3;
	default:
		return USART1;
	}
}

void usart_init(usart_instance usart_instance, struct usart_clock_config *clock_config) {
	switch(usart_instance) {
	case USART_INSTANCE_1:
		RCC->AHB4ENR |= (1U << 0);
		RCC->APB2ENR |= (1U << 4);
		GPIOA->MODER &= ~(1U << 18);
		GPIOA->MODER |= (1U << 19);
		GPIOA->MODER &= ~(1U << 20);
		GPIOA->MODER |= (1U << 21);
		GPIOA->AFR[1] |= (7U << 4);
		GPIOA->AFR[1] |= (7U << 8);
		RCC->CDCCIP2R &= ~(0b111 << 3);
		RCC->CDCCIP2R |= (clock_config->clock_source << 3);
		break;
	case USART_INSTANCE_3:
		// enable gpio alternate functions and apb clocks
		RCC->AHB4ENR |= (1U << 3);
		RCC->APB1LENR |= (1U << 18);
		GPIOD->MODER &= ~(1U << 16);
		GPIOD->MODER |= (1U << 17);
		GPIOD->MODER &= ~(1U << 18);
		GPIOD->MODER |= (1U << 19);
		GPIOD->AFR[1] &= ~(0xFU << 0); // set the mux to get uart alternate fn to the gpio
		GPIOD->AFR[1] |= (7U << 0);
		GPIOD->AFR[1] &= ~(0xF << 4); // set rx mux for alternate fn gpio
		GPIOD->AFR[1] |= (7U << 4);
		RCC->CDCCIP2R &= ~(0b111);
		RCC->CDCCIP2R |= (clock_config->clock_source);
		break;
	default:
		break;
	}
	RCC->AHB1ENR |= (1U << 0); // enable dma clock

}

static void usart_config_word_length(usart_word_length word_length, USART_TypeDef* instance) {
    switch (word_length)
    {
        case USART_DATA_BITS_8:
        	instance->CR1 &= ~(1U << 28);
        	instance->CR1 &= ~(1U << 12);
            break;

        case USART_DATA_BITS_9:
        	instance->CR1 &= ~(1U << 28);
			instance->CR1 |= (1U << 12);
            break;

        case USART_DATA_BITS_7:
        	instance->CR1 |= (1U << 28);
			instance->CR1 &= ~(1U << 12);
            break;

        default:
            break;
    }
}

static void usart_config_stop_bits(usart_stop_bits stop_bits, USART_TypeDef* instance) {
	switch(stop_bits) {
	case USART_STOP_BITS_1:
		instance->CR2 &= ~(1U << 13);
		instance->CR2 &= ~(1U << 12);
		break;
	case USART_STOP_BITS_0_5:
		instance->CR2 &= ~(1U << 13);
		instance->CR2 |= (1U << 12);
		break;
	case USART_STOP_BITS_2:
		instance->CR2 |= (1U << 13);
		instance->CR2 &= ~(1U << 12);
		break;
	case USART_STOP_BITS_1_5:
		instance->CR2 |= (1U << 13);
		instance->CR2 |= (1U << 12);
		break;
	default:
		break;
	}
}

static uint32_t get_clock_frequency(usart_clock_source clock_source,
                                    clock_prescaler prescaler) {
    static const uint32_t prescaler_values[] =
    {
        1,
        2,
        4,
        6,
        8,
        10,
        12,
        16,
        32,
        64,
        128,
        256
    };
    uint32_t clock_frequency = 0;
    switch (clock_source)
    {
        case RCC_PCLK:
            clock_frequency = get_pclk1_frequency();
            break;

        case PLL2_Q_CK:
//            clock_frequency = get_pll2_q_frequency();
            break;

        case PLL3_Q_CK:
//            clock_frequency = get_pll3_q_frequency();
            break;

        case HSI_KER_CK:
            clock_frequency = get_hsi_ker_frequency();
            break;

        case CSI_KER_CK:
            clock_frequency = get_csi_ker_frequency();
            break;

        case LSE_CK:
            clock_frequency = get_lse_frequency();
            break;

        default:
            return 0;
    }
    if (prescaler >= PRESCALE_LAST_VALUE)
        return 0;
    return clock_frequency / prescaler_values[prescaler];
}

static uint32_t get_pclk1_frequency(void) {
    return 64000000U;
}

static uint32_t get_hsi_ker_frequency(void) {
    return 64000000U;
}

static uint32_t get_csi_ker_frequency(void) {
    return 4000000U;
}

static uint32_t get_lse_frequency(void) {
    return 32768U;
}

static void usart_config_baud_rate(usart_baud_rate baud_rate,struct usart_clock_config* clock_config, USART_TypeDef* instance) {
	uint32_t scaled_value = get_clock_frequency(clock_config->clock_source, clock_config->prescaler);
	instance->BRR = (uint16_t)(scaled_value/baud_rate);
}

void usart_config(struct usart_config* config, struct usart_clock_config* clock_config, USART_TypeDef* instance) {
    usart_config_word_length(config->word_length, instance);
    instance->PRESC = clock_config->prescaler;
    if (config->is_fifo_en) instance->CR1 |= (1U << 29);
    else instance->CR1 &= ~(1U << 29);
    if (config->is_parity_enable) {
		instance->CR1 |= (1U << 10);
		if (config->parity == USART_EVEN_PARITY) instance->CR1 &= ~(1U << 9);
		else instance->CR1 |= (1U << 9);
    }
    instance->CR3 = ((instance->CR3 & ~(1U << 4)) | ((config->noise_config ? 1U: 0U) << 4));
    usart_config_baud_rate(config->baud_rate, clock_config, instance);
    usart_config_stop_bits(config->stop_bits, instance);
    instance->CR1 |= (1U << 0);
}

void usart_register_callback(usart_callback app_callback) {
	application_callback = app_callback;
}

void usart_transmit_data(uint8_t *usart_data_out, uint16_t buffer_size, USART_TypeDef* instance, usart_transfer_mode transfer_type) {
	instance->CR1 |= (1U << 3);
	if (transfer_type == USART_POLLING_MODE) { //polling
		for(uint16_t i=0;i<buffer_size;i++) {
			while(!(instance->ISR & 1U << 7)); // same bit used for fifo_en or not. if fifo_en bit states if fifo is empty and can be written. if not it is info about tx_rdr register if its moved its data to shift reg or not
			instance->TDR = *(usart_data_out+i);
		}
		while(!(instance->ISR & 1U << 6));
		instance->CR1 &= ~(1U << 3);
	}
	else if (transfer_type == USART_INTERRUPT_MODE) { //interrupt driven
		tx_data = usart_data_out;
		instance->CR1 |= (1U << 30);
		tx_buffer_size = buffer_size;
		tx_index = 0;
		if (instance == USART1) NVIC_EnableIRQ(USART1_IRQn);
		else if (instance == USART2) NVIC_EnableIRQ(USART2_IRQn);
		else if (instance == USART3) NVIC_EnableIRQ(USART3_IRQn);
	}
}


void usart_transmit_data_non_blocking(uint8_t *usart_data_out, uint16_t buffer_size, USART_TypeDef* instance, dma_stream stream) {
	struct dma_config config = {
			.buffer_size = buffer_size,
			.direction = MEMORY_TO_PERIPHERAL,
			.dma_mux_req = DMA_MUX_REQ_USART1_TX,
			.mem_addr = (uint32_t*)usart_data_out,
			.peri_addr = (uint32_t*)&instance->TDR,
			.priority = PRIORITY_VERY_HIGH,
			.threshold = FIFO_HALF_FULL,
	};
	struct dma_handle *handle = get_dma_stream_handler(stream);
	config_dma(handle, &config);
	instance->CR3 |= (1U << 7); // enable dma transmitter usart
	instance->CR1 |= (1U << 3);
}


usart_status usart_receive_data(uint8_t *usart_data_in, uint16_t buffer_size, USART_TypeDef* instance) {
	instance->CR1 |= (1U << 2);
	for(uint16_t i=0;i<buffer_size;i++) {
		while(!(instance->ISR & 1U << 5));
		if (instance->ISR & 1U << 3) {
			//overrun detected do something
			instance->ICR |= (1U << 3);
			instance->CR1 &= ~(1U << 2); // re disable
			return STATUS_USART_OVERRUN_ERROR;
		}
		*(usart_data_in+i) = instance->RDR;

		if (instance->ISR & (1U << 2)) {
			// noise detected in the corresponding byte
			instance->ICR |= (1U << 2);
			instance->CR1 &= ~(1U << 2); // re disable
			return STATUS_USART_NOISE_ERROR;
		}

		if (instance->ISR & (1U << 1)) {
			//framing error
			instance->ICR |= (1U << 1);
			instance->CR1 &= ~(1U << 2); // re disable
			return STATUS_USART_FRAMING_ERROR;
		}
	}
	instance->CR1 &= ~(1U << 2);
	return STATUS_OK;
}

void USART1_IRQHandler(void)
{
	if (USART1->ISR & (1U << 23) && USART1->CR1 & (1U << 30)) { // enter when u get interrupt from txfe and only if
		//txfeie is enabled. cause if txfeie is not enabled it means
		//the flag is set by hw when fifo is empty but last data trasnfer already happened
		while(tx_index < tx_buffer_size && (USART1->ISR & (1U << 7))) {
			USART1->TDR = *(tx_data + tx_index);
			tx_index++;
		}
		if (tx_index == tx_buffer_size) {
			USART1->CR1 &= ~(1U << 30);  // Disable TXFEIE
			USART1->ICR = (1U << 6);     // Clear TC flag
			USART1->CR1 |= (1U << 6);
		}
	}

	if (USART1->ISR & (1U << 6) && USART1->CR1 & (1U << 6)) {
		if (tx_index == tx_buffer_size) {
			USART1->CR1 &= ~(1U << 3);
			USART1->CR1 &= ~(1U << 6);
			USART1->CR1 &= ~(1U << 30);
			if (application_callback != NULL) application_callback();
		}
	}
}

void USART3_IRQHandler(void) {
	if (USART3->ISR & (1U << 23) && USART3->ISR & (1U << 6)) {
		USART3->CR1 &= ~(1U << 3);
		USART3->CR1 &= ~(1U << 6);
		USART3->CR1 &= ~(1U << 30);
	}
}
