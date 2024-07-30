#include "buffers.h"
#include <stdio.h>
// buffer object
Buffer b; // buffer object
uint8_t data[5];  // array I want to use as buffer

uint8_t msgOut[5]; 
uint8_t msgSize;

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
	dataIn = 0;
	enq(&dataIn,&b);  // add element to object
	deq(&dataOut,&b); // read from buffer and store it in data out
	
	printf("dataOut: %d\tdataIn: %d\n",dataOut,dataIn);

	dataIn = 1;
	enq(&dataIn,&b);  // add 1
	deq(&dataOut,&b); // read 1
	setMsgStart(&b);  // set bookmark
	enq(&dataIn,&b);  // add 1
	deq(&dataOut,&b); // read 1
	enqMsg(&b); // block the message [ 1 1 ]

	dataIn=9; //wrong data
	enq(&dataIn,&b);  // add 9
	dataIn=2; 
	enq(&dataIn,&b);  // add 2
	deq(&dataOut,&b); // read 9
	deq(&dataOut,&b); // read 2
	setMsgStart(&b);  // set bookmark
	enq(&dataIn,&b);  // add 2
	deq(&dataOut,&b); // read 2
	dataIn = 3;
	enq(&dataIn,&b);  // add 3
	enqMsg(&b); // block the message [ 2 2 ]	
	deq(&dataOut,&b); // read 3
	dataIn = 3;
	enq(&dataIn,&b);  // add 3
	deq(&dataOut,&b); // read wrong data
	deq(&dataOut,&b); // read 2
	dataIn = 4;
	enq(&dataIn,&b);  // add 4
	getMsg(&b,msgOut,&msgSize); // get the first element
	dataIn = 5;
	enq(&dataIn,&b);  // add 5
	enq(&dataIn,&b);  // add 5
	deq(&dataOut,&b); // read 5
	setMsgStart(&b);  // set bookmark
	deq(&dataOut,&b); // read 2
	enqMsg(&b); // block the message
	getMsg(&b,msgOut,&msgSize); // get the first element
	getMsg(&b,msgOut,&msgSize); // get the first element

	deq(&dataOut,&b); // read ?
	dataIn = 99;
	enq(&dataIn,&b);  // add 2

	// enq(&dataIn,&b);  // add element to object
	// unmarkMsg(&b); // unblock the first element
	// getMsg(&b,&nDataOut); // get the first element
	// deq(&dataOut,&b); // read from buffer and store it in data out
	// dataIn++;
	// enq(&dataIn,&b);  // add element to object
	// dataIn++;
	// enq(&dataIn,&b);  // add element to object
	// // also supports multiple elements write/read
	// deq(&dataOut,&b); // read from buffer and store it in data out
	// deq(&dataOut,&b); // read from buffer and store it in data out
	// deq(&dataOut,&b); // read from buffer and store it in data out
	// deq(&dataOut,&b); // read from buffer and store it in data out
	// reset(&b); // resets the buffer object to initial state
	// nEnq(&nDataIn,&b,3);
	// nDeq(&nDataOut,&b,3); // now nDataOut = [1,2,3];

	// // supports partial buffer read/write
	// reset(&b); // resets the buffer object to initial state
	// nDataIn[0] = 4;
	// nDataIn[1] = 5;
	// nDataIn[2] = 6;
	// nEnq(&nDataIn,&b,3);
	// nDeq(&nDataOut,&b,2); // reads only 2 elemetns -> nDataOut = [4,5,3];
	
	return 0;
}