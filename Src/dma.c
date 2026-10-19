#include "dma.h"
#include <stddef.h>


struct dma_handle *get_dma_stream_handler(dma_stream stream)
{
    static struct dma_handle handle;
    switch (stream)
    {
        case DMA_STREAM0:
            handle.dma_stream = DMA1_Stream0;
            handle.dma_mux = DMAMUX1_Channel0;
            break;

        case DMA_STREAM1:
            handle.dma_stream = DMA1_Stream1;
            handle.dma_mux = DMAMUX1_Channel1;
            break;

        case DMA_STREAM2:
            handle.dma_stream = DMA1_Stream2;
            handle.dma_mux = DMAMUX1_Channel2;
            break;

        default:
            break;
    }
    return &handle;
}


void config_dma(struct dma_handle *handle, struct dma_config *config) {
	handle->dma_stream->CR &= ~(1U << 0);
	while(handle->dma_stream->CR & (1U << 0));
	DMA1->LIFCR = 0x3DU;
	handle->dma_stream->PAR = (uint32_t)config->peri_addr;
	handle->dma_stream->M0AR = (uint32_t)config->mem_addr;
	handle->dma_stream->NDTR = config->buffer_size;
	handle->dma_mux->CCR &= ~(0xFFU);
	handle->dma_mux->CCR |= (uint32_t)config->dma_mux_req;

	handle->dma_stream->CR = (handle->dma_stream->CR & ~(3U << 16)) | ((uint32_t)config->priority << 16);
	handle->dma_stream->FCR |= (1U << 2); //fifo enabled
	handle->dma_stream->FCR = (handle->dma_stream->FCR & ~(3U << 0)) | ((uint32_t)config->threshold<< 0);

	handle->dma_stream->CR = (handle->dma_stream->CR & ~(3U << 6)) | ((uint32_t)config->direction << 6);
	handle->dma_stream->CR |= (1U << 10); // inc mem mode
	handle->dma_stream->CR |= (1U << 0);
//	handle->dma_stream->CR |= (1U << 4); // dma transfer complete interrupt enabled.
//	NVIC_EnableIRQ(DMA1_Stream0_IRQn); // nvic side interrupt enabled.
}
