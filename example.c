#include "buffers.h"

// buffer object
volatile Buffer b; // buffer object
uint16_t data[3];  // array I want to use as buffer

// single data example
uint16_t dataIn;   // holder for bytes I want to save in buffer
uint16_t dataOut;  // holder for bytes I will later read from buffer

//multiple data example
uint16_t nDataIn[3] = {1,2,3};   // holder for bytes I want to save in buffer
uint16_t nDataOut[3] = {0,0,0};  // holder for bytes I will later read from buffer

int main(){
	// initialize the buffer object
	b = initBuffer(data, sizeof(data[0]), sizeof(data)/sizeof(data[0]));
    
	// single data wirte/read
	dataIn = 1;
	enq(&dataIn,&b);  // add element to object
	deq(&dataOut,&b); // read from buffer and store it in data out

	// also supports multiple elements write/read
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