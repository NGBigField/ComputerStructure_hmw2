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

	unsigned MemCyc = 100, BSize = 5, L1Size = 16, L2Size = 20, L1Assoc = 3,
	L2Assoc = 4, L1Cyc = 1, L2Cyc = 5, WrAlloc = 1;


	/*Our Code: START*/
	cache* pL1 = new cache(pow(2, L1Size), pow(2, BSize), pow(2, L1Assoc), L1Cyc, (bool)WrAlloc);
	cache* pL2 = new cache(pow(2, L2Size), pow(2, BSize), pow(2, L2Assoc), L2Cyc, (bool)WrAlloc);
	/*Our Code: END*/



	/* Read \ Write Requests: */
	char operation = 'W';  // R  or  W
	unsigned long int address = 5;


	/* Update those: */
	double L1MissRate = 0;
	double L2MissRate = 0;
	double avgAccTime = 0;




	printf("L1miss=%.03f ", L1MissRate);
	printf("L2miss=%.03f ", L2MissRate);
	printf("AccTimeAvg=%.03f\n", avgAccTime);

	return 0;
}
