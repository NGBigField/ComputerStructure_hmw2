#include "cache.h"
#include <algorithm>
#include <math.h>


/*=============================================== Proxy Functions  ===============================================*/
uli get_middle_bits(uli input, int numLSBbits2ignore, int numBits2keep)
{
	input = input / ((uli)pow(2, numLSBbits2ignore));   //ignore last "numLSBbits2ignore" bits;
	uli res = input % ((uli)pow(2, numBits2keep)); //keep last "numBits2keep" bits
	return res;
}


uli address2set(uli address, int blockSize, int numBlocks)
{
	int offset_length = (int)log2(blockSize) + 2;
	int set_length = (int)log2(numBlocks);
	return get_middle_bits(address, offset_length, set_length);
}

uli address2tag(uli address, int blockSize, int numBlocks)
{
	int offset_length = (int)log2(blockSize) + 2;
	int set_length = (int)log2(numBlocks);
	return get_middle_bits(address, offset_length + set_length, ADDRESS_LENGTH - (offset_length + set_length));
}




/*=============================================== way  ===============================================*/
way::way(int numBlocks_, int blockSize_)
{
	numBlocks = numBlocks_;
	blockSize = blockSize_;

	blocks = new int[numBlocks];
	is_dirty = new bool[numBlocks];
	tags = new uli[numBlocks];
	addresses = new uli[numBlocks];
	

	std::numeric_limits<uli> limits;
	uli maxVal_ = limits.max();
	uli minVal_ = limits.min();

	/*Initialize the arrays*/
	for (int i = 0; i < numBlocks; i++)
	{
		blocks[i] = 0;
		is_dirty[i] = false;
		tags[i] = maxVal_;  
		addresses[i] = 0;
	}

}


bool way::is_exist(uli address)
{
	int set = address2set(address,blockSize,numBlocks);
	uli crnt_tag = tags[set];
	return ( (crnt_tag == address2tag(address, blockSize, numBlocks)) ? true : false  ) ;
}

bool way::change_block(uli address , uli* pOldAddress)
{
	int set = address2set(address, blockSize , numBlocks);
	uli new_tag = address2tag(address, blockSize, numBlocks);

	/*Before update*/
	*pOldAddress = addresses[set];
	bool old_dirty = is_dirty[set];

	/*After update*/
	tags[set] = new_tag;
	addresses[set] = address;
	is_dirty[set] = false;

	return old_dirty;
}

void way::write(uli address)
{
	int set = address2set(address,blockSize,numBlocks);
	is_dirty[set] = true;
}




/*=============================================== cache  ===============================================*/
cache::cache(int cacheSize_, int blockSize_, int numWays_, int numCycles_, bool is_writeAllocate_ , int level_)
{
	//Basic Params:
	cacheSize	= cacheSize_;
	blockSize	= blockSize_;
	numWays		= numWays_;
	numCycles	= numCycles_;
	is_writeAllocate = is_writeAllocate_;

	//
	level = level_;
	numBlocks_perWay = cacheSize / (blockSize * numWays);

	/*Creat ways:*/
	for (int i = 0; i < numWays; i++)
	{
		ways.push_back(new way(numBlocks_perWay, blockSize)  );
	}


	/*Create LRU list:*/
	for (int blockIndx = 0; blockIndx < numBlocks_perWay; blockIndx++)
	{
		deque<int> newLRUelem;
		for (int wayIndx = 0; wayIndx < numWays; wayIndx++)
		{
			newLRUelem.push_back(wayIndx);
		}
		LRU.push_back(newLRUelem);
	}
		

	missCount = 0;
	accessCount = 0;
}



bool cache::search_and_update(uli address, bool is_actionWrite , uli* pAddressOfDirtyData )
{
	accessCount++;

	uli  setIndex = address2set(address, blockSize, numBlocks_perWay);
	for (int wayIndex = 0; wayIndex < numWays; wayIndex++)
	{
		if (ways[wayIndex]->is_exist(address))
		{
			if (is_actionWrite)
			{
				ways[wayIndex]->write(address);
			}
			/*update LRU*/
			update_LRU(setIndex, wayIndex);

			return true;
		}
	}

	/*Missing block  :  Must update one of the ways, if the new requested block*/
	missCount++;
	
	if (!is_actionWrite   ||  is_writeAllocate)  //Read or write allocate
	{
		
		bool is_dirty = false;
		int way2update = LRU[setIndex].front();

		is_dirty = ways[way2update]->change_block(address, pAddressOfDirtyData);
		update_LRU(setIndex, way2update);

	}
	return false;
}



double cache::get_missRate()
{
	return  ((double)missCount) / ((double)accessCount);
}


void cache::update_LRU(int setIndex, int way2update)
{
	deque<int> stack;  //stack to keep all the relevant ways in order.
	int crntWay = LRU[setIndex].front();

	/*peel off queue until we find the element we're looking for*/
	while (crntWay != way2update)
	{
		//check something that shouldn't happen:
		if (LRU[setIndex].empty())
		{
			printf("L%d::update_LRU(setIndex=%d, way2update%d)	--element not found" , this->level , setIndex , way2update);
			return;
		}

		stack.push_front(crntWay);  //keep relevant ways in order
		LRU[setIndex].pop_front();  // take from queue untill we find way2update
		crntWay = LRU[setIndex].front();
	}

	/*put the element at end of queue*/
	LRU[setIndex].pop_front();
	LRU[setIndex].push_back(crntWay);

	/*put everything else back to order:*/
	while (!stack.empty())
	{
		LRU[setIndex].push_front(stack.front());
		stack.pop_front();
	}

}


/*=============================================== Main Functions:  ===============================================*/

int calc_time_and_update(cache* pL1, cache* pL2, uli address, char action_char, unsigned MemCyc, bool is_writeAllocate)
{
	//convert action to bool:
	bool is_actionWrite = (action_char == 'w') ? true : false;
	uli oldAdress = 0;

	/*Search in L1*/
	bool is_exist_L1 = pL1->search_and_update(address, is_actionWrite , &oldAdress);
	if (is_exist_L1) 
	{
		return pL1->get_numCycles();
	}
	else
	{
		///*If something dirty was changed,  Update in L2:*/
		//if (pDirtyAddress)  //Not NULL   so there was a dirty address
		//{
		//	//We know it must be exist. But this function will also update  like dirty to true
		//	pL2->search_and_update(address, is_actionWrite, pDirtyAddress);
		//}


		/*Search in L2*/
		bool is_exist_L2 = pL2->search_and_update(address, is_actionWrite, &oldAdress);
		if (is_exist_L2)
		{
			return  pL1->get_numCycles() + pL2->get_numCycles();
		}
		else
		{
			return MemCyc + pL1->get_numCycles() +pL2->get_numCycles();
		}

	}



}