#include<iostream>
#include<cmath>
#include<cstdlib>
#include<chrono>
#include<thread>
#include<ctime>
#include<vector>
#include<mutex>
#include<condition_variable>
#include<cassert>

//maximum capacity for any resource type
static const int MaxCapacity = 6;

int N; // number of threads
int M; // number of resources

//lock variable
std::mutex lock;

//condition variable
std::condition_variable cv;

std::vector<int> 
  // amount of resource / resource type
  capacity, 
  // amount of available resource / resource type
  available;
std::vector<std::vector<int> > 
  // max number of resources a thread can request
  max, 
  // number of resources currently allocated to each thread
  allocation, 
  // number of resources requested by each thread
  request;

void printArray(std::vector<int> a) {
  for(int i = 0; i < a.size(); i++) {
    printf("%i ", a[i]);
  }
  printf("\n\n");
}

void print2DArray(std::vector<std::vector<int> > a) {
  for(int i = 0; i < a.size(); i++) {
    for (int j = 0; j < a[i].size(); j++) {
      printf("%i ", a[i][j]);
    }
    printf("\n");
  }
  printf("\n");
}

void initialize() {
  for(int i = 0; i < M; i++) { // for each resource
    capacity.push_back(rand() % (MaxCapacity - 1) + 1); // 1-->MaxCapacity+1 instead of 0-MaxCapacity
    available.push_back(capacity[i]);
  }
  for(int i = 0; i < N; i++) { // for each thread
    request.push_back(*new std::vector<int>());
    allocation.push_back(*new std::vector<int>());
    max.push_back(*new std::vector<int>());

    for(int j = 0; j < M; j++) { // for each resource
      request[i].push_back(0);
      allocation[i].push_back(0);
      //just to make things more interesting, increase max resource request size
      max[i].push_back(capacity[j] == 1 ? 1 : rand() % (capacity[j] + 1));
    }
  }
}

void createRandomRequest(int tid) {
  for(int i = 0; i < M; i++) {
    request[tid][i] = max[tid][i] == 0 ? 0 : rand() % max[tid][i];
  }
  printf("thread %i's request: ", tid);
  printArray(request[tid]);
}

std::vector<std::vector<int> > matrixSub(std::vector<std::vector<int> > a, std::vector<std::vector<int> > b) {
  std::vector<std::vector<int> > c;
  for(int i = 0 ; i < a.size(); i++) {
    c.push_back(*new std::vector<int>);
    for(int j = 0; j < a[i].size(); j++) {
      c[i].push_back(a[i][j] - b[i][j]);
    }
  }
  return c;
}


bool isSafe() {
  std::vector<int> toBeAvail(available);
  std::vector<std::vector<int> > need = matrixSub(max, allocation);
  bool finish[M]; for(int i = 0; i < M; i++) finish[i] = false;
  bool allFinished;
  int j = -1;
  while (true) {
    for(int i = 0; i < N; i++) { // for each thread
      bool canFillNeed = true;
      for(int k = 0; k < M; k++) { // for each resource
        if(need[i][k] > toBeAvail[k]) canFillNeed = false;
      }
      if(finish[i] == false && canFillNeed) { // if thread isn't finished yet
        j = i;
        break;
      }
    }
    if(j == -1) { // j not found
      allFinished = true;
      for(int i = 0; i < N; i++) {
        if(finish[i] == false) allFinished = false;
      }
      // if(allFinished == false) {
      //   std::cout << "not safe\n";
      //   print2DArray(request);
      //   print2DArray(allocation);
      //   printArray(available);
      // }
      return allFinished;
    } else {
      finish[j] = true;
      for(int l = 0; l < M; l++) 
        toBeAvail[l] = toBeAvail[l] + allocation[j][l];
    }
    j = -1;
  }
}

bool wouldBeSafe(int tid, int rid) {
  bool result = false;
  available[rid]--;
  allocation[tid][rid]++;
  if (isSafe()) {
    result = true;
  }
  available[rid]++;
  allocation[tid][rid]--;
  return result;
}
 
/*
tid = thread ID
rid = resource ID
*/
void makeRequest(int tid, int rid) {
  printf("thread %i requesting resource %i\n", tid, rid);
   // auto unlocked once out of scope
  std::unique_lock<std::mutex> lk(lock);
  cv.wait(lk, [&]{ // passing callback is equivalet to putting this in a while loop
    printf("thread %i waiting for resource %i\n", tid, rid);
    return wouldBeSafe(tid, rid);
  });
  assert(isSafe());
  allocation[tid][rid]++;
  available[rid]--;
  assert(isSafe());
  printf("thread %i granted resource %i\n", tid, rid);
}

void threadFunction(int tid) {
  int count = 0;
  while(true) {
    createRandomRequest(tid);
    for(int i = 0; i < M; i++) {
      for(int j = 0; j < request[tid][i]; j++) {
        makeRequest(tid, i);
        count++;
      }
    }
    //reset allocation
    for(int i = 0; i < M; i++) {
      available[i] += allocation[tid][i];
      allocation[tid][i] = 0;
    }
    cv.notify_all();
    if(count % 5 == 0) 
      printf("thread %i requested %i times\n", tid, count);
    std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 1000));
  }
  return;
}
/*
argv[1] = M = Resources
argv[2] = N = Threads
*/
int main(int argc, char** argv) {
  srand(time(NULL));
  try {
    M = std::stoi(argv[1]), N = std::stoi(argv[2]);
    if(M <= 0 || N <= 0) throw "invalid args";
  } catch(std::exception e) {
    std::cout << "Invalid Args\n" << e.what() << std::endl;
    return -1;
  }
  initialize();  
  std::cout << "initialized\n";
  printArray(capacity);
  printArray(available);
  print2DArray(allocation);
  print2DArray(request);
  print2DArray(max);

  std::thread threads[N];
  int i;
  for(i = 0; i < N; i++) {
    printf("creating thread %i\n", i);
    threads[i] = std::thread(threadFunction, i);
  }

  for(int i = 0; i < N; i++) threads[i].join();
  return 0;
}






