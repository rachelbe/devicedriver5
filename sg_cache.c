////////////////////////////////////////////////////////////////////////////////
//
//  File           : sg_driver.c
//  Description    : This file contains the driver code to be developed by
//                   the students of the 311 class.  See assignment details
//                   for additional information.
//
//   Author        : Rachel Brooks
//   Last Modified : 11/16/2020
//

// Include Files
#include <stdlib.h>
#include <cmpsc311_log.h>

// Project Includes
#include <sg_cache.h>

// Defines

// Functional Prototypes

//
// Functions

//#include <cmpsc311_log.h>

//created cache


typedef struct 
{
    

    
    int time; // keeps the time the 
    char datablock[1024];

    uint64_t  nodeID; //node ID
    uint64_t  blockID; //block ID

    int occupied; //0 if empty 1 if not



    //where blocks are located (20 max)
    


}blockstruct;

blockstruct block; //block

blockstruct *cache = NULL;  //global variable

int hits =0;  //global value for hits
int misses  =0;  //global variable for misses

//a fucntion that takes in a struct and prints it

////////////////////////////////////////////////////////////////////////////////
//
// Function     : structprint
// Description  : prints the strut values
	//not used only for testing
//
// Inputs       : no input
// Outputs      : void and doesnt return
void structprint(blockstruct *b)
{
	printf("Time: %d\n",b->time);
	printf("Data Blocks: ");
	for (int i = 0; i<5; i++)
	{
		printf("%c",b->datablock[i]);
	}
	printf("\n");
	printf("Node ID: %lu\n",b->nodeID);

	printf("Block ID: %lu\n",b->blockID);


}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : printcache
// Description  : prints the cache
	//not used only for testing
//
// Inputs       : no input
// Outputs      : void and doesnt return
void printcache()
{

	for (int i =0;i<128;i++)
	{
		structprint(&cache[i]);
		printf("\n\n");

	}
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : setcache
// Description  : create the cache and initializes it to empty
//
// Inputs       : no input
// Outputs      : returns the cache
blockstruct *setcache()
{
	cache = malloc((128) * sizeof(blockstruct));  //create cache array
	

	for(int i=0;i<128;i++)  //set occupied to 0
	{
		cache[i].occupied =0;
		//cache[i].datablock = malloc((1024) * sizeof(char));

	}

	return(cache);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : setlen
// Description  : finds the legnth of the cache and if any spots are empty
//
// Inputs       : no input
// Outputs      : returns spots that are full
int checklen()
{
	int countlen =0;
	for (int i =0; i<128; i++)  //loops through cache
	{
		if (cache[i].occupied==1)
		{
			countlen+=1;  //keeps count of occupied spots
		}
	}
	return(countlen); //returns the count
}
void freecache()
{

	free(cache);
}


////////////////////////////////////////////////////////////////////////////////
//
// Function     : addin
// Description  : add in the information to the cache
//
// Inputs       : node
//                block
//                data block
// Outputs      : 0 if successful, -1 if failure
int addin(uint64_t  node,uint64_t  block, char *data)
{


	if(node==0 || block ==0 || data == NULL)  //check if any of them are empty
	{
		return (-1); //return 0
	}
	int highest=0;
	
	for(int k=0;k<128;k++)  //find the highest time
	{
		if(cache[k].time>highest)
		{
			highest =cache[k].time;
			
		}
	}

	int i =0;

	while(cache[i].occupied ==1)  //find next open spot
	{
		i++;
	}
	
	cache[i].occupied = 1;  //set it to occupied
	cache[i].time = highest + 1;  //set the time

	cache[i].nodeID = node;  //set the node
	cache[i].blockID = block; //set the block

	for(int j=0; j<1024;j++)
	{
		
		cache[i].datablock[j]= data[j];  //set data into datablock
	}

	return(0);
	

}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : evict
// Description  : removes the least recently used from the cache
//
// Inputs       : no inputs
// Outputs      : void function no outputs
void evict() //add input
{
	int lowest=999999999;  // highest number to use to compare
	int lowestindex =0; //start at 0 index
	for(int i=0;i<128;i++) 
	{
		if(cache[i].time<lowest && cache[i].occupied==1) //finds the lowest time thats occupied
		{
			
			lowest =cache[i].time;
			
			lowestindex=i;
			
		}
	}

	//reset the data so its gone from the cache now
	for(int j=0; j<1024;j++)
	{
		
		cache[lowestindex].datablock[j] = 0;
	}
	cache[lowestindex].nodeID = 0;
	cache[lowestindex].blockID = 0;
	cache[lowestindex].occupied=0;

}


////////////////////////////////////////////////////////////////////////////////
//
// Function     : search
// Description  : searched the cache to find if its in there
//
// Inputs       : node 
//                block
//                
// Outputs      : NULL if its not , or the value if it is 
char *search(uint64_t  node,uint64_t  block)
{
	for (int i=0;i<128;i++) //loop through cache
	{
		if(cache[i].nodeID == node && cache[i].blockID ==block)   //if it is
		{
			int highest=0;
	 
			for(int k=0;k<128;k++)
			{
				if(cache[k].time>highest)  //find the highest time
				{
					highest =cache[k].time;
					
				}
			}
			cache[i].time = highest + 1;  //set the new one to the next time
			return &(cache[i].datablock[0]);  //return it
		}


	}

	return(NULL);

}



/*
int main()
{

	char one[1024] = "hello";
	char two[1024] = "hi";
	char three[1024] = "hey";
	setcache();

	addin(3,17,one);

	addin(2,18,two);
	
	addin(4,20,three);
	
	
	printcache();
	printf("%s\n",search(4,20));

}
*/

////////////////////////////////////////////////////////////////////////////////
//
// Function     : initSGCache
// Description  : Initialize the cache of block elements
//
// Inputs       : maxElements - maximum number of elements allowed
// Outputs      : 0 if successful, -1 if failure

int initSGCache( uint16_t maxElements ) 
{

	setcache(); // call function to set the cache 
    // Return successfully
    return( 0 );
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : closeSGCache
// Description  : Close the cache of block elements, clean up remaining data
//
// Inputs       : none
// Outputs      : 0 if successful, -1 if failure

int closeSGCache( void ) 
{
	//summary of hits and misses

	double total = hits+misses; //calculate the hit ratio
	double hit_ratio = (hits/total) *100 ;



	logMessage(LOG_INFO_LEVEL, "Cache Hits: %d\n", hits);
	logMessage(LOG_INFO_LEVEL,"Cache Misses: %d\n", misses);
	logMessage(LOG_INFO_LEVEL, "Cache Hit Ratio: %1.2f\n", hit_ratio); //hit ratio rounded to 2 decimal places 


	//deallocate cache
	//freecache();

    // Return successfully
    return( 0 );
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : getSGDataBlock
// Description  : Get the data block from the block cache
//
// Inputs       : nde - node ID to find
//                blk - block ID to find
// Outputs      : pointer to block or NULL if not found

char * getSGDataBlock( SG_Node_ID nde, SG_Block_ID blk ) 
{
//null miss
	//not null hit 
	char *get = search(nde,blk); //search if in the cache

	if (get != NULL)  //if it is 
	{
		hits+=1;//add one to hit
		return(get); //return 
	}
	else  //if its not
	{
		misses+=1;//add one to miss 
		return(NULL); // or return NULL
	}

    // Return successfully
    return( 0 );
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : putSGDataBlock
// Description  : Get the data block from the block cache
//
// Inputs       : nde - node ID to find
//                blk - block ID to find
//                block - block to insert into cache
// Outputs      : 0 if successful, -1 if failure

int putSGDataBlock( SG_Node_ID nde, SG_Block_ID blk, char *block ) 
{

	char *in = search(nde,blk); //if its in the cache

	

	if(in!=NULL) //if it is
	{
		
		for(int j=0; j<1024;j++)
		{
			
		
			in[j] = block[j] ; //copy data from block
		}

		hits+=1; //add one to hits

		return(0); //return 0 because successful
	}
	
	misses+=1; //otherwise miss

	
	int emptcount = checklen();  //check to see how may are empty
	if (emptcount == 128)  //if the cache is full
	{
		
		evict();  //evict least recently used
		emptcount = checklen();  //checklen again and it should be 31
		

	}
	
	
	int got = addin(nde,blk,block); //add into the cache
	
	return (got);  //-1 or 0 if it was successful


    
}
