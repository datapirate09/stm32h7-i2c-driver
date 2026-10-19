#include <i2c.h>
#include "logger.h"
#include <stdio.h>

#define BUFFER_SIZE 1024

void i2c_leader_example_process_action(void);

struct i2c_clock_config clock_config = {
		.clk_src = RCC_PCLK,
};

struct i2c_config i2c_kernel_config = {
		.addr_mode = I2C_7_BIT_ADDRESS,
		.address = 64,
		.analog_filter = ANALOG_FILTER_ON,
		.digital_filter = DIGITAL_FILTER_OFF,
		.mode = I2C_LEADER,
		.speed = I2C_STANDARD_MODE,
};

I2C_TypeDef *instance_handle;

uint8_t transmit_buffer[BUFFER_SIZE];

void application_callback(void);

int main() {
	logger_init();
	printf("Starting I2C Leader Example Application\n");

	instance_handle = get_instance_handle(I2C_INSTANCE_1);
	i2c_init(&clock_config, I2C_INSTANCE_1);
	printf("I2C initialized successfully\n");
	i2c_configure(instance_handle, &i2c_kernel_config);
	printf("I2C configuration set successfully\n");
	i2c_register_callback(&application_callback);
	// initialization and configuration done
	while(1) {
		i2c_leader_example_process_action();
	}
}

void i2c_leader_example_process_action(void) {
	i2c_transmit_data(instance_handle, transmit_buffer, sizeof(transmit_buffer)/sizeof(uint8_t));
}

void application_callback(void) {
	printf("I2C Transfer Complete\n");
}

