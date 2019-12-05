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
	int set = address2set(address);
	uli crnt_tag = tags[set];
	return ( (crnt_tag == address2tag(address)) ? true : false  ) ;
}

bool way::change_block(uli address)
{
	int set = address2set(address, blockSize , numBlocks);
	uli new_tag = address2tag(address, blockSize, numBlocks);
	tags[set] = new_tag;
	addresses[set] = address;
	bool old_dirty = is_dirty[set];
	is_dirty[set] = false;
	return old_dirty;
}

void way::write(uli address)
{
	int set = address2set(address);
	is_dirty[set] = true;
}


/*
uli way::tag2address(int set)
{
	int offset_length = (int)log2(blockSize) + 2;
	int set_length = (int)log2(numBlocks);

	uli res = (uli)pow(2,offset)

	return get_middle_bits(address, offset_length + set_length, ADDRESS_LENGTH - (offset_length + set_length));
}
*/

/*=============================================== cache  ===============================================*/
cache::cache(int cacheSize_, int blockSize_, int numWays_, int numCycles_, bool is_writeAllocate_)
{
	//Basic Params:
	cacheSize	= cacheSize_;
	blockSize	= blockSize_;
	numWays		= numWays_;
	numCycles	= numCycles_;
	is_writeAllocate = is_writeAllocate_;

	//
	numBlocks_perWay = cacheSize / (blockSize * numWays);

	/*Creat ways:*/
	for (int i = 0; i < numWays; i++)
	{
		ways.push_back(new way(numBlocks_perWay, blockSize)  );
	}

	/*Create LRU list:*/
	LRU = new int[numBlocks_perWay];
	for (int i = 0; i < numBlocks_perWay; i++)
		LRU[i] = 0;


	missCount = 0;
	accessCount = 0;
}



bool cache::is_exist(uli address, bool is_actionWrite , uli* addressOfDirtyData )
{
	accessCount++;
	for (int wayIndex = 0; wayIndex < numWays; wayIndex++)
	{
		if (ways[wayIndex]->is_exist(address))
		{
			if (is_actionWrite)
			{
				ways[wayIndex]->write(address);
			}
			return true;
		}
	}

	/*Missing block*/
	missCount++;

	uli  index = address2set(address, blockSize, numBlocks_perWay);
	 
	if (!is_actionWrite   ||  is_writeAllocate)  //Read = bring the block
	{
		*is_dirtyAndThrew  = ways[LRU[index]]->change_block(address);
		LRU[index] = update_LRU(LRU[index]);
	}
	return false;
}



double cache::get_missRate()
{
	return  ((double)missCount) / ((double)accessCount);
}



/*=============================================== Main Functions:  ===============================================*/

int calc_time_and_update(cache* pL1, cache* pL2, uli address, char action_char, unsigned MemCyc, bool is_writeAllocate)
{
	//convert action to bool:
	bool is_actionWrite = (action_char == 'W') ? true : false;
	uli* pDirtyAddress = NULL;

	/*Search in L1*/
	bool is_exist_L1 = pL1->is_exist(address, is_actionWrite , pDirtyAddress );
	if (is_exist_L1) 
	{
		return pL1->get_numCycles();
	}
	else
	{
		/*If something dirty was changed,  Upsate in L2:*/
		if (pDirtyAddress)  //Not NULL   so there was a dirty address
		{
			//We know it must be exist. But this function will also update  like dirty to true
			pL2->is_exist(address, is_actionWrite, pDirtyAddress);
		}




		/*Search in L2*/
		bool is_exist_L2 = pL2->is_exist(address, is_actionWrite, pDirtyAddress);
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