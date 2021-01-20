////////////////////////////////////////////////////////////////////////////////
//
//  File           : sg_driver.c
//  Description    : This file contains the driver code to be developed by
//                   the students of the 311 class.  See assignment details
//                   for additional information.
//
//   Author        : Rachel Brooks
//   Last Modified : 11-16-2020
//

// Include Files

// Project Includes
#include <sg_driver.h>
#include <sg_service.h>
#include <sg_cache.h>
#include <stdlib.h>
// Defines
//make define for the array size
#define filearr_size 700
//
// Global Data

// Driver file entry

// Global data
int sgDriverInitialized = 0; // The flag indicating the driver initialized
SG_Block_ID sgLocalNodeId;   // The local node identifier
SG_SeqNum sgLocalSeqno;      // The local sequence number


//make struct for all variables
typedef struct 
{
    

    const char *file_name; //file name
    int32_t file_handle; //file handle
    uint32_t file_size; //file size
    int is_open; //if file is open or not (1 if open, 0 if not)
    int file_pointer;

    uint64_t  nodeID[20]; //node ID
    uint64_t  blockID[20]; //block ID
    uint16_t  rsequencenum;  // SGSEQNUM for rqseq

    //where blocks are located (20 max)

}structmake;

structmake file;

//make a struct array to be able to handle multiple files

structmake filearr [filearr_size]; //using malloc to allocate (tried)

int fileamount = 0;  //keep track of how many files
/*
structmake *filearr = NULL;


void function () // any function 
{
    filearr = malloc(30 *sizeof(file_size));
}
int fileamount = 0;  //keep track of how many files
*/
// Driver support functions
int sgInitEndpoint( void ); // Initialize the endpoint


//
// Functions

//
// File system interface implementation
////////////////////////////////////////////////////////////////////////////////
//
// Function     : struct_search
// Description  : searches the array for the file handle to see if it exists already
//
// Inputs       : file handle
// Outputs      : int of where the filehandle is, -1 if it isnt found

int struct_search(SgFHandle fh)
{

    for(int i=0; i<filearr_size; i++)
    {
        structmake fileh=filearr[i];
        if(fh== fileh.file_handle)
        {
            return (i);
        }
        
    }

    return -1;
}
////////////////////////////////////////////////////////////////////////////////
//
// Function     : struct_store
// Description  : puts the filehandle, file name and opens file into the struct
//
// Inputs       : file name, fildhandle
// Outputs      : doesnt return anything

void struct_store(SgFHandle fh, const char* filename)
{


    
    filearr[fileamount].is_open = 1;

    filearr[fileamount].file_name = filename;
    filearr[fileamount].file_handle = fh;




    fileamount ++;


    

    

}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : sgopen
// Description  : Open the file for for reading and writing
//
// Inputs       : path - the path/filename of the file to be read
// Outputs      : file handle if successful test, -1 if failure

SgFHandle sgopen(const char *path) {

    // First check to see if we have been initialized
    
    if (!sgDriverInitialized) {

        // Call the endpoint initialization 
        if ( sgInitEndpoint() ) {
            logMessage( LOG_ERROR_LEVEL, "sgopen: Scatter/Gather endpoint initialization failed." );
            return( -1 );
        }

        // Set to initialized
        sgDriverInitialized = 1;
    }


    /*
    int32_t filehand = 36;
    file.file_handle = filehand;

    file.file_name = path;

    file.file_size = 0;
    file.file_pointer = 0;

    file.is_open = 1; //1 if open
     */
    
//sends the file and the path to struct store 
    struct_store(fileamount, path);

   
    
// FILL IN THE REST OF THE CODE
    initSGCache(128); //call initSGCache which creates the cache with bigger cache size
    
    


    // Return the file handle  (-1 since 1 was added in struct_store)
    return(fileamount-1);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : sgread
// Description  : Read data from the file
//
// Inputs       : fh - file handle for the file to read from
//                buf - place to put the data
//                len - the length of the read
// Outputs      : number of bytes read, -1 if failure

int sgread(SgFHandle fh, char *buf, size_t len) 
{
    
//gets the right spot of the file in the struct and uses it 


    int search = struct_search(fh);
    if(search == -1)
    {
        return -1;
    }

    

//makes sure fh exists and file is open
    if(filearr[search].file_handle != fh || filearr[search].is_open==0)
    {
        return (-1);
    }

//take in base packet size and return data packet size

    char initPacket[SG_BASE_PACKET_SIZE], recvPacket[SG_DATA_PACKET_SIZE];
    size_t pktlen, rpktlen;
    SG_Node_ID loc, rem;
    SG_Block_ID blkid;
    SG_SeqNum sloc, srem;
    SG_System_OP op;
    SG_Packet_Status ret;




    int i=filearr[search].file_pointer/1024; //use i 
//makes 2 ints to search the file
    /*

    int block_spot=0;
    int i=0;

    while(i<=20)
    {
        if(filearr[search].file_pointer!=block_spot)
        {
            i++;
            block_spot+=1024; //go through all blocks to find which one
        }
        else
        {
            break;   
        }
        
    }

    */
//get the block and node from the rigth spot in the file struct
    int the_block = filearr[search].blockID[i]; 
    int the_node = filearr[search].nodeID[i];
    
    

    char tempbuff [1024]; //create an empty tempbuff to use 


    getSGDataBlock( the_node, the_block );  //call getSGDataBlock 


    pktlen = SG_BASE_PACKET_SIZE;
    if ( (ret = serialize_sg_packet( sgLocalNodeId, // Local ID
                                        the_node,   // Remote ID
                                        the_block,  // Block ID
                                        SG_OBTAIN_BLOCK,  // Operation
                                        sgLocalSeqno++,    // Sender sequence number
                                        SG_SEQNO_UNKNOWN,  // Receiver sequence number
                                        NULL, initPacket, &pktlen)) != SG_PACKT_OK ) 
    {
        logMessage( LOG_ERROR_LEVEL, "sgread: failed serialization of packet [%d].", ret );
        return( -1 );
    }


   
        // Send the packet
    rpktlen = SG_DATA_PACKET_SIZE;
    if ( sgServicePost(initPacket, &pktlen, recvPacket, &rpktlen) ) 
    {
        logMessage( LOG_ERROR_LEVEL, "sgread: failed packet post" );
        return( -1 );
    }


  
        // Unpack the recieived data
    if ( (ret = deserialize_sg_packet(&loc, &rem, &blkid, &op, &sloc, 
                                        &srem, tempbuff, recvPacket, rpktlen)) != SG_PACKT_OK ) 
    {
        logMessage( LOG_ERROR_LEVEL, "sgread: failed deserialization of packet [%d]", ret );
        return( -1 );

    }
   
/*
half block implementation
    //check if first or second half of the block
    if (filearr[search].file_pointer % 1024 == 0 ) //starting at the begining at the block for what copies
    {
        for(int i =0; i<512; i++)
        {
            buf[i] = tempbuff[i];
        }
    }
    else
    {
        for(int i =0; i<512; i++)
        {
            buf[i] = tempbuff[512+i];
        }

    }
*/
//quarter block reads
    if (filearr[search].file_pointer % 1024 == 0 ) //starting at the begining at the block for what copies
    {
        for(int i =0; i<256; i++)
        {
            buf[i] = tempbuff[i];
        }
    }
    else if(filearr[search].file_pointer % 512== 0 && filearr[search].file_pointer % 1024 != 0 ) //starting halfway though block
    {
        for(int i =0; i<256; i++)
        {
            buf[i] = tempbuff[512+i];
        }

    }
    else if(filearr[search].file_pointer % 256 == 0  && (((filearr[search].file_pointer-256) % 1024)!= 0 )) //starting at last quarter 768
    {
        for(int i =0; i<256; i++)
        {
            buf[i] = tempbuff[768+i];
        }

    }
    else //starting at second quarter 256
    {
        for(int i =0; i<256; i++)
        {
            buf[i] = tempbuff[256+i];
        }

    }
    filearr[search].file_pointer= filearr[search].file_pointer + len;
// add the len to the file pointer spot

    // Return the bytes processed (len)
    return( len );
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : sgwrite
// Description  : write data to the file
//
// Inputs       : fh - file handle for the file to write to
//                buf - pointer to data to write
//                len - the length of the write
// Outputs      : number of bytes written if successful test, -1 if failure

int sgwrite(SgFHandle fh, char *buf, size_t len) 
{
//gets the right spot of the file in the struct and uses it 
    int search = struct_search(fh);
    if(search == -1)
    {
        return -1;
    }
//makes sure fh exists and file is open
    if(filearr[search].file_handle != fh || filearr[search].is_open==0)
    {
        return (-1);
    }

    // create temp buff then copy the data from buf in 
    char tempbuff [1024];
    for (int i = 0; i<512; i++)
    {
        tempbuff[i]=buf[i];
    }



//if statment for when you have to write to the second half (need to read in first)
    


    if (filearr[search].file_pointer%1024 != 0)  //if writing to any part of the block except first quarter
    {
     
        char writebuff [1024];

        

    //take in base packet size and return data packet size

        char initPacket[SG_BASE_PACKET_SIZE], recvPacket[SG_DATA_PACKET_SIZE];
        size_t pktlen, rpktlen;
        SG_Node_ID loc, rem;
        SG_Block_ID blkid;
        SG_SeqNum sloc, srem;
        SG_System_OP op;
        SG_Packet_Status ret;

        
    //makes 2 ints to search the file
        //int block_spot=0;
        int i=filearr[search].file_pointer/1024;
/*
        while(i<=20)
        {
            if(filearr[search].file_pointer>=block_spot)
            {
                i++;
                block_spot+=1024; //go through all blocks to find which one
            }
            else
            {
                break;   
            }
            
        }

        i=fil
*/
        
    //get the block and node from the rigth spot in the file struct
        uint64_t the_block = filearr[search].blockID[i];

        uint64_t the_node = filearr[search].nodeID[i];

        uint64_t the_rseq = filearr[search].rsequencenum;  //tried to get the rseq

        //uint64_t the_rseq = filearr[search].nodeID[i];

        char *getit = getSGDataBlock( the_node, the_block ); //searched cache for it 

        if (getit != NULL)  // if its in there just copy the data over  a
        {

            for (int i=0; i<1024;i++)
            {
                writebuff[i]=getit[i];
            }


        }

        else //otherwise write to it
        {

                //char tempbuff [1024];

            pktlen = SG_BASE_PACKET_SIZE;
            if ( (ret = serialize_sg_packet( sgLocalNodeId, // Local ID
                                                the_node,   // Remote ID
                                                the_block,  // Block ID
                                                SG_OBTAIN_BLOCK,  // Operation
                                                sgLocalSeqno++,    // Sender sequence number
                                                the_rseq,  // Receiver sequence number
                                                NULL, initPacket, &pktlen)) != SG_PACKT_OK ) 
            {
                logMessage( LOG_ERROR_LEVEL, "sgwrite: failed serialization of packet [%d].", ret );
                return( -1 );
            }

                    // Send the packet
            rpktlen = SG_DATA_PACKET_SIZE;
            if ( sgServicePost(initPacket, &pktlen, recvPacket, &rpktlen) ) 
            {
                logMessage( LOG_ERROR_LEVEL, "sgwrite: failed packet post" );
                return( -1 );
            }

                    // Unpack the recieived data
            if ( (ret = deserialize_sg_packet(&loc, &rem, &blkid, &op, &sloc, 
                                                &srem, writebuff, recvPacket, rpktlen)) != SG_PACKT_OK ) 
            {
                logMessage( LOG_ERROR_LEVEL, "sgwrite: failed deserialization of packet [%d]", ret );
                return( -1 );
            }

            filearr[search].rsequencenum = the_rseq;
            
            
        }

            //copy data into writebuf
        if(filearr[search].file_pointer % 512== 0 && filearr[search].file_pointer % 1024 != 0 ) //starting halfway though block
        {
            for(int i =0; i<256; i++)
            {
                writebuff[512+i]=buf[i];
                
            }

        }
        else if(filearr[search].file_pointer % 256 == 0  && (((filearr[search].file_pointer-256) % 1024)!= 0 )) //starting at last quarter 768
        {
            for(int i =0; i<256; i++)
            {
                writebuff[768+i]=buf[i];
                
            }

        }
        else //starting at second quarter 256
        {
            for(int i =0; i<256; i++)
            {   
                writebuff[256+i]=buf[i];
                
            }
        }
            
        //for (int i=0; i<512;i++)
    //    {
    //        writebuff[512+i]=buf[i];
      //  }
        pktlen = SG_DATA_PACKET_SIZE;
        if ( (ret = serialize_sg_packet( sgLocalNodeId, // Local ID
                                            the_node,   // Remote ID
                                            the_block,  // Block ID
                                            SG_UPDATE_BLOCK,  // Operation
                                            sgLocalSeqno++,    // Sender sequence number
                                            the_rseq,  // Receiver sequence number
                                            writebuff, initPacket, &pktlen)) != SG_PACKT_OK ) 
        {
            logMessage( LOG_ERROR_LEVEL, "sgwrite: failed serialization of packet [%d].", ret );
            return( -1 );
        }

                    // Send the packet
        rpktlen = SG_DATA_PACKET_SIZE;
        if ( sgServicePost(initPacket, &pktlen, recvPacket, &rpktlen) ) 
        {
            logMessage( LOG_ERROR_LEVEL, "sgwrite: failed packet post" );
            return( -1 );
        }

        char emptbuff [1024]; //empty buf just to send 

                    // Unpack the recieived data
        if ( (ret = deserialize_sg_packet(&loc, &rem, &blkid, &op, &sloc, 
                                            &srem, emptbuff, recvPacket, rpktlen)) != SG_PACKT_OK ) 
        {
            logMessage( LOG_ERROR_LEVEL, "sgwrite: failed deserialization of packet [%d]", ret );
            return( -1 );

        }
        filearr[search].rsequencenum = srem;
        
        filearr[search].file_pointer+=len;
    
        filearr[search].file_size = filearr[search].file_size + len;

        putSGDataBlock( the_node, the_block, writebuff); //add in the data 
        

        return(len);

    }
    
    
    else if(filearr[search].file_pointer == filearr[search].file_size) 
    {
        

        char initPacket[SG_DATA_PACKET_SIZE], recvPacket[SG_BASE_PACKET_SIZE];
        size_t pktlen, rpktlen;
        SG_Node_ID loc, rem;
        SG_Block_ID blkid;
        SG_SeqNum sloc, srem;
        SG_System_OP op;
        SG_Packet_Status ret;

        
        // Setup the packet
        pktlen = SG_DATA_PACKET_SIZE;
        if ( (ret = serialize_sg_packet( sgLocalNodeId, // Local ID
                                        SG_NODE_UNKNOWN,   // Remote ID
                                        SG_BLOCK_UNKNOWN,  // Block ID
                                        SG_CREATE_BLOCK,  // Operation
                                        sgLocalSeqno++,    // Sender sequence number
                                        SG_SEQNO_UNKNOWN,  // Receiver sequence number
                                        tempbuff, initPacket, &pktlen)) != SG_PACKT_OK ) {
            logMessage( LOG_ERROR_LEVEL, "sgwrite: failed serialization of packet [%d].", ret );
            return( -1 );
        }

        // Send the packet
        rpktlen = SG_BASE_PACKET_SIZE;
        if ( sgServicePost(initPacket, &pktlen, recvPacket, &rpktlen) ) {
            logMessage( LOG_ERROR_LEVEL, "sgwrite: failed packet post" );
            return( -1 );
        }

        char chararr [1024];

        // Unpack the recieived data
        if ( (ret = deserialize_sg_packet(&loc, &rem, &blkid, &op, &sloc, 
                                        &srem, chararr, recvPacket, rpktlen)) != SG_PACKT_OK ) {
            logMessage( LOG_ERROR_LEVEL, "sgwrite: failed deserialization of packet [%d]", ret );
            return( -1 );

        }
        

        //get the int i 
        int i = filearr[search].file_size/1024;
//use it to get the block id, node id
        filearr[search].blockID[i]=blkid;
        filearr[search].nodeID[i]=rem;
        filearr[search].rsequencenum = srem;
        //map to node id

        
        

        if(filearr[search].file_pointer == filearr[search].file_size )
        {
            filearr[search].file_size = filearr[search].file_size + len;
        }

//set file pointer to file size
        filearr[search].file_pointer+=len;

        putSGDataBlock( blkid, rem, tempbuff); //add data in 

        return(len);


    }

    else //fp is less than file size and its divisible by 1024 (begining of block)
    {
        
        char writebuff [1024];
    //take in base packet size and return data packet size

        char initPacket[SG_BASE_PACKET_SIZE], recvPacket[SG_DATA_PACKET_SIZE];
        size_t pktlen, rpktlen;
        SG_Node_ID loc, rem;
        SG_Block_ID blkid;
        SG_SeqNum sloc, srem;
        SG_System_OP op;
        SG_Packet_Status ret;

        
    //makes 2 ints to search the file
        //int block_spot=0;
        int i=filearr[search].file_pointer/1024;

        
    //get the block and node from the rigth spot in the file struct
        uint64_t the_block = filearr[search].blockID[i];

        uint64_t the_node = filearr[search].nodeID[i];

        uint16_t the_rseq = filearr[search].rsequencenum ;

        //char tempbuff [1024];

        char *getit = getSGDataBlock( the_node, the_block ); //get if its in the cache

        if (getit != NULL)
        {

            for (int i=0; i<256;i++)
            {
                writebuff[i]=getit[i]; //copy data into writebuff for first quarter
            }


        }

        else
        {

            pktlen = SG_BASE_PACKET_SIZE;
            if ( (ret = serialize_sg_packet( sgLocalNodeId, // Local ID
                                                the_node,   // Remote ID
                                                the_block,  // Block ID
                                                SG_OBTAIN_BLOCK,  // Operation
                                                sgLocalSeqno++,    // Sender sequence number
                                                the_rseq,  // Receiver sequence number
                                                NULL, initPacket, &pktlen)) != SG_PACKT_OK ) 
            {
                logMessage( LOG_ERROR_LEVEL, "sgwrite: failed serialization of packet [%d].", ret );
                return( -1 );
            }

           
                // Send the packet
            rpktlen = SG_DATA_PACKET_SIZE;
            if ( sgServicePost(initPacket, &pktlen, recvPacket, &rpktlen) ) 
            {
                logMessage( LOG_ERROR_LEVEL, "sgwrite: failed packet post" );
                return( -1 );
            }

                // Unpack the recieived data
            if ( (ret = deserialize_sg_packet(&loc, &rem, &blkid, &op, &sloc, 
                                                &srem, writebuff, recvPacket, rpktlen)) != SG_PACKT_OK ) 
            {
                logMessage( LOG_ERROR_LEVEL, "sgwrite: failed deserialization of packet [%d]", ret );
                return( -1 );

            }
            filearr[search].rsequencenum = srem;
        }
    //is null put data in

        for (int i=0; i<256;i++)
        {
            writebuff[i]=buf[i];
        }

        pktlen = SG_DATA_PACKET_SIZE;

        if ( (ret = serialize_sg_packet( sgLocalNodeId, // Local ID
                                            the_node,   // Remote ID
                                            the_block,  // Block ID
                                            SG_UPDATE_BLOCK,  // Operation
                                            sgLocalSeqno++,    // Sender sequence number
                                            the_rseq,  // Receiver sequence number
                                            writebuff, initPacket, &pktlen)) != SG_PACKT_OK ) 
        {
            logMessage( LOG_ERROR_LEVEL, "sgwrite: failed serialization of packet [%d].", ret );
            return( -1 );
        }

            // Send the packet
        rpktlen = SG_DATA_PACKET_SIZE;
        if ( sgServicePost(initPacket, &pktlen, recvPacket, &rpktlen) ) 
        {
            logMessage( LOG_ERROR_LEVEL, "sgwrite: failed packet post" );
            return( -1 );
        }

        char emptbuff [1024];

            // Unpack the recieived data
        if ( (ret = deserialize_sg_packet(&loc, &rem, &blkid, &op, &sloc, 
                                            &srem, emptbuff, recvPacket, rpktlen)) != SG_PACKT_OK ) 
        {
            logMessage( LOG_ERROR_LEVEL, "sgwrite: failed deserialization of packet [%d]", ret );
            return( -1 );

        }
        filearr[search].rsequencenum = srem; //set rseq
        filearr[search].file_pointer+=len;


        //filearr[search].file_size = filearr[search].file_size + len;
        putSGDataBlock( the_node, the_block, writebuff); //add data in 

        return(len);

    }


        // Log the write, return bytes written (len)

    return( len );
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : sgseek
// Description  : Seek to a specific place in the file
//
// Inputs       : fh - the file handle of the file to seek in
//                off - offset within the file to seek to
// Outputs      : new position if successful, -1 if failure

int sgseek(SgFHandle fh, size_t off) 
{

    int search = struct_search(fh);  //if file handle exists
    if(search == -1)  //if it doesnt error
    {
        return -1;
    }
//makes sure fh exists and file is open
    if(filearr[search].file_handle != fh || filearr[search].is_open==0)
    {
        return (-1);
    }
//if the position is less than or equal to file size
    if(off<=filearr[search].file_size)
    {
        filearr[search].file_pointer = off;   //set file pointer to off
    }
    else //otherwise error
    {
        return (-1);
    }

    // Return new position
    return( off );
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : sgclose
// Description  : Close the file
//
// Inputs       : fh - the file handle of the file to close
// Outputs      : 0 if successful test, -1 if failure

int sgclose(SgFHandle fh) 
{
    int search = struct_search(fh); //if file handle exists
    if(search == -1)
    {
        return -1;
    }
//if filehandle doesnt exist or the file is closed then error
    if(filearr[search].file_handle != fh || filearr[search].is_open==0)
    {
        return (-1);
    }
// close file
//set file pointer to 0
    filearr[search].is_open = 0;
    filearr[search].file_pointer =0;

    closeSGCache(); //close the cache which prints hits and misses 



    // Return successfully
    return( 0 );
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : sgshutdown
// Description  : Shut down the filesystem
//
// Inputs       : none
// Outputs      : 0 if successful test, -1 if failure

int sgshutdown(void) 
{


    char initPacket[SG_BASE_PACKET_SIZE], recvPacket[SG_BASE_PACKET_SIZE];
    size_t pktlen, rpktlen;
    SG_Node_ID loc, rem;
    SG_Block_ID blkid;
    SG_SeqNum sloc, srem;
    SG_System_OP op;
    SG_Packet_Status ret;

    // Local and do some initial setup
    logMessage( LOG_INFO_LEVEL, "Shut Down Local Endpoint ..." );
    sgLocalSeqno = SG_INITIAL_SEQNO;

    // Setup the packet
    pktlen = SG_BASE_PACKET_SIZE;
    if ( (ret = serialize_sg_packet( sgLocalNodeId, // Local ID
                                    SG_NODE_UNKNOWN,   // Remote ID
                                    SG_BLOCK_UNKNOWN,  // Block ID
                                    SG_STOP_ENDPOINT,  // Operation
                                    sgLocalSeqno++,    // Sender sequence number
                                    SG_SEQNO_UNKNOWN,  // Receiver sequence number
                                    NULL, initPacket, &pktlen)) != SG_PACKT_OK ) {
        logMessage( LOG_ERROR_LEVEL, "sgshutdown: failed serialization of packet [%d].", ret );
        return( -1 );
    }

    // Send the packet
    rpktlen = SG_BASE_PACKET_SIZE;
    if ( sgServicePost(initPacket, &pktlen, recvPacket, &rpktlen) ) {
        logMessage( LOG_ERROR_LEVEL, "sgshutdown: failed packet post" );
        return( -1 );
    }

    // Unpack the recieived data
    if ( (ret = deserialize_sg_packet(&loc, &rem, &blkid, &op, &sloc, 
                                    &srem, NULL, recvPacket, rpktlen)) != SG_PACKT_OK ) {
        logMessage( LOG_ERROR_LEVEL, "sgshutdown: failed deserialization of packet [%d]", ret );
        return( -1 );
    }

    // Sanity check the return value
    if ( loc == SG_NODE_UNKNOWN ) {
        logMessage( LOG_ERROR_LEVEL, "sgshutdown: bad local ID returned [%ul]", loc );
        return( -1 );
    }

    // Set the local node ID, log and return successfully
   
    // Log, return successfully
    logMessage( LOG_INFO_LEVEL, "Shut down Scatter/Gather driver." );


    return( 0 );
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : unpack
// Description  : Copies the data from the packet and puts it into the variables
//
// Inputs       : loc - the local node identifier
//                rem - the remote node identifier
//                blk - the block identifier
//                op - the operation performed/to be performed on block
//                sseq - the sender sequence number
//                rseq - the receiver sequence number
//                data - the data block (of size SG_BLOCK_SIZE) or NULL
//                packet - the buffer to place the data
//                plen - the packet length (int bytes)
// Outputs      : returns length of the packet after putting all the bytes in (int)
int unpack(char **loc, char **rem, char **blk, 
        char **op, char **sseq, char **rseq, char **data, 
        char *packet, size_t plen)

{
    int pack_count = 4; //count starts after first 4 of magic

    for(int i =0; i<8; i++) 
    {
        (*loc)[i] = packet[pack_count]; //assigns loc values
        pack_count++;
    }
    for(int i =0; i<8; i++)
    {
        (*rem)[i] = packet[pack_count]; //assigns rem values
        pack_count++;
    }
    for(int i =0; i<8; i++)
    {
        (*blk)[i] = packet[pack_count];  //assigns blk values
        pack_count++; 
    }
    for(int i =0; i<4; i++)
    {
        (*op)[i] = packet[pack_count]; //assigns op values
        pack_count++;
    }
    for(int i =0; i<2; i++)
    {
        (*sseq)[i] = packet[pack_count]; //assigns sseq values
        pack_count++;
    }
    for(int i =0; i<2; i++)
    {
        (*rseq)[i] = packet[pack_count]; //assigns rseq values
        pack_count++;
    }

    if (packet[pack_count] == 0)  //will be either 0 or 1024
    {
        
        pack_count++; //if 0 just put that in 
    }
    else
    {
        pack_count++;
        for(int i =0;i<1024;i++)  //if 1024 then loop throught to assign data all 1024
        {
            (*data)[i]=packet[pack_count];
            pack_count++;
        }
    }

    return(pack_count); 
  
}
////////////////////////////////////////////////////////////////////////////////
//
// Function     : byte_block
// Description  : Creates a packet by putting all variable values in 
//
// Inputs       : magic - this is a special number placed in memory to detect when a packet has been corrupted
//                loc - the local node identifier
//                rem - the remote node identifier
//                blk - the block identifier
//                op - the operation performed/to be performed on block
//                sseq - the sender sequence number
//                rseq - the receiver sequence number
//                data - the data block (of size SG_BLOCK_SIZE) or NULL
//                packet - the buffer to place the data
// Outputs      : returns packet length (int)
int byte_block(uint32_t magic, SG_Node_ID loc, SG_Node_ID rem, SG_Block_ID blk, 
                SG_System_OP op, SG_SeqNum sseq, SG_SeqNum rseq,char *data, char **packet)
{

    int pack_count =0;  //keeps count of how logn the packet is

    for(int i =0; i<4; i++)
    {
        (*packet)[pack_count] = ((char *) &magic)[i];  //puts magic into packet
        pack_count++;
    }

    for(int i =0; i<8; i++)
    {
        (*packet)[pack_count] = ((char *) &loc)[i];  //puts loc into packet
        pack_count++;
    }

    for(int i =0; i<8; i++)
    {
        (*packet)[pack_count] = ((char *) &rem)[i];  //puts rem into packet
        pack_count++;
    }

    for(int i =0; i<8; i++)
    {
        (*packet)[pack_count] = ((char *) &blk)[i];  //puts blk into packet
        pack_count++;
    }
    for(int i =0; i<4; i++)
    {
        (*packet)[pack_count] = ((char *) &op)[i];   //puts op into packet
        pack_count++;
    }
    for(int i =0; i<2; i++)
    {
        (*packet)[pack_count] = ((char *) &sseq)[i];  //puts sseq into packet
        pack_count++;
    }
    for(int i =0; i<2; i++)
    {
        (*packet)[pack_count] = ((char *) &rseq)[i];   //puts rseq into packet
        pack_count++;
    }

    if (data == NULL)   //if data is null then that spot will have one space and be 0
    {
        (*packet)[pack_count] = 0;
        pack_count++;
    }
    else  //otherwise it will be 1024 and has to loop through to put it into the packet
    {
        (*packet)[pack_count] = 1;
        pack_count++;
        for(int i =0;i<1024;i++)
        {
            (*packet)[pack_count] = data[i];
            pack_count++;
        }
    }
    for(int i =0; i<4; i++)  //add magic to the end
    {
        (*packet)[pack_count] = ((char *) &magic)[i];
        pack_count++;
    }

    return((size_t)pack_count);  //return packet length

}
////////////////////////////////////////////////////////////////////////////////
//
// Function     : deser_validate
// Description  : checks that the values from deserialize are valid
//
// Inputs       : 
//                loc - the local node identifier
//                rem - the remote node identifier
//                blk - the block identifier
//                op - the operation performed/to be performed on block
//                sseq - the sender sequence number
//                rseq - the receiver sequence number
//                data - the data block (of size SG_BLOCK_SIZE) or NULL
//                packet - the buffer to place the data
//                plen - the packet length (int bytes)
// Outputs      : returns error number or 0 if there is no error

SG_Packet_Status deser_validate(SG_Node_ID *loc, SG_Node_ID *rem, SG_Block_ID *blk, 
        SG_System_OP *op, SG_SeqNum *sseq, SG_SeqNum *rseq, char *data, char *packet, size_t plen)
{


    if(*loc==0)     //All non-zero values are valid.
        return (1);
    if(*rem==0)     //All non-zero values are valid.
        return (2);
    if(*blk==0)     //All non-zero values are valid.
        return (3);
    if(*op<0 || *op>6)  //must be between 0-6
        return(4);
    if(*sseq==0)    //All non-zero values are valid.
        return(5);
    if(*rseq==0)    //All non-zero values are valid.
        return(6);
    //if(*data == NULL)
    //  return(7);
    if(plen !=41 && plen!=1065)     //must be 41 or 1065
        return (8);
    if(packet == NULL) //cant be null
        return(9);
    
    //otherwise return 0
    return(0);

}
////////////////////////////////////////////////////////////////////////////////
//
// Function     : ser_validate
// Description  : checks that the values from serialize are valid
//
// Inputs       : 
//                loc - the local node identifier
//                rem - the remote node identifier
//                blk - the block identifier
//                op - the operation performed/to be performed on block
//                sseq - the sender sequence number
//                rseq - the receiver sequence number
//                data - the data block (of size SG_BLOCK_SIZE) or NULL
// Outputs      : returns error number or 0 if there is no error
SG_Packet_Status ser_validate(SG_Node_ID loc, SG_Node_ID rem, SG_Block_ID blk, 
        SG_System_OP op, SG_SeqNum sseq, SG_SeqNum rseq, char *data)
{

    if(loc==0)      //All non-zero values are valid
        return (1);
    if(rem==0)      //All non-zero values are valid
        return (2);
    if(blk==0)      //All non-zero values are valid
        return (3);
    if(op<0 || op>6)        //must be between 0-6
        return(4);
    if(sseq==0)         //All non-zero values are valid 
        return(5);
    if(rseq==0)     //All non-zero values are valid
        return(6);
//  if(data == NULL)
//      return(7);
    return(0);

}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : serialize_sg_packet
// Description  : Serialize a ScatterGather packet (create packet)
//
// Inputs       : loc - the local node identifier
//                rem - the remote node identifier
//                blk - the block identifier
//                op - the operation performed/to be performed on block
//                sseq - the sender sequence number
//                rseq - the receiver sequence number
//                data - the data block (of size SG_BLOCK_SIZE) or NULL
//                packet - the buffer to place the data
//                plen - the packet length (int bytes)
// Outputs      : 0 if successfully created, -1 if failure

SG_Packet_Status serialize_sg_packet( SG_Node_ID loc, SG_Node_ID rem, SG_Block_ID blk, 
        SG_System_OP op, SG_SeqNum sseq, SG_SeqNum rseq, char *data, 
        char *packet, size_t *plen ) 
{
    //validate the values
    
    SG_Packet_Status error = ser_validate(loc,rem, blk,op,sseq,rseq,data); 

    if(error!=0) //if there is an error then return it
    {
        return(error);
    }

    //set magic value to send into the packet
    uint32_t magic = SG_MAGIC_VALUE;

    //call byte_block to create packet
    *plen = byte_block(magic, loc,rem,blk,op,sseq,rseq, data, &packet);




    return(0); 
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : deserialize_sg_packet
// Description  : De-serialize a ScatterGather packet (unpack packet)
//
// Inputs       : loc - the local node identifier
//                rem - the remote node identifier
//                blk - the block identifier
//                op - the operation performed/to be performed on block
//                sseq - the sender sequence number
//                rseq - the receiver sequence number
//                data - the data block (of size SG_BLOCK_SIZE) or NULL
//                packet - the buffer to place the data
//                plen - the packet length (int bytes)
// Outputs      : 0 if successfully created, -1 if failure

SG_Packet_Status deserialize_sg_packet( SG_Node_ID *loc, SG_Node_ID *rem, SG_Block_ID *blk, SG_System_OP *op, SG_SeqNum *sseq, SG_SeqNum *rseq, char *data, char *packet, size_t plen ) 
{
    
    //make magic variable
    uint32_t magic;

    char *m = (char *)&magic;
    
    //unpack packet for magic 
    for(int i =0; i<4; i++)
    {
        (m)[i] = packet[i];
        
    }
    //check if error with magic (bad packet data)
    if(magic != SG_MAGIC_VALUE)
    {
        return(9);
    }
    
    //call unpack to unpack the packet
    int unpacked_count = unpack((char **)&loc, (char **)&rem, (char **)&blk, (char **)&op, (char **)&sseq, (char **)&rseq, &data, packet, plen);

    //unpack packet for magic 
    for(int i =0; i<4; i++)
    {
        (m)[i] = packet[unpacked_count];
        unpacked_count++;
    }
    //check if error with magic (bad packet data)
    if(magic != SG_MAGIC_VALUE)
    {
        return(9);
    }

    //call deser_validate to validate the values
    SG_Packet_Status error =deser_validate( loc, rem, blk, op, sseq, rseq,data, packet, plen);

    if(error!=0)//if there is an error then return it
    {
        return(error);
    }


    
    return(0);
}
// Driver support functions

////////////////////////////////////////////////////////////////////////////////
//
// Function     : sgInitEndpoint
// Description  : Initialize the endpoint
//
// Inputs       : none
// Outputs      : 0 if successfull, -1 if failure

int sgInitEndpoint( void ) {

    // Local variables
    char initPacket[SG_BASE_PACKET_SIZE], recvPacket[SG_BASE_PACKET_SIZE];
    size_t pktlen, rpktlen;
    SG_Node_ID loc, rem;
    SG_Block_ID blkid;
    SG_SeqNum sloc, srem;
    SG_System_OP op;
    SG_Packet_Status ret;

    // Local and do some initial setup
    logMessage( LOG_INFO_LEVEL, "Initializing local endpoint ..." );
    sgLocalSeqno = SG_INITIAL_SEQNO;  //sseq


    // Setup the packet
    pktlen = SG_BASE_PACKET_SIZE;
    if ( (ret = serialize_sg_packet( SG_NODE_UNKNOWN, // Local ID
                                    SG_NODE_UNKNOWN,   // Remote ID
                                    SG_BLOCK_UNKNOWN,  // Block ID
                                    SG_INIT_ENDPOINT,  // Operation
                                    sgLocalSeqno++,    // Sender sequence number
                                    SG_SEQNO_UNKNOWN,  // Receiver sequence number
                                    NULL, initPacket, &pktlen)) != SG_PACKT_OK ) {
        logMessage( LOG_ERROR_LEVEL, "sgInitEndpoint: failed serialization of packet [%d].", ret );
        return( -1 );
    }

    // Send the packet
    rpktlen = SG_BASE_PACKET_SIZE;
    if ( sgServicePost(initPacket, &pktlen, recvPacket, &rpktlen) ) {
        logMessage( LOG_ERROR_LEVEL, "sgInitEndpoint: failed packet post" );
        return( -1 );
    }

    // Unpack the recieived data
    if ( (ret = deserialize_sg_packet(&loc, &rem, &blkid, &op, &sloc, 
                                    &srem, NULL, recvPacket, rpktlen)) != SG_PACKT_OK ) {
        logMessage( LOG_ERROR_LEVEL, "sgInitEndpoint: failed deserialization of packet [%d]", ret );
        return( -1 );
    }



    // Sanity check the return value
    if ( loc == SG_NODE_UNKNOWN ) {
        logMessage( LOG_ERROR_LEVEL, "sgInitEndpoint: bad local ID returned [%ul]", loc );
        return( -1 );
    }

    // Set the local node ID, log and return successfully
    sgLocalNodeId = loc;
   
    logMessage( LOG_INFO_LEVEL, "Completed initialization of node (local node ID %lu", sgLocalNodeId );
    return( 0 );
}
