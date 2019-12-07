#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>

#include "cache.h"

using std::FILE;
using std::string;
using std::cout;
using std::endl;
using std::cerr;
using std::ifstream;
using std::stringstream;

int main() {

	unsigned MemCyc = 100, BSize = 0, L1Size = 0, L2Size = 20, 
		L1Assoc = 0, L2Assoc = 4, L1Cyc = 1, L2Cyc = 5, WrAlloc = 1;


	/*Our Code: START*/
	cache* pL1 = new cache((int)pow(2, L1Size), (int)pow(2, BSize), (int)pow(2, L1Assoc), L1Cyc, (bool)WrAlloc , 1);
	cache* pL2 = new cache((int)pow(2, L2Size), (int)pow(2, BSize), (int)pow(2, L2Assoc), L2Cyc, (bool)WrAlloc , 2);
	/*Our Code: END*/


	char operation;
	unsigned long int address;
	int crntTimePassed = 0;		//in clk cycles
	int globalTimePassed = 0;  //in clk cycles
	int cmnd_count = 0;

	/* Read \ Write Requests: */
	operation = 'w'; 
	address = 4;
	crntTimePassed = calc_time_and_update(pL1, pL2, address, operation, MemCyc, WrAlloc);
	globalTimePassed += crntTimePassed;
	cmnd_count++;


	operation = 'r';  
	address = 16;
	crntTimePassed = calc_time_and_update(pL1, pL2, address, operation, MemCyc, WrAlloc);
	globalTimePassed += crntTimePassed;
	cmnd_count++;

	operation = 'w';
	address = 4;
	crntTimePassed = calc_time_and_update(pL1, pL2, address, operation, MemCyc, WrAlloc);
	globalTimePassed += crntTimePassed;
	cmnd_count++;



	/* Update those: */
	double L1MissRate = pL1->get_missRate();
	double L2MissRate = pL2->get_missRate();
	double avgAccTime = ((double)globalTimePassed)/((double)cmnd_count);



	printf("L1miss=%.03f ", L1MissRate);
	printf("L2miss=%.03f ", L2MissRate);
	printf("AccTimeAvg=%.03f\n", avgAccTime);

	return 0;
}
