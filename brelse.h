#ifndef B_RELSE
#define B_RELSE
#include "BufferStructure.h"
#include "getblk.h"

using namespace std;

void notify(int blockNumber) {
    bool isNotified = false;
    if(waitUniqueListCount > 0){
        for (int i = 0; i<waitUniqueListCount; i++){
            if (blockNumber == WaitUniqueList[i].blockNumber){
                WaitUniqueList[i].CV->notify_one();

                cout<<"\nNotify Unique process "<<WaitUniqueList[i].processID<<" for block "<<blockNumber<<endl;

                waitUniqueListCount--;
                isNotified = true;
                for (int j = i; j<waitUniqueListCount; j++){
                    WaitUniqueList[j] = WaitUniqueList[j + 1];
                }
                break;
            }
        }
    }

    
    if(waitAnyListCount > 0 && !isNotified){
        
        WaitAnyList[0].CV->notify_one();

        cout<<"\nNotify Any process "<<WaitAnyList[0].processID<<" for block "<<blockNumber<<endl;
        waitAnyListCount--;
        
        for (int i = 0; i<waitAnyListCount; i++){
            WaitAnyList[i] = WaitAnyList[i + 1];
        }
       
    }
}



void brelse(int processID, int blockNumber, int NUMBER_OF_QUEUES, HASH_QUEUE **hashQueue,FREE_LIST *freeList)
{

    lock();
    cout << "=====>[process["<<processID<<"]: Executing BRELSE for block "<<blockNumber<<endl;
    unlock();
    int index = hashIndex(blockNumber, NUMBER_OF_QUEUES);
    lock();
    BUFFER* buffer = hashQueue[index]->HQFind(blockNumber);
    unlock();
    
    if(buffer->status[1])
    {
        lock();
        freeList->FLInsertTail(buffer);
        cout << "\tprocess["<<processID<<"]:brelse: Enqueue buffer at end of free list.\n";
        unlock();
    }
    else
    {
        lock();
        freeList->FLInsertHead(buffer);
        cout << "\tprocess["<<processID<<"]:brelse: Enqueue buffer at beginning of free list.\n";
        unlock();
    }

    lock();
    buffer->status[0] = true;
    notify(blockNumber);
    unlock();
}

#endif
