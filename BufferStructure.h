#ifndef BUFFER_STRUCTURE
#define BUFFER_STRUCTURE

#include <iostream>
#include <string>

using namespace std;



struct BUFFER
{
    public:
        int blockNumber;
        bool status[3];     //free - valid - delayedWrite
        struct BUFFER* HQNextBuffer;
        struct BUFFER* HQPrevBuffer;
        struct BUFFER* FLNextBuffer;
        struct BUFFER* FLPrevBuffer;
    
        BUFFER()
        {
            status[0] = status[1] = true;
            status[2] = false;
            HQNextBuffer = HQPrevBuffer = FLNextBuffer = FLPrevBuffer = NULL;
        }
};


class FREE_LIST
{
    private:
        BUFFER* head;
        BUFFER* tail;

    public:
        bool isEmpty() { return (head==NULL); }

        void FLInsertHead(BUFFER* item)
        {
            if(isEmpty())
            {
                item->FLNextBuffer = item->FLPrevBuffer = item;
                head = tail = item;
            }
            else
            {
                item->FLNextBuffer = head;
                item->FLPrevBuffer = tail;
                tail->FLNextBuffer = head->FLPrevBuffer = item;
                head = item;
            }
        }

        void FLInsertTail(BUFFER* item)
        {
            if(isEmpty())
            {
                item->FLNextBuffer = item->FLPrevBuffer = item;
                head = tail = item;
            }
            else
            {
                item->FLPrevBuffer = tail;
                item->FLNextBuffer = head;
                head->FLPrevBuffer = tail->FLNextBuffer = item;
                tail = tail->FLNextBuffer;
            }
        }

        BUFFER* FLRemoveHead()
        {
            BUFFER* buffer = NULL;
            if(!isEmpty())
            {
                if(head == head->FLNextBuffer)		// when list contains only one element
                {
                    buffer = head;
                    head = tail = NULL;
                }
                else                                   //when list contains more than one element
                {
                    tail->FLNextBuffer = head->FLNextBuffer;
                    head->FLNextBuffer->FLPrevBuffer = head->FLPrevBuffer;
                    buffer = head;
                    head = head->FLNextBuffer;
                }
            }
            return buffer;
        }
        
        BUFFER* FLRemoveTail()
        {
            BUFFER* buffer = NULL;
            if(!isEmpty())
            {
                if(head == head->FLNextBuffer)		
                {
                    buffer = head;
                    head = tail = NULL;
                }
                else
                {
                    tail->FLNextBuffer = head->FLNextBuffer;
                    head->FLNextBuffer->FLPrevBuffer = head->FLPrevBuffer;
                    buffer = head;
                    head = head->FLNextBuffer;
                }
            }
            return buffer;
        }


        BUFFER* FLFind(int blockNumber)
        {
            BUFFER* buffer = NULL;
            if (!isEmpty())
            {
                BUFFER* node = head;
                do
                {
                    if (node->blockNumber == blockNumber)
                    {
                        buffer = node;
                        break;
                    }
                    node = node->FLNextBuffer;
                } while (node != head);
            }
            return buffer;
        }

        
        void FLRemove(int blockNumber)
        {
            BUFFER* node = FLFind(blockNumber);
            if(node->FLNextBuffer == node) 		// when list contains only one element
                head = tail = NULL;
            else
            {
                if(node == head) head = head->FLNextBuffer;
                else if(node == tail) tail = tail->FLPrevBuffer;
                node->FLPrevBuffer->FLNextBuffer = node->FLNextBuffer;
                node->FLNextBuffer->FLPrevBuffer = node->FLPrevBuffer;
            }
        }

        
        void FLShow()
        {
            BUFFER* node = head;
            if(!isEmpty())
            {
                do
                {
                    cout << "<---> [" << node->blockNumber << "|" << node->status[0]<<":" <<node->status[1]<< ":"<<node->status[2]<<"] ";
                    node = node->FLNextBuffer;
                } while (node != head);
                cout << endl;
            }
            else cout << "EMPTY FREE LIST" << endl;
        }
};


class HASH_QUEUE
{
    private:
        BUFFER* head;
        BUFFER* tail;

    public:
        bool isEmpty() { return (head==NULL); }


        void HQInsertTail(BUFFER* item)
        {
            if(isEmpty())
            {
                item->HQNextBuffer = item->HQPrevBuffer = item;
                head = tail = item;
            }
            else
            {
                item->HQPrevBuffer = tail;
                item->HQNextBuffer = head;
                head->HQPrevBuffer = tail->HQNextBuffer = item;
                tail = tail->HQNextBuffer;
            }
        }

        // To find an element in Hash Queue
        BUFFER* HQFind(int blockNumber)
        {
            BUFFER* buffer = NULL;
            if (!isEmpty())
            {
                BUFFER* node = head;
                do
                {
                    if (node->blockNumber == blockNumber)
                    {
                        buffer = node;
                        break;
                    }
                    node = node->HQNextBuffer;
                } while (node != head);
            }
            return buffer;
        }
        
        // To remove an element from Hash Queue
        void HQRemoveNode(int blockNumber)
        {
            BUFFER* node = HQFind(blockNumber);
            if(node->HQNextBuffer == node)		// when list contains only one element
                head = tail = NULL;
            else
            {
                if(node == head) head = head->HQNextBuffer;
                else if(node == tail) tail = tail->HQPrevBuffer;
                node->HQPrevBuffer->HQNextBuffer = node->HQNextBuffer;
                node->HQNextBuffer->HQPrevBuffer = node->HQPrevBuffer;
            }
        }

        // To display a Hash Queue
        void HQShow()
        {
            BUFFER* node = head;
            if(!isEmpty())
            {
                do
                {
                    cout << "<--->[" << node->blockNumber << "|" << node->status[0] <<":"<<node->status[1]<<":"<<node->status[2]<<"]";
                    node = node->HQNextBuffer;
                } while (node != head);
            }
            else cout << "EMPTY HASH QUEUE" << endl;
        }

};

int hashIndex(int blockNumber,int NUMBER_OF_QUEUES){
    return(blockNumber % NUMBER_OF_QUEUES);
} 

void displayStucture(int NUMBER_OF_QUEUES,HASH_QUEUE** hashQueue,FREE_LIST* freeList)
{  

    cout<< "\n\t**Hash Queue Status**\n";
    for(int i=0; i<NUMBER_OF_QUEUES; i++)
    {   
        cout<< "\n-------";
        cout << "\n| HQ" << i << " | ";
        hashQueue[i]->HQShow();
        cout<< "\n-------";
    }

    cout<< "\n\t**Free List Status**\n\n";
    freeList->FLShow();

    cout << "\n\n";
}

#endif
