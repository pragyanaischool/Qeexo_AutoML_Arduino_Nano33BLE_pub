#include <DMA.h>

int dma_channel;
volatile int dma_done;

void dma_complete(int channel) {
    dma_done = 1;
}

void setup() {
    DMA.begin();
    dma_channel = DMA.allocateChannel();
    DMA.setTriggerSource(dma_channel, 0); /* software trigger */
    DMA.setTransferWidth(dma_channel, 32);
    DMA.incSrc(dma_channel);
    DMA.incDst(dma_channel);
    DMA.onTransferComplete(dma_channel, dma_complete);
}

void loop() {
    uint32_t src[17];
    uint32_t dst[17];
    for(int i=0; i<16; i++) {
        src[i] = (i + 'A') * 0x01010101;
        dst[i] = 0xcccccccc;
    }
    src[16] = dst[16] = 0;
    dma_done = 0;
    DMA.transfer(dma_channel, src, dst, 64);
    while(!dma_done)
        ;
    Serial.println((char *)dst);
    delay(1000);
}
