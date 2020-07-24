#ifndef GET_BLOCK
#define GET_BLOCK

#include <chrono>
#include <iostream>
#include <unistd.h>
#include <thread>
#include <mutex>
#include<condition_variable>
#include "BufferStructure.h"




using namespace std;

struct WaitAny{
condition_variable *CV;
int processID;
};

struct WaitUnique{
condition_variable *CV;
int blockNumber;
int processID;
};

WaitUnique WaitUniqueList[10];
WaitAny WaitAnyList[10];

int cvCount = 0;
int waitAnyListCount = 0;
int waitUniqueListCount = 0;
mutex myLock,anyBufferLock,uniqueBufferLock;

void lock()   { myLock.lock(); }
void unlock() { myLock.unlock(); }


void delayedWrite(BUFFER* delayedBuffer, FREE_LIST* freeList, int NUMBER_OF_QUEUES,HASH_QUEUE **hashQueue )
{
    lock();
    cout<<"\nKERNEL: Asynchronous Write(DelayedWrite) of Buffer Data Initiated for BlockNumber "<<delayedBuffer->blockNumber<<endl;
   
   delayedBuffer->status[2] = false;
   delayedBuffer->status[0] = true;
   freeList->FLInsertHead(delayedBuffer); 
    
    cout<<"nKERNEL: Buffer "<<delayedBuffer->blockNumber<<" marked for DelayedWrite added to Head of FreeList Asynchronously...";
    unlock();
    
    lock();
    cout << "\n---------------------------------After Delayed write[KERNEL]----------------------------------" << endl;
    displayStucture(NUMBER_OF_QUEUES, hashQueue, freeList);
    cout << "\n*********************************************************************\n";
    unlock();
  
    lock();
    bool isNotified = false;
    if(waitUniqueListCount > 0){
        for (int i = 0; i<waitUniqueListCount; i++){
            if (delayedBuffer->blockNumber == WaitUniqueList[i].blockNumber){
                WaitUniqueList[i].CV->notify_one();

                cout<<"\nNotify Unique process "<<WaitUniqueList[i].processID<<" for block "<<delayedBuffer->blockNumber<<endl;

                waitUniqueListCount--;
                isNotified = true;
                for (int j = i; i<waitUniqueListCount; j++){
                    WaitUniqueList[j] = WaitUniqueList[j + 1];
                }
                break;
            }
        }
        
    }

    
    if(waitAnyListCount > 0 && !isNotified){
        
        WaitAnyList[0].CV->notify_one();

        cout<<"\nNotify Any process for block "<<delayedBuffer->blockNumber<<endl;

        for (int i = 0; i<waitAnyListCount; i++){
            WaitAnyList[i] = WaitAnyList[i + 1];
        }
        waitAnyListCount--;
    }

unlock();

}







BUFFER* getblk(int processID, int blockNumber, int NUMBER_OF_QUEUES, HASH_QUEUE **hashQueue,FREE_LIST *freeList)
{
    lock();
    cout << "=====>process["<<processID<<"]: Executing GETBLK" << endl;
    unlock();
    BUFFER* buffer = NULL;
    int index = hashIndex(blockNumber, NUMBER_OF_QUEUES);
    int FLBlockNumber, FLIndex;

    BUFFER* HQBuffer;
    BUFFER* FLBuffer;

    while (buffer == NULL)
    {
        lock();
        HQBuffer = hashQueue[index]->HQFind(blockNumber);
        unlock();
        if(HQBuffer != NULL)                        
        {
            if(HQBuffer->status[0] == false)             
            {
                lock();
                cout << "\tprocess["<<processID<<"]:getblk: Scenario 5 has been encountered. Block was found on the hash queue, but its buffer is currently busy." << endl;
                unlock();

                unique_lock<std::mutex> UniqueBufferLock(uniqueBufferLock);
              
                condition_variable CV;
               
                struct WaitUnique wu;
                wu.blockNumber= blockNumber;
                wu.processID=processID;
                wu.CV = &CV;

                lock();
                WaitUniqueList[waitUniqueListCount++] = wu;
                cout << "WaitUniqueList["<<waitUniqueListCount<<"] = ";
                for(int i=0; i<waitUniqueListCount;i++){
                    cout << WaitUniqueList[i].processID << ", ";
                }
                cout<<endl;
                unlock();
                CV.wait(UniqueBufferLock);

                continue;
            }
            
            lock();
            cout << "\tprocess["<<processID<<"]:getblk: Scenario 1 has been encountered. Block is found on its hash queue and its buffer is free." << endl;
            HQBuffer->status[0] = false;
		//	if(HQBuffer->status[2]==true){
		//		cout<<"Performing Synchronous write: ";
		//		HQBuffer->status[2] = false;
		//	}                
            buffer = HQBuffer;
            freeList->FLRemove(blockNumber); 
			 
            unlock();
        }
        else                                       
        {
            if(freeList->isEmpty())                 
            {
                lock();
                cout << "\tprocess["<<processID<<"]:getblk: Scenario 4 has been encountered. Block could not be found on the hash queue and the free list of buffers is empty." << endl;
                unlock();

                unique_lock<std::mutex> AnyBufferLock(anyBufferLock);
               
                condition_variable CV;
                struct WaitAny wa;
                wa.processID=processID;
                wa.CV = &CV;

                lock();
                WaitAnyList[waitAnyListCount++] = wa;
                cout << "waitAnyList["<<waitAnyListCount<<"] = ";
                for(int i=0; i<waitAnyListCount;i++){
                    cout << WaitAnyList[i].processID<<", ";
                }
                cout<<endl;
                unlock();
                CV.wait(AnyBufferLock);
               
                continue;
            }
            lock();
            FLBuffer = freeList->FLRemoveHead();
            unlock();
            if(FLBuffer->status[2])               
            {

                lock();
                cout << "\tprocess["<<processID<<"]:getblk: Scenario 3 has been encountered with block "<<FLBuffer->blockNumber<<". Block could not be found on the hash queue, and when allocating a buffer from free list, a buffer marked \"delayed write\" is allocated. Then the kernel must write the \"delayed write\" buffer to disk and allocate another buffer." << endl;
                FLBuffer->status[0] = false;
                cout << "\tprocess["<<processID<<"]:getblk: Performing asynchronous writing." << endl;
                std::thread temp(delayedWrite,FLBuffer,freeList,NUMBER_OF_QUEUES,hashQueue);
                temp.detach();     				//simulating kernel
                
                
                FLBuffer->status[2] = false;
                unlock();
                continue;
            }
            lock();
            buffer = FLBuffer;
            FLBlockNumber = FLBuffer->blockNumber;
            FLIndex = hashIndex(FLBlockNumber, NUMBER_OF_QUEUES);
            hashQueue[FLIndex]->HQRemoveNode(FLBlockNumber);   
            buffer->status[0] = false;
            buffer->status[1] = true;
            buffer->status[2] = false;
            buffer->blockNumber = blockNumber;
            cout << "\tprocess["<<processID<<"]:getblk: Scenario 2 has been encountered. Block could not be found on the hash queue, so a buffer from the free list is allocated." << endl;
            hashQueue[index]->HQInsertTail(FLBuffer);
            unlock();
        }
    }
    return buffer;
}

#endif
