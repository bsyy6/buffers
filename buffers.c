#include "buffers.h"

Buffer initBuffer(void *data, uint8_t arraySize) {
    return (Buffer) {
        .initialArray = data,
        .initialArraySize = arraySize,
        .array = data,
        .arraySize = arraySize,
        .head = 0,
        .tail = 0,
        .msgStartIdx = 0,
        .isEmpty = true,
        .isFull = false,
        .dataLoss = false,
        .Blocked = false,
        .msgCount = 0,
    };
}

void enq(void *data, volatile Buffer *buffer) {
    buffer->dataLoss = buffer->dataLoss || buffer->isFull;
    
    if(BLOCK_WHEN_FULL &&  buffer->dataLoss){
        // doesn't add anymore
        return;
    }else{
        
        
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
    
    

    memcpy(data, (uint8_t *)buffer->array + buffer->tail, 1);
    buffer->tail = (buffer->tail + 1) % buffer->arraySize;
    // check that buffer tail is not pointing to a blocked range
    if(buffer->msgCount > 0){
        if(buffer->tail <= buffer->msgRanges[buffer->msgCount-1].end){
            buffer->tail = buffer->msgRanges[buffer->msgCount-1].end + 1;
        }
    }

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
        if(buffer->tail == 0){
            buffer->msgStartIdx = buffer->arraySize - 1;
        }else{
            buffer->msgStartIdx = buffer->tail - 1;
        }
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

    for (i=i ; i != buffer->head; i = (i+1) % buffer->arraySize) {
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

void enqMsg(volatile Buffer *buffer){
    if(buffer->msgCount>=3){
        buffer->dataLoss = true;
        return;
    }
    
    if(buffer->Blocked){
        uint8_t end =  (buffer->tail == 0) ? (buffer->arraySize - 1) : (buffer->tail - 1);
        uint8_t start = buffer->msgStartIdx;
        uint8_t msgLength = (end >= start) ? end - start + 1 : buffer->arraySize - start + end + 1;

        buffer->msgRanges[buffer->msgCount].start = start;
        buffer->msgRanges[buffer->msgCount].end = end; // = msgEnd - 1 wrapped around the array
        buffer->msgCount++;
        shiftMsgLeft(buffer);
        updateBufferStart(buffer);
        removeMsgStart(buffer); // free the buffer
    }
    return;

}

void deqMsg (volatile Buffer *buffer){
    // removes the oldest message from the buffer
    if(buffer->msgCount == 0){
        return;
    }
    if(buffer->msgCount >1 ){
        buffer->array = buffer->initialArray + buffer->msgRanges[0].end + 1;
    }else{
        buffer->array = buffer->initialArray;
    }
    buffer->msgCount = buffer->msgCount - 1;
    // buffer->Blocked = false;
    // shift the msgRanges to next one is in msgRanges[1]
    for (uint8_t i = 1; i <= buffer->msgCount ; i++) {
        buffer->msgRanges[i-1] = buffer->msgRanges[i];
    }
    return;
}

void getMsg(volatile Buffer *buffer, uint8_t* msgOut, uint8_t* msgSize){
    // gets the oldest message found in buffer
    if(buffer->msgCount == 0){
        return;
    }
    
    uint8_t sz = 0;
    bool normalOrder = buffer->msgRanges[0].end >= buffer->msgRanges[0].start;
    if (normalOrder) {
        // Message doesn't wrap around the end of the buffer
        sz = buffer->msgRanges[0].end - buffer->msgRanges[0].start + 1;
        memcpy(msgOut, (uint8_t *)buffer->array + buffer->msgRanges[0].start, sz);   
    }
    *msgSize = sz;   
    // remove the message from the buffer
    deqMsg(buffer);
    return;
}


void shiftBuffer(volatile Buffer *buffer, uint8_t n ){
    // scrolls the buffer to the right n times
    // moves the head and tail to the right n times and msgStartIdx if blocked

    if(n == 0){
        return;
    }

    // keeps aways from messages
    uint8_t startScroll = buffer->msgRanges[buffer->msgCount-1].end + 1;

    shiftRight(buffer->array+startScroll, n, buffer->arraySize - startScroll);

    buffer->head = (buffer->head + n) % buffer->arraySize;
    buffer->tail = (buffer->tail + n) % buffer->arraySize;
    if(buffer->Blocked){
        buffer->msgStartIdx = (buffer->msgStartIdx + n) % buffer->arraySize;
    }
    return;
}

/*  takes current message and lsh 
    so it is either in the start of buffer 
    or after the last message.
*/
void shiftMsgLeft(volatile Buffer *buffer){
   
    if(buffer->msgCount == 0){
        return;
    }
    
    uint8_t n = 0;
    uint8_t ArrayStart = 0;
    uint8_t jumpBlockedRange = 0;


    if(buffer->msgCount == 1){
         // if there is only one message
       n = buffer->msgRanges[buffer->msgCount-1].start;

    }
    else{
        n = buffer->msgRanges[buffer->msgCount-1].start - buffer->msgRanges[buffer->msgCount-2].end -1 ;
        ArrayStart = buffer->msgRanges[buffer->msgCount-2].end+1;
        jumpBlockedRange = buffer->msgRanges[buffer->msgCount-1].end - n + 1;
    }
    

    for(uint8_t i = 0; i < buffer->msgCount; i++){
        buffer->msgRanges[i].start = (buffer->msgRanges[i].start + buffer->arraySize - n) % buffer->arraySize;
        buffer->msgRanges[i].end = (buffer->msgRanges[i].end + buffer->arraySize - n) % buffer->arraySize;
    }

    shiftLeft(buffer->array+ArrayStart, n, buffer->arraySize - ArrayStart);
   
    buffer->head = ((buffer->head  + buffer->arraySize - n) % buffer->arraySize ) + jumpBlockedRange;// + buffer->msgRanges[buffer->msgCount-1].end+1;
    buffer->tail = ((buffer->tail  + buffer->arraySize - n) % buffer->arraySize ) + jumpBlockedRange;// + buffer->msgRanges[buffer->msgCount-1].end+1;

    if(buffer->Blocked){
        buffer->msgStartIdx = (buffer->msgStartIdx + buffer->arraySize - n) % buffer->arraySize;
    }
    return;
}

void reverse(uint8_t *arr, uint8_t start, uint8_t end) {
    while (start < end) {
        uint8_t temp = arr[start];
        arr[start] = arr[end];
        arr[end] = temp;
        start++;
        end--;
    }
    return;
}

void shiftRight(uint8_t *arr, uint8_t n, uint8_t size) {
    n = n % size;
    if(n==0) return;
    reverse(arr, 0, size - 1);
    reverse(arr, 0, n - 1);
    reverse(arr, n, size - 1);
    return;
}

void shiftLeft(uint8_t *arr, uint8_t n, uint8_t size) {
    n = n % size;
    if(n==0) return;
    reverse(arr, 0, size - 1); // all array
    reverse(arr, 0, size - n - 1); // first size-n elements
    reverse(arr, size - n, size - 1); // last n elements
    return;
}

void updateBufferStart(volatile Buffer *buffer){
    // move the buffer array to after the last message
    if(buffer->msgCount == 0){
        buffer->array = buffer->initialArray;
        buffer->arraySize = buffer->initialArraySize;   
    }else{
        buffer->array = buffer->initialArray + buffer->msgRanges[buffer->msgCount-1].end + 1;
        buffer->arraySize = buffer->initialArraySize - buffer->msgRanges[buffer->msgCount-1].end - 1;
        buffer->head = buffer->head - buffer->msgRanges[buffer->msgCount-1].end - 1;
        buffer->tail = buffer->tail - buffer->msgRanges[buffer->msgCount-1].end - 1;
        if(buffer->Blocked){
            buffer->msgStartIdx = buffer->msgStartIdx - buffer->msgRanges[buffer->msgCount-1].end - 1;
        }
    }
    return;
}
