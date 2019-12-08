#ifndef _CACHE_
#define _CACHE_


#include <vector>
#include <queue>
#include <deque>

using std::vector;
using std::queue;
using std::deque;


typedef unsigned long int   uli;
typedef deque<int> int_queue;
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
	bool change_block(uli address, uli* pOldAddress);  //also return if it was dirty
	void write(uli address);
	//uli tag2address(int set);
	void snooped_out(uli address, uli setIndex);

protected:

	int numBlocks;
	int blockSize;

	int* blocks;
	bool* is_dirty;
	unsigned long int* tags;
	uli* addresses;
};



class cache {

public:
	cache(int cacheSize_, int blockSize_, int numWays_, int numCycles_, bool is_writeAllocate_, int level_); //c'tor
	bool search_and_update(uli address, bool is_actionWrite, uli* pDirtyAddress, bool is_update);
	int get_numCycles() { return numCycles; }
	double get_missRate();
	void snooped_out(uli address );


protected:
	void update_LRU(int setIndex, int wayIndex);

	//Basic Params:
	int cacheSize;
	int blockSize;
	int numWays;
	int numCycles;
	bool is_writeAllocate;

	//===//:
	int level;  //cache L1  or L2  etc..
	vector<way*> ways;
	unsigned int missCount;
	unsigned int accessCount;
	int numBlocks_perWay;

	//	int* LRU;  //size of LRU:   numBlocks_perWay
	vector<int_queue> LRU;

};


#endif



/* Main Functions: */
int calc_time_and_update(cache* pL1, cache* pL2, uli address, char action, unsigned MemCyc, bool is_writeAllocate);
