#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#ifndef BUFFERS_H
#define BUFFERS_H

#define BLOCK_WHEN_FULL 1

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
    bool Blocked; // Temporary tail. Used to block the buffer while handling data in other programs.
    
    uint8_t msgStartIdx; // Temporary tail. Used to block the buffer while handling data in other programs.
    Range msgRanges[3]; 
    uint8_t msgCount; 
} Buffer;



Buffer initBuffer(void *array, uint8_t arraySize);
// basic buffer operations
void enq(void *data, volatile Buffer *buffer); // add data
void deq(void *data, volatile Buffer *buffer); // read data
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

bool findFlag(volatile Buffer *buffer, void *data); // find a flag in buffer
void unmarkMsg(volatile Buffer *buffer); // unblocks from start to end
void markMsg(volatile Buffer *buffer); // blocks from bookmark to current tail
void getMsg(volatile Buffer *buffer, uint8_t* msgOut); // gets the oldest message found in buffer
#endif