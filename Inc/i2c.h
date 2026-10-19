#include <stdint.h>
#include "stm32h7a3xxq.h"

typedef enum {
	RCC_PCLK = 0,
	PLL_RCLK = 1,
	HSI_KER_CK = 2,
	CSI_KER_CK = 3,
} i2c_ker_clock_source_t;

typedef enum {
	I2C_INSTANCE_1 = 0,
	I2C_INSTANCE_2 = 1,
	I2C_INSTANCE_3 = 2,
	I2C_INSTANCE_4 = 3,
} i2c_instance_t;


typedef enum {
	ANALOG_FILTER_OFF = 1,
	ANALOG_FILTER_ON = 0,
} analog_filter_t;

typedef enum {
	DIGITAL_FILTER_OFF = 0,
	DIGITAL_FILTER_ON = 1,
} digital_filter_t;

typedef enum {
	I2C_STANDARD_MODE = 0,
	I2C_FAST_MODE = 1,
	I2C_FAST_PLUS_MODE = 2,
} i2c_speed_t;

typedef enum {
	I2C_LEADER = 0,
	I2C_FOLLOWER = 1,
} i2c_mode_t;

typedef enum {
	I2C_7_BIT_ADDRESS = 0,
	I2C_10_BIT_ADDRESS = 1,
} i2c_address_mode_t;


struct i2c_clock_config {
	i2c_ker_clock_source_t clk_src;
};

struct i2c_config {
	i2c_speed_t speed;
	analog_filter_t analog_filter;
	digital_filter_t digital_filter;
	i2c_mode_t mode;
	i2c_address_mode_t addr_mode;
	uint16_t address;
};

typedef void (*i2c_callback)(void);

I2C_TypeDef* get_instance_handle(i2c_instance_t instance);
void i2c_init(struct i2c_clock_config *clock_config, i2c_instance_t instance);
void i2c_configure(I2C_TypeDef* instance, struct i2c_config *config);
void i2c_register_callback(i2c_callback callback);
void i2c_receive_data(uint8_t *rx_buffer, uint32_t buffer_size);
void i2c_transmit_data(I2C_TypeDef* instance, uint8_t *tx_buffer, uint32_t buffer_size);

