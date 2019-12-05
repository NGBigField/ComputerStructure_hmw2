#ifndef _CACHE_
#define _CACHE_


#include <vector>

using std::vector;

typedef unsigned long int   uli; 
const int ADDRESS_LENGTH = 32;

/*Our Proxy Functions:*/
uli get_middle_bits(uli input, int numLSBbits2ignore, int numBits2keep);
uli address2set(uli address, int blockSize, int numBlocks);
uli address2tag(uli address, int blockSize, int numBlocks);


/*Our Classes:*/

class way {
public:
	way(int numBlocks, int blockSize); //c'tor
	bool is_exist(uli address);
	bool change_block(uli address);  //also return if it was dirty
	void write(uli address);
	//uli tag2address(int set);

protected:

	int numBlocks;
	int blockSize;

	int *blocks;
	bool* is_dirty;
	unsigned long int* tags;
	uli* addresses;

	
};



class cache {

public:
	cache(int cacheSize_ , int blockSize_ , int numWays_, int numCycles_ , bool is_writeAllocate_ ); //c'tor
	bool is_exist(uli address , bool is_actionWrite , uli* pDirtyAddress);
	int get_numCycles() { return numCycles; }
	double get_missRate();
	
protected:
	//Basic Params:
	int cacheSize;
	int blockSize;
	int numWays;
	int numCycles;
	bool is_writeAllocate;

	//===//:
	vector<way*> ways;
	int* LRU;  //size of LRU:   numBlocks_perWay
	unsigned int missCount;
	unsigned int accessCount;
	int numBlocks_perWay;
};


#endif



/* Main Functions: */
int calc_time_and_update(cache* pL1, cache* pL2, uli address, char action, unsigned MemCyc , bool is_writeAllocate);