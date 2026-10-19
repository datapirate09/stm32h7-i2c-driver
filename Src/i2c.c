#include "i2c.h"
#include <stddef.h>

static void i2c_configure_timing_parameters(I2C_TypeDef* instance, struct i2c_config *config);
static void i2c_leader_configure(I2C_TypeDef* instance, struct i2c_config *config);
static void i2c_follower_configure(I2C_TypeDef* instance, struct i2c_config *config);

//prescaler, tcll, tclh, scldel, sdadel
uint8_t i2c_timing_parameters[3][5] = {{15, 19, 16, 3, 2}, {3, 21, 9, 3, 2}, {0, 33, 20, 7, 4}};
static i2c_callback application_callback = NULL;

volatile uint8_t *tx_buf;
volatile uint8_t *rx_buf;
uint32_t transfer_size;
volatile uint32_t remaining_transfer_size;
volatile uint32_t index = 0;
uint8_t error_flag = 0;


I2C_TypeDef* get_instance_handle(i2c_instance_t instance) {
	switch(instance) {
	case I2C_INSTANCE_1:
		return I2C1;
	case I2C_INSTANCE_2:
		return I2C2;
	case I2C_INSTANCE_3:
		return I2C3;
	case I2C_INSTANCE_4:
		return I2C4;
	default:
		return I2C1;
	}
}

void i2c_init(struct i2c_clock_config *clock_config, i2c_instance_t instance) {
	if (instance == I2C_INSTANCE_1 || instance == I2C_INSTANCE_2 || instance == I2C_INSTANCE_3) {
		RCC->CDCCIP2R &= ~(3U << 12);
		RCC->CDCCIP2R |= ((uint32_t)clock_config->clk_src & 3U) << 12; // to ensure bits dont spill if user passes val greater than 4
		switch(instance) {
		case I2C_INSTANCE_1:
			RCC->APB1LENR |= (1U << 21);
			break;

		case I2C_INSTANCE_2:
			RCC->APB1LENR |= (1U << 22);
			break;

		case I2C_INSTANCE_3:
			RCC->APB1LENR |= (1U << 23);
			break;
		}
	}
	else if (instance == I2C_INSTANCE_4) {
		RCC->SRDCCIPR &= ~(3U << 8);
		RCC->SRDCCIPR |= (clock_config->clk_src << 8);
		RCC->APB4ENR |= (1U << 7);
	}
}

void i2c_configure(I2C_TypeDef* instance, struct i2c_config *config) {
	instance->CR1 &= ~(1U << 0);
	instance->CR1 = (instance->CR1 & ~(1U << 12)) | ((config->analog_filter & 1U) << 12);
	instance->CR1 = (instance->CR1 & ~(15U << 8)) | ((config->digital_filter & 15U) << 8);
	// now configure clock timing parameters
	i2c_configure_timing_parameters(instance, config);
//	instance->CR1 |= (1U << 17);
	instance->CR1 |= (1U << 0);
	if (config->mode == I2C_LEADER) i2c_leader_configure(instance, config);
	else if (config->mode == I2C_FOLLOWER) i2c_follower_configure(instance, config);

}

void i2c_register_callback(i2c_callback callback) {
	application_callback = callback;
}

void i2c_receive_data(uint8_t *rx_buffer, uint32_t buffer_size) {
	index = 0;
	rx_buf = rx_buffer;
	transfer_size = buffer_size;
}

void i2c_transmit_data(I2C_TypeDef* instance, uint8_t *tx_buffer, uint32_t buffer_size) {
	index = 0;
	tx_buf = tx_buffer;
	transfer_size = buffer_size;
	error_flag = 0;
	if (buffer_size > 255) {
		instance->CR2 |= (1U << 24);
		instance->CR2 |= (0xFFU << 16);
		remaining_transfer_size = transfer_size;
	}
	else {
		instance->CR2 &= ~(1U << 24);
		instance->CR2 |= (((uint8_t)buffer_size) << 16);
		instance->CR2 |= (1U << 25); // autoend enabeld to send stop bit after transfer
	}
	instance->CR2 |= (1U << 13);
}

static void i2c_leader_configure(I2C_TypeDef* instance, struct i2c_config *config) {
	instance->CR1 |= (1U << 6); // transfer complete enable
	instance->CR1 |= (1U << 4); // NACK interrupt enable
	instance->CR1 |= (1U << 1); // tx interrupt enable
	instance->CR1 |= (1U << 5); // stop bit interrupt enable
	instance->CR2 &= ~(1U << 11);
	uint32_t cr2 = 0;
	if (config->addr_mode == I2C_7_BIT_ADDRESS) {
		cr2 |= ((uint32_t)(config->address & 0x7FU) << 1);
	}
	else {
		cr2 |= ((uint32_t)(config->address & 0x3FFU));
		cr2 |= (1U << 11);
	}
	instance->CR2 = cr2;

}

static void i2c_follower_configure(I2C_TypeDef* instance, struct i2c_config *config) {
	instance->OAR1 &= ~(1U << 15);                 // OA1EN off
	instance->OAR1 &= ~(0x3FFU << 0);              // clear address field
	if (config->addr_mode == I2C_7_BIT_ADDRESS) {
		instance->OAR1 &= ~(1U << 10);             // OA1MODE = 7 bit
		instance->OAR1 |= ((config->address & 0x007FU) << 1);
	}
	else {
		instance->OAR1 |= (1U << 10);              // OA1MODE = 10 bit
		instance->OAR1 |= ((config->address & 0x03FFU) << 0);
	}
	instance->OAR1 |= (1U << 15);                  // OA1EN on

	instance->CR1 |= (1U << 2);
	instance->CR1 |= (1U << 3);
	instance->CR1 |= (1U << 5);
}

static void i2c_configure_timing_parameters(I2C_TypeDef* instance, struct i2c_config *config) {
	instance->TIMINGR &= ~(0xFU << 28);
	instance->TIMINGR |=  (i2c_timing_parameters[config->speed][0] << 28); // prescaler set
	instance->TIMINGR &= ~(0xFFU << 0);
	instance->TIMINGR |= (i2c_timing_parameters[config->speed][1] << 0);
	instance->TIMINGR &= ~(0xFFU << 8);
	instance->TIMINGR |= (i2c_timing_parameters[config->speed][2] << 8);
	instance->TIMINGR &= ~(0xFU << 20);
	instance->TIMINGR |= (i2c_timing_parameters[config->speed][3] << 20);
	instance->TIMINGR &= ~(0xFU << 16);
	instance->TIMINGR |= (i2c_timing_parameters[config->speed][4] << 16);
}

void I2C1_EV_IRQHandler(void) {
	if (I2C1->ISR & (1U << 2)) {
		// data is ready to be read so copy into buffer
		*(rx_buf + index) = I2C1->RXDR;
		if (index < transfer_size) {
			index++;
		}
	}
	if (I2C1->ISR & (1U << 3)) {
		index = 0;
		I2C1->ICR |= (1U << 3);
	}
	if (I2C1->ISR & (1U << 5)) {
		I2C1->ICR |= (1U << 5);
		application_callback();
	}
	if (I2C1->ISR & (1U << 1)) { // transmit code starts
		I2C1->TXDR = *(tx_buf + index);
		if (index < transfer_size) {
			index++;
		}
	}
	if (I2C1->ISR & (1U << 7)) { // tcr set so subtract nbytes
		remaining_transfer_size -= 255;
		uint32_t cr2 = I2C1->CR2;
		cr2 &= ~(0xFFU << 16);
		if (remaining_transfer_size > 255) {
			cr2 |= (0xFFU << 16);
		}
		else {
			cr2 &= ~(1U << 24);
			cr2 |= (1U << 25);
			cr2 |= (remaining_transfer_size << 16);
		}
		I2C1->CR2 = cr2;
	}
	if (I2C1->ISR  & (1U << 4)) {
		I2C1->ICR = (1U << 4);
		error_flag = 1;
	}
}
