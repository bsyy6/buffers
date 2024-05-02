// fixed size -> uint8_t

#include "buffers.h"

volatile Buffer initBuffer(void *data, uint8_t arraySize) {
    return (volatile Buffer) {
        .array = data,
        .arraySize = arraySize,
        .head = 0,
        .tail = 0,
        .msgStartIdx = 0,
        .isEmpty = true,
        .isFull = false,
        .dataLoss = false,
        .Blocked = false,
    };
}

void enq(void *data, volatile Buffer *buffer) {
    buffer->dataLoss |= buffer->isFull;
    
    if(BLOCK_WHEN_FULL &&  buffer->dataLoss){
        // doesn't add anymore
        return;
    }else{
        
        // check that buffer head is not pointing to a blocked range
        for (uint8_t i = 0; i <= buffer->msgCount; i++) {
            if (buffer->head == buffer->msgRanges[i].start) {
                if(buffer->msgRanges[i].restriction > 1){
                    buffer->head = buffer->msgRanges[i].end + 1;
                }
                else
                {   
                    buffer->msgRanges[i].restriction = 0; // open for reading
                }
            }
        }

        memcpy((uint8_t *)buffer->array + buffer->head, data, 1);
        buffer->head = (buffer->head + 1) % buffer->arraySize;
        buffer->isEmpty = false;
        buffer->isFull = buffer->head == buffer->tail;
        if(buffer->Blocked){
            buffer->isFull = buffer->head == buffer->msgStartIdx;
        }
        return ;
    }
}

void deq(void *data, volatile Buffer *buffer) {
    if (buffer->isEmpty){
        return;
    }
    
    // check that buffer tail is not pointing to a blocked range
    for (uint8_t i = 0; i <= buffer->msgCount; i++) {
        if (buffer->tail == buffer->msgRanges[i].start && buffer->msgRanges[i].restriction == 2) {
            buffer->tail = buffer->msgRanges[i].end + 1;
        }
    }

    memcpy(data, (uint8_t *)buffer->array + buffer->tail, 1);
    buffer->tail = (buffer->tail + 1) % buffer->arraySize;
    buffer->isFull = false;
    buffer->isEmpty = buffer->head == buffer->tail;
    return;
}

void nEnq(void *data, volatile Buffer *buffer, uint8_t size) {
    //check if there is enough space in the buffer
    if(buffer->arraySize - buffer->head + buffer->tail < size){
        buffer->dataLoss = true;
        return;
    }else{
        for (uint8_t i = 0; i < size; i++) {
            enq((uint8_t *)data + i, buffer);
        }
    }
}

void nDeq(void *data, volatile Buffer *buffer, uint8_t size) {

    if(buffer->arraySize - buffer->head + buffer->tail < size){
        size = buffer->arraySize - buffer->head + buffer->tail + 1;
    }

    for (uint8_t i = 0; i < size; i++) {
        deq((uint8_t *)data + i, buffer);
    }
    
}

void reset(volatile Buffer *buffer) {
    buffer->head = 0;
    buffer->tail = 0;
    buffer->isEmpty = true;
    buffer->isFull = false;
    buffer->dataLoss = false;
    buffer->Blocked = false;
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

void setMsgStart(volatile Buffer *buffer){
    if(!buffer->Blocked){
        buffer->Blocked = true;
        buffer->msgStartIdx = (buffer->tail+ buffer->arraySize)% (buffer->arraySize+1);
    }
}

void removeMsgStart(volatile Buffer *buffer){
    buffer->Blocked = false;
    if(buffer->isFull && buffer->isEmpty){
        buffer->isFull = false; // release the buffer
    }
}

bool findNextMsgStart(volatile Buffer *buffer){
    if(buffer->Blocked){
        return(findFlag(buffer, (uint8_t *)buffer->array + buffer->msgStartIdx));// +1 to skip the current start flag
    }else{
        return false;
    }
}


bool findFlag(volatile Buffer *buffer, void *data){

    uint8_t i = buffer->tail; // where to start searching

    if(buffer->Blocked){
        i = (buffer->msgStartIdx+1) % buffer->arraySize;
    }

    for (i ; i != buffer->head; i = (i+1) % buffer->arraySize) {
        if(memcmp((uint8_t *)buffer->array + i, data, 1) == 0){
        buffer->msgStartIdx = i;
        return true;
        }
    }
    // if the data is not found
    return false;
}

void jumpToMsgStart(volatile Buffer *buffer){
    
    if(buffer->Blocked){
        buffer->tail = buffer->msgStartIdx;
        buffer->isEmpty = (buffer->head == buffer->tail); // no data to read
        buffer->isFull = !(buffer->head == buffer->tail); // no place to write
    }
    return;
    
}

void rollback( volatile Buffer *buffer, uint8_t N){
    // move back the last N elements written incorrectly
    
    if (buffer->isEmpty || N == 0) {
        return;
    }
    // moves the head backwards by N elements
    if(N >= howMuchData(buffer)){
        buffer->head = buffer->tail;
        buffer->isEmpty = true;
    }else{
        buffer->head = (buffer->arraySize - N + buffer->head) % buffer->arraySize;
    }
    return;
}

void markMsg(volatile Buffer *buffer){
    if(buffer->msgCount>=3){
        buffer->dataLoss = true;
        return;
    }
    buffer->msgRanges[buffer->msgCount+1].start = buffer->msgStartIdx;
    buffer->msgRanges[buffer->msgCount+1].end = (buffer->tail + buffer->arraySize) % (buffer->arraySize+1); // = msgEnd - 1 wrapped around the array
    buffer->msgRanges[buffer->msgCount+1].restriction = 2;
    buffer->msgCount++;
    removeMsgStart(buffer); // free the buffer
    return;
}

void unmarkMsg (volatile Buffer *buffer){
    buffer->msgRanges[1].restriction = 1;
    buffer->Blocked = false;
    return;
}

void getMsg(volatile Buffer *buffer, uint8_t* msgOut, uint8_t* msgSize){
    // gets the oldest message found in buffer
    if(buffer->msgCount == 0){
        return;
    }
    
    uint8_t sz;
    bool normalOrder = buffer->msgRanges[1].end >= buffer->msgRanges[1].start;
    if (normalOrder) {
        // Message doesn't wrap around the end of the buffer
        sz = buffer->msgRanges[1].end - buffer->msgRanges[1].start + 1;
        memcpy(msgOut, (uint8_t *)buffer->array + buffer->msgRanges[1].start, sz);
    } else {
        // Message wraps around the end of the buffer
        uint8_t firstPartSize = buffer->arraySize - buffer->msgRanges[1].start;
        sz = (buffer->arraySize - buffer->msgRanges[1].start) + (buffer->msgRanges[1].end + 1);
        // copy the start of the message
        memcpy(msgOut, (uint8_t *)buffer->array + buffer->msgRanges[1].start, firstPartSize);
        // the rest of the message
        memcpy(msgOut+firstPartSize, (uint8_t *)buffer->array ,  buffer->msgRanges[1].end+1);
    }

    *msgSize = sz;   
    // remove the message from the buffer
    unmarkMsg(buffer);
    buffer->msgCount = buffer->msgCount - 1;
    // shift the msgRanges to next one is in msgRanges[0]
    for (uint8_t i = 1; i < 4 ; i++) {
        buffer->msgRanges[i-1] = buffer->msgRanges[i];
    }
    return;
}