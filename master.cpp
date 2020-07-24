#include "BufferStructure.h"
#include "getblk.h"
#include "brelse.h"
#include <thread>
#include <mutex>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define NUMBER_OF_QUEUES 4
#define NUMBER_OF_BUFFERS 4
#define NUMBER_OF_MAIN_MEMORY 6
#define NUMBER_OF_THREADS 6

using namespace std;

HASH_QUEUE **hashQueue;
FREE_LIST *freeList;

int i = 0 ;
void processDef()
{
    int processID = i++ ;
    int operationType, blockNumber;
    
    blockNumber = rand() % NUMBER_OF_MAIN_MEMORY;
    operationType = rand() % 2;    
    
    lock();
    cout << "process["<<processID<<"]: "<<" Process " << processID <<" requested buffer "<<blockNumber<< endl;
    unlock();

    BUFFER* block = getblk(processID, blockNumber,NUMBER_OF_QUEUES, hashQueue, freeList);
    
    lock();
    cout << "process["<<processID<<"]: "<<" Process " << processID <<" got buffer "<<block->blockNumber << endl;
    cout << "\n-------------------------------AFTER GETBLK[process:"<<processID<<"]-------------------------------"<< endl;
    displayStucture(NUMBER_OF_QUEUES, hashQueue, freeList);
    unlock();
    if (operationType == 0)
    {
        lock();
        cout << "process["<<processID<<"]: "<< " Operation type : READ"<< endl;
        cout << "process["<<processID<<"]: "<< " Reading content " << endl;
        unlock();
        this_thread::sleep_for(chrono::seconds(1));
    }
    else
    {
        lock();
        cout << "process["<<processID<<"]: "<< " Operation type : WRITE"<< endl;
        cout << "process["<<processID<<"]: "<< " Writing content " << endl;
        unlock();
        this_thread::sleep_for(chrono::seconds(1));
        lock();
        block->status[1] = rand()%2 ;
        if(!block->status[1]){
        cout << "process["<<processID<<"]: "<< " Data got corrupted!" << endl;
        }
        block->status[2] = true;
        unlock();
    }
    brelse(processID, blockNumber,NUMBER_OF_QUEUES, hashQueue, freeList);
    lock();
    cout << "\n---------------------------------AFTER BRELSE[process:"<<processID<<"]----------------------------------" << endl;
    displayStucture(NUMBER_OF_QUEUES, hashQueue, freeList);
    cout << "\n*********************************************************************\n";
    unlock();
}

int main()
{
    hashQueue = new HASH_QUEUE*[NUMBER_OF_QUEUES];
    for(int i = 0; i<NUMBER_OF_QUEUES; i++) 
        hashQueue[i] = new HASH_QUEUE();
    
    freeList = new FREE_LIST();
    for(int i = 0; i<NUMBER_OF_BUFFERS; i++)
    {
        BUFFER* newBufferNode = new BUFFER();
        newBufferNode->blockNumber = i;
        freeList->FLInsertTail(newBufferNode);
        hashQueue[i % NUMBER_OF_QUEUES]->HQInsertTail(newBufferNode);
    }

    cout << "----------------------------INITIAL STATUS----------------------------\n";
    displayStucture(NUMBER_OF_QUEUES, hashQueue, freeList);
    cout << "----------------------------------------------------------------------\n";

    srand(time(NULL));
    thread p_threads[NUMBER_OF_THREADS];
    for(int i=0; i<NUMBER_OF_THREADS; i++)
        p_threads[i] =  thread(processDef);


    for(int i=0; i<NUMBER_OF_THREADS; i++)
        p_threads[i].join();

    cout << "----------------------------FINAL STATUS----------------------------\n";
    displayStucture(NUMBER_OF_QUEUES, hashQueue, freeList);
    cout << "----------------------------------------------------------------------\n";

    cout<<endl;
    return 0;
}
