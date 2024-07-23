#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#ifndef BUFFERS_H
#define BUFFERS_H

#define BLOCK_WHEN_FULL 0

typedef struct {
    uint8_t start;
    uint8_t end;
    uint8_t restriction; // 0:no restriction, 1:write only, 2:blocked no read no write.
} Range;


typedef struct {
    // circular buffer
    uint8_t *array;         // pointer to the array that will be used as buffer
    uint8_t arraySize;   // how many elements in the array
    uint8_t head;  // write index
    uint8_t tail;  // read  index
    bool isEmpty;  // there is nothing inside
    bool isFull;  // there is no space left if you write before reading it sets the dataLoss flag
    bool dataLoss;   // it went to overflow at some point (lost data)
    bool Blocked; // Temporary tail.
    
    uint8_t msgStartIdx; // Temporary tail.
    Range msgRanges[4]; 
    uint8_t msgCount; 
} Buffer;



Buffer initBuffer(void *array, uint8_t arraySize);
// basic buffer operations
void enq(void *data, volatile Buffer *buffer); // add data
void deq(void *data, volatile Buffer *buffer); // read data
void getMsg(volatile Buffer *buffer, uint8_t* msgOut, uint8_t* msgSize); // gets the oldest message found in buffer

void nEnq(void *data, volatile Buffer *buffer, uint8_t size); // add n data
void nDeq(void *data, volatile Buffer *buffer, uint8_t size); // read n data
void reset(volatile Buffer *buffer); // reset buffer
void rollback(volatile Buffer *buffer, uint8_t size); // rollback buffer
uint8_t howMuchData(volatile Buffer *buffer);

// message handling!
void setMsgStart(volatile Buffer *buffer);  // mark message start
void removeMsgStart(volatile Buffer *buffer); // remove message start mark
bool findNextMsgStart(volatile Buffer *buffer);// find next message start
void jumpToMsgStart(volatile Buffer *buffer);   // jump to message start
void delRange(volatile Buffer *buffer, uint8_t delStart, uint8_t delEnd, bool safe);

bool findFlag(volatile Buffer *buffer, void *data); // find a flag in buffer
void deqMsg(volatile Buffer *buffer); // unblocks from start to end
void enqMsg(volatile Buffer *buffer); // blocks from bookmark to current tail
#endif