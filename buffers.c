#include "buffers.h"

volatile Buffer initBuffer(void *data, uint8_t elementSize, uint8_t arraySize) {
    return (volatile Buffer) {
        .array = data,
        .elementSize = elementSize,
        .arraySize = arraySize,
        .head = 0,
        .tail = 0,
        .isEmpty = true,
        .isFull = false,
        .whatIsLife = 42,
    };
}

void enq(void *data, volatile Buffer *buffer) {
    buffer->isFull |= ((buffer->tail == buffer->head) && !buffer->isEmpty);
    if(BLOCK_WHEN_FULL &&  buffer->isFull){
        // doesn't add anymore
        return;
    }else{
        memcpy((uint8_t *)buffer->array + buffer->elementSize * buffer->head, data, buffer->elementSize);
        buffer->head = (buffer->head + 1) % buffer->arraySize;
        buffer->isEmpty = false;
        buffer->isFull = buffer->head == buffer->tail;
        return ;
    }
}

void deq(void *data, volatile Buffer *buffer) {
    if (buffer->isEmpty){
        return;
    }
    
    memcpy(data, (uint8_t *)buffer->array + buffer->elementSize * buffer->tail, buffer->elementSize);
    buffer->tail = (buffer->tail + 1) % buffer->arraySize;
    buffer->isFull = false;
    buffer->isEmpty = buffer->head == buffer->tail;
    return;
}

void nEnq(void *data, volatile Buffer *buffer, uint8_t size) {
    //check if there is enough space in the buffer
    if(buffer->arraySize - buffer->head + buffer->tail < size){
        buffer->isFull = true;
        return;
    }else{
        for (uint8_t i = 0; i < size; i++) {
            enq((uint8_t *)data + buffer->elementSize * i, buffer);
        }
    }
}

void nDeq(void *data, volatile Buffer *buffer, uint8_t size) {

    if(buffer->arraySize - buffer->head + buffer->tail < size){
        size = buffer->arraySize - buffer->head + buffer->tail + 1;
    }

    for (uint8_t i = 0; i < size; i++) {
        deq((uint8_t *)data + buffer->elementSize * i, buffer);
    }
    
}

void reset(volatile Buffer *buffer) {
    buffer->head = 0;
    buffer->tail = 0;
    buffer->isEmpty = true;
    buffer->isFull = false;
}

uint8_t howMuchData(volatile Buffer *buffer) {
    if (buffer->isEmpty) {
        return 0;
    }
    if (buffer->head > buffer->tail) {
        return buffer->head - buffer->tail;
    }
    return buffer->arraySize - buffer->tail + buffer->head;
}

