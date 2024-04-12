#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#ifndef BUFFERS_H
#define BUFFERS_H

#define BLOCK_WHEN_FULL 1

typedef struct {
    // circular buffer
    void *array;         // pointer to the array that will be used as buffer
    uint8_t elementSize; // how many bytes for each element
    uint8_t arraySize;   // how many elements in the array
    uint8_t head;  // write index
    uint8_t tail;  // read  index
    bool isEmpty;  // there is nothing inside
    bool isFull;  // there is no space left if you write before reading it sets the dataLoss flag
    bool dataLoss;   // it went to overflow at some point (lost data)
    bool Blocked; // Temporary tail. Used to block the buffer while handling data in other programs.
    uint8_t bookmarkIdx; // Temporary tail. Used to block the buffer while handling data in other programs.
    uint8_t whatIsLife;
} Buffer;

Buffer initBuffer(void *array, uint8_t elementSize, uint8_t arraySize);
void enq(void *data, volatile Buffer *buffer);
void deq(void *data, volatile Buffer *buffer);
void setBookmark(volatile Buffer *buffer);
void removeBookmark(volatile Buffer *buffer);
bool findNextBookmark(volatile Buffer *buffer);
void jumpToBookmark(volatile Buffer *buffer);
bool findFlag(volatile Buffer *buffer, void *data);
void nEnq(void *data, volatile Buffer *buffer, uint8_t size);
void nDeq(void *data, volatile Buffer *buffer, uint8_t size);
void reset(volatile Buffer *buffer);
void rollback(volatile Buffer *buffer, uint8_t size);
uint8_t howMuchData(volatile Buffer *buffer);

#endif