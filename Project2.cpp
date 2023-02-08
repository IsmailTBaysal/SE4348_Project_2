#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <random>
#include <unistd.h>

std::mutex mtx;

// Flag for Test and Set
std::atomic_flag flag = ATOMIC_FLAG_INIT;

// Atomic variable for Fetch and Increment
std::atomic<long long>data{0};

// Initialize variables
int algorithmType;
int numThreads;
int x = 0;

int arr[100]={0}; // for tree
volatile int victim[100];
volatile  bool interested[100];
int arrSize=0;
int shared=0;
int countParent=0;
std::thread myThreads[50];
int iterations=1000;
int mean=1;

// Function to check if n is power of 2
bool isPowerOfTwo(int n)
{
    if (n == 0)
        return 0;
    while (n != 1)
    {
        if (n%2 != 0)
            return 0;
        n = n/2;
    }
    return 1;
}
// Unlock for TT
void tournamentUnlock(int me)
{
    int i;
    for(i=0;i<arrSize;i++)
    {
        if (arr[i] == me)
            interested[i] = false;
    }
}
//Lock for TT
void tournamentLock(int me)
{

    int i,index=0;

    //get self index
    for (i=countParent;i<arrSize;i++)
    {
        if(arr[i]==me)
        {
            index=i;
            break;
        }
    }

    while(index>0)
    {

        interested[index] = true;
        if(index%2)
        {
            victim[index+1]=me;
            while(interested[index+1] && victim[index+1]==me);
            index=(index-1)/2;
        }
        else
        {
            victim[index]=me;
            while(interested[index-1] && victim[index]==me);
            index=(index-2)/2;
        }
        arr[index]=me;  // me thread moving up
    }

}
// Critical Section for TT
void enterCS(int threadNum)
{
    for(int i=0;i<iterations;i++)
    {
        tournamentLock(threadNum);
        std::cout << "Entering CS for" << threadNum << std::endl;
        shared++;
        tournamentUnlock(threadNum);
    }

}

// Test and Set
void testAndSet(int tid){

    while (flag.test_and_set()){}       // Lock
        std::cout << "Launched by thread " << tid + 1<< std::endl;
        for (int i = 0; i < 10000000; ++i) {
            x++;
        }
        flag.clear();       // Unlock
    }

// Fetch and Increment
void fetchAdd(int tid){
    for (int i = 0; i < 10000000; ++i) {
        long long val = data.fetch_add(1,std::memory_order_relaxed);
    }
}

int main() {
    int nodes=numThreads;
    int level=0,i,j=1;

    // Getting inputs from user
    std::cout << "Please enter\n"
                 "0 for TT based algorithm\n"
                 "1 for TAS based algorithm\n"
                 "2 for FAI based algorithm: ";
    std::cin >> algorithmType;
    while (algorithmType<0 || algorithmType>2){         // Invalid input
        std::cout << "Please enter valid input: ";
        std::cin >> algorithmType;
    }
    std::cout << "How many threads would you like to create? ";
    std::cin >> numThreads;
    while (isPowerOfTwo(numThreads) == 1){         // Invalid input
        std::cout << "Thread number not need not be a power of two. Please enter again: ";
        std::cin >> numThreads;
    }
    std::cout << "Launched from the main\n";
    std::thread t[numThreads];

    switch (algorithmType) {
        case 0:
            std::cout << "TT:\n ";
            //TT
            for(i=0;i<100;i++)
                interested[i]=false;

            for(i=0;i<100;i++)
                victim[i]=0;

            while(nodes>0)
            {
                nodes=nodes/2;
                level++;
            }

            countParent=pow(2,level)-1;
            arrSize=countParent+numThreads;

            // copy node number from level to arrsize-1
            for(i=countParent;i<arrSize;i++)
                arr[i]=j++;

            //each thread will call enterCS function
            for(i=0;i<numThreads;i++)
                myThreads[i] = std::thread(enterCS, i+1);

            for(i=0;i<numThreads;i++)
                myThreads[i].join();

            std::cout<<"After number of threads "<<numThreads<<"* iterations "<<iterations<<" the result is: "<<shared<<std::endl;
            break;
        case 1:
            //TAS
            std::cout << "TAS:\n ";

            // Create threads
            for (int i = 0; i < numThreads; ++i) {
                t[i] = std::thread(testAndSet,i);
            }

            for (int i = 0; i < numThreads; ++i) {
                t[i].join();
            }
            //Output
            std::cout << "Result: "<<x<< std::endl;
            break;

        case 2:
            //FAI
            std::cout << "FAI:\n";

            // Create threads
            for (int i = 0; i < numThreads; ++i) {
                t[i] = std::thread(fetchAdd,i);
            }

            for (int i = 0; i < numThreads; ++i) {
                t[i].join();
            }
            //Output
            std::cout << "Result: "<<data<< std::endl;
            break;
    }
    return 0;


}
