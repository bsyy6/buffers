#include "buffers.h"

// buffer object
volatile Buffer b; // buffer object
uint8_t data[5];  // array I want to use as buffer

// single data example
uint8_t dataIn;   // holder for bytes I want to save in buffer
uint8_t dataOut;  // holder for bytes I will later read from buffer

//multiple data example
uint8_t nDataIn[3] = {3,3,3};   // holder for bytes I want to save in buffer
uint8_t nDataOut[3] = {0,0,0};  // holder for bytes I will later read from buffer

int main(){
	// initialize the buffer object
	b = initBuffer(data,5);
    
	// single data wirte/read
	dataIn = 1;
	enq(&dataIn,&b);  // add element to object
	deq(&dataOut,&b); // read from buffer and store it in data out
	setMsgStart(&b);  // set bookmark
	dataIn++;
	enq(&dataIn,&b);  // add element to object
	dataIn++;
	enq(&dataIn,&b);  // add element to object
	dataIn++;
	deq(&dataOut,&b); // read from buffer and store it in data out
	markMsg(&b); // block the first element
	enq(&dataIn,&b);  // add element to object
	unmarkMsg(&b,0,0); // unblock the first element
	deq(&dataOut,&b); // read from buffer and store it in data out
	dataIn++;
	enq(&dataIn,&b);  // add element to object
	dataIn++;
	enq(&dataIn,&b);  // add element to object
	// also supports multiple elements write/read
	deq(&dataOut,&b); // read from buffer and store it in data out
	deq(&dataOut,&b); // read from buffer and store it in data out
	deq(&dataOut,&b); // read from buffer and store it in data out
	deq(&dataOut,&b); // read from buffer and store it in data out
	reset(&b); // resets the buffer object to initial state
	nEnq(&nDataIn,&b,3);
	nDeq(&nDataOut,&b,3); // now nDataOut = [1,2,3];

	// supports partial buffer read/write
	reset(&b); // resets the buffer object to initial state
	nDataIn[0] = 4;
	nDataIn[1] = 5;
	nDataIn[2] = 6;
	nEnq(&nDataIn,&b,3);
	nDeq(&nDataOut,&b,2); // reads only 2 elemetns -> nDataOut = [4,5,3];
	
	return 0;
}