# Buffers


A minimal fifo circular buffer library for embedded systems [limited to 255 array size]. You should:
1) initialize a normal array.
```c
uint16_t data[3];  // array I want to use as buffer
```
2) assign it to a buffer object.
```c
volatile Buffer b;
b = initBuffer(data, sizeof(data[0]), sizeof(data)/sizeof(data[0]));
```
3) use enq/deq to add/read data.
```c
uint16_t dataIn;   // holder for bytes I want to save in buffer
uint16_t dataOut;  // holder for bytes I will later read from buffer

enq(&dataIn,&b)  // add element to object
deq(&dataOut,&b) // read from buffer and store it in data out
```

## Examples:
### Single Data writes
```c
#include<buffers.h>

volatile Buffer b; // buffer object
uint16_t data[3];  // array I want to use as buffer

// single data example
uint16_t dataIn;   // holder for bytes I want to save in buffer
uint16_t dataOut;  // holder for bytes I will later read from buffer

int main(){
	// initialize the buffer object
	b = initBuffer(data, sizeof(data[0]), sizeof(data)/sizeof(data[0]));
    
	// single data wirte/read
	dataIn = 1;
	enq(&dataIn,&b)  // add element to object
	deq(&dataOut,&b) // read from buffer and store it in data out
}
```

### Multiple data write
```c
#include<buffers.h>

volatile Buffer b; // buffer object
uint16_t data[3];  // array I want to use as buffer

// multiple data example
uint16_t nDataIn[3] = {1,2,3};   // holder for bytes I want to save in buffer
uint16_t nDataOut[3] = {0,0,0};  // holder for bytes I will later read from buffer

int main(){
	// initialize the buffer object
	b = initBuffer(data, sizeof(data[0]), sizeof(data)/sizeof(data[0]));
	
	nEnq(&nDataIn,&b,3);
	nDeq(&nDataOut,&b,3); // now nDataOut = [1,2,3];
	
	// supports partial buffer read/write
	reset(&b) resets the buffer object to initial state
	nDataIn[0] = 4;
	nDataIn[1] = 5;
	nDataIn[3] = 6;
	nEnq(&nDataIn,&b,3);
	nDeq(&nDataOut,&b,2); // reads only 2 elemetns -> nDataOut = [4,5,3];
	return 0;	
}
```

## Buffer structure

The `Buffer` structure represents a circular buffer. It contains the following fields:

- `void *array`: This is a pointer to the array that is used as the buffer.
- `uint8_t elementSize`: This field represents the size (in bytes) of each element in the buffer.
- `uint8_t arraySize`: This field represents the number of elements in the array.
- `uint8_t head`: This is the write index. It points to where the next element will be written.
- `uint8_t tail`: This is the read index. It points to where the next element will be read.
- `bool isEmpty`: This field is `true` if there are no elements in the buffer, and `false` otherwise.
- `bool isFull`: This field is `true` if the buffer has overflowed at some point (i.e., data has been lost), and `false` otherwise.
- `uint8_t whatIsLife`: This is used for debugging.


## Buffer methods

- `Buffer initBuffer(void *array, uint8_t elementSize, uint8_t arraySize)`: Initializes a `Buffer` with the given array, element size, and array size.

- `void enq(void *data, volatile Buffer *buffer)`: Enqueues (adds) the given data to the buffer.

- `void deq(void *data, volatile Buffer *buffer)`: Dequeues (removes) data from the buffer and stores it in the location pointed to by `data`.

- `void nEnq(void *data, volatile Buffer *buffer, uint8_t size)`: Enqueues a number of elements specified by `size` from the data to the buffer.

- `void nDeq(void *data, volatile Buffer *buffer, uint8_t size)`: Dequeues a number of elements specified by `size` from the buffer and stores them in the location pointed to by `data`.

- `void reset(volatile Buffer *buffer)`: Resets the buffer, clearing all data.

- `uint8_t howMuchData(volatile Buffer *buffer)`: Returns the amount of data currently stored in the buffer.
