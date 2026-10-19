#include <stdint.h>
#include "stm32h7a3xxq.h"

/* file only provides support for DMA1 controller */

typedef enum {
	DMA_STREAM0 = 0,
	DMA_STREAM1 = 1,
	DMA_STREAM2 = 2,
} dma_stream;

typedef enum {
	PRIORITY_LOW = 0,
	PRIORITY_MEDIUM = 1,
	PRIORITY_HIGH = 2,
	PRIORITY_VERY_HIGH = 3,
} dma_stream_priority;

typedef enum {
	FIFO_QUARTER_FULL = 0,
	FIFO_HALF_FULL = 1,
	FIFO_THREE_QUARTERS_FULL = 2,
	FIFO_FULL = 3,
} fifo_threshold;

typedef enum {
	PERIPHERAL_TO_MEMORY = 0,
	MEMORY_TO_PERIPHERAL = 1,
	MEMORY_TO_MEMORY = 2,
} transfer_direction;

typedef enum {
	DMA_MUX_REQ_USART1_TX = 46,
	DMA_MUX_REQ_USART1_RX = 47,
} dma_mux_req_in;

struct dma_handle{
	DMA_Stream_TypeDef* dma_stream;
	DMAMUX_Channel_TypeDef* dma_mux;
};

struct dma_config {
	uint32_t *peri_addr;
	uint32_t *mem_addr;
	uint32_t buffer_size;
	dma_stream_priority priority;
	fifo_threshold threshold;
	transfer_direction direction;
	dma_mux_req_in dma_mux_req;
};

struct dma_handle *get_dma_stream_handler(dma_stream stream);
void config_dma(struct dma_handle *handle, struct dma_config *config);
