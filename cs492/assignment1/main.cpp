#include <iostream>
#include <istream>
#include <sstream>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <vector>
#include <queue>
#include <cstdlib>
#include <limits>
/*I pledge my honor that I've abided by the stevens honors system
  name: Hariharan Vijayachandran
        Hannah Rodom

*/
using namespace std;

int parameters[7];
int getinputs(int argc, char* argv[]);
void fn();//computes 10th fibbonacci number
class product{
private:
  int id;
  clock_t timestamp;
  double wait;//in milliseconds
  clock_t temp;
  int life;
public:
  product(int id_t,int life_t ,clock_t timestamp_t);//set id, life, time
  int getid();//get id
  int getlife();//get life
  clock_t getTstamp();//get stamp
  void update_wait();
  void update_temp();
  void updateLife(int x){//updates life
    life = x;
  }
  ~product();
};
class productq{
private:
  int id;
  queue <product*> q;
   int maxsize;
    int consumed;
  int produced;
  
public:

  productq( int size);//set size of queue (0 means unlimited)
  bool isfull();//if queue size 0, returns true, else returns if queue is full
  bool isempty();//returns if queue is empty
  int push();//pushes a new product 
  product* pop();//pops and returns a product
  int getConsumed();//returns amount consumed
  int getProduced();//returns amount produced
  void consume();
  void addBack(product * p){//adds back product
    p->update_temp();
    q.push(p);
  }
};
clock_t start;
pthread_t* consumer;//consumer threads
pthread_t* producer;//producer threads
pthread_mutex_t acc;//blocks access to product queue
pthread_cond_t isfull;//conditional for full queue
pthread_cond_t isempty;//cond for empty queue
void* producerFunc(void*);//producer function for both fcfs and rr
void instantiateProducerThreads();//instantiates producer threads
void instantiateConsumerThreads(void* (*f)(void*));// consumer threads
void* roundrobinConsumer(void* id);//roundrobin consumer function
void roundrobin();//runs roundrobin
void destroyThreads();//destroys all threads, both consumer and producer and their id
void joinThreads();//joins all threads
int* producerID;//array of numbers for id
int* consumerID;//array of numbers for id
productq* q;//pointer to the product queue object
void instantiateMutexCond();//creates mutex and cond vars
void destroyMutexCond();//destroys mutex and cond vars
void* fcfs(void* id );//first come first serve
double totaltime, minT, maxT, avgT, minW, maxW,avgW, consumerT, producerT, processtime, producerThroughputTime;//global vars for statistics
void initStats();//starts the stats
void printStats();//ends the stats and prints it
int main(int argc, char * argv[]){
  if(  getinputs(argc, argv)==0){
    /*print error message for not enough inputs*/
    return 0;
  }
  q = new productq(parameters[3]);
   instantiateMutexCond();
   initStats();
   
  instantiateProducerThreads();
  
  
  if(parameters[4]==0){
    //fcfs
     instantiateConsumerThreads(fcfs);
  }else{
    //rr
    instantiateConsumerThreads(roundrobinConsumer);
  }
  joinThreads();
  //cout<<"here 1"<<endl;
  printStats();
  destroyThreads();
  //cout<<"here 2"<<endl;
 
  destroyMutexCond();
  //printStats();
  pthread_exit(0);
}










void fn(){
  int first, second, third;
  first = 0;
  second = 1;
  for(int x = 2; x<=10; x++){
    third = first+second;
    first = second;
    second = third;

  }





}
int  getinputs(int argc,char* argv[]){
  if(argc<8){
    return 0;
  }
  for(int x = 0; x<7;x++){
    stringstream str(argv[x+1]);
    str>>parameters[x];
  }
  return 1;
}



product::product(int id_t,int life_t ,clock_t timestamp_t): id{id_t},life{life_t}, timestamp {timestamp_t}, temp {timestamp_t},wait{0.0}{};
int product::getid(){
    return id;
  }

void product::update_wait(){  
  //clock_t x = clock();
  wait+= (double)clock()/(CLOCKS_PER_SEC/1000.0)-(double)temp/(CLOCKS_PER_SEC/1000.0);
}
void product::update_temp(){
  temp = clock();

}
int product:: getlife(){
    return life;
  }


clock_t product::getTstamp(){
    return timestamp;
  }


productq::productq( int size){
    maxsize = size;
    srand(parameters[6]);
    id=produced=consumed = 0;
   
  } 

bool productq::isfull(){
    if(maxsize==0){
      return false;
    }
    return q.size()==maxsize;
  }
bool productq::isempty(){
  if(parameters[2]==produced){
    return false;
  }
    return q.empty();
  }
int  productq::push(){
  product *p = new product(++id,rand()%1024 ,clock());
    produced++;
    if(produced==1){
      start = p->getTstamp();
    }
    q.push(p);
    if(produced==parameters[2]){
      producerThroughputTime = clock()/(CLOCKS_PER_SEC/1000.0)-producerThroughputTime;
    }
    return id;
  }
product* productq::pop(){
       
    product *p = q.front();
    p->update_wait();
     q.pop();
     return p;
  }
int productq::getConsumed(){
  return consumed;
}
int productq::getProduced(){
  return produced;

}

void* producerFunc( void *id){
  
  while(1){   
  pthread_mutex_lock(&acc);
  while((q->isfull())&&(parameters[2]!=q->getProduced())){
    //cout<<"prod here"<<*(int*)id<<endl;
    //fflush(stdout);
    pthread_cond_wait(&isfull,&acc);
  }
  if(parameters[2]!=q->getProduced()){
  int a = q->push();
  cout<<"producer "<<(int)*(int*)(id)<<" produced product "<<a<<endl;
  fflush(stdout);
   pthread_cond_signal(&isempty);
  
  pthread_mutex_unlock(&acc);
  usleep(100000);
  }else{
    pthread_cond_broadcast(&isfull);
    pthread_mutex_unlock(&acc);
    usleep(100000);
    break;
  }
  
  }
  pthread_exit(NULL);
}


void instantiateProducerThreads(){
  producer = new pthread_t[parameters[0]];
  producerID = new int[parameters[0]]; 
  for(int x = 0; x<parameters[0]; x++){
    producerID[x] = x+1;
    pthread_create(&producer[x],NULL, producerFunc, &producerID[x]);
    
  }

}

void destroyThreads(){
  
  delete [] producer;
  delete [] producerID;
  delete [] consumer;
  delete [] consumerID;

}
void productq::consume(){
  consumed++;
}
void joinThreads(){
  for(int x = 0; x<parameters[0]; x++){
    pthread_join(producer[x],NULL);
  }
  for(int x = 0; x<parameters[1];x++){
     pthread_join(consumer[x],NULL);
    }
}

void* roundrobinConsumer(void* id){
  int quantum = parameters[5];
  product *p;
  int life;
  while(1){
    pthread_mutex_lock(&acc);
    while(q->isempty()){
      pthread_cond_wait(&isempty,&acc);
    }
    if(q->getConsumed()==parameters[2]){
      pthread_cond_broadcast(&isempty);
      
      pthread_mutex_unlock(&acc);
      break;
    }
    p = q->pop();
    life = p->getlife();
    if(life<=quantum){
      double eat = clock()/(CLOCKS_PER_SEC/1000.0);
      for(int x = 0; x<life; x++){
	fn();
      }
      eat = clock()/(CLOCKS_PER_SEC/1000.0)-eat;
      totaltime+=eat;
      cout<<"consumer "<<*(int*)id<<" consumed product "<<p->getid()<<endl;
      delete p;
      q->consume();
      pthread_cond_signal(&isfull);
      fflush(stdout);
    }else{
       double eat = clock()/(CLOCKS_PER_SEC/1000.0);
      for(int x = 0; x<quantum; x++){
	fn();
      }
      eat = clock()/(CLOCKS_PER_SEC/1000.0)-eat;
      totaltime+=eat;
      p->updateLife(life-quantum);
      q->addBack(p);
      //cout<<"added back"<<endl;
      
    }
    pthread_mutex_unlock(&acc);
    usleep(100000);
  }
  // cout<<"exited"<<endl;
  pthread_exit(NULL);
  
}

void instantiateConsumerThreads(void* (*f)(void*)){
  consumer = new pthread_t[parameters[1]];
  consumerID = new int [parameters[1]];
  for(int x = 0; x<parameters[1];x++){
    consumerID[x]=x+1;
     pthread_create(&consumer[x],NULL,(*f), &consumerID[x]);
  }

}
					  
void instantiateMutexCond(){
  pthread_mutex_init(&acc,NULL);
  pthread_cond_init(&isfull,NULL);
  pthread_cond_init(&isempty,NULL);

}

void destroyMutexCond(){
  pthread_mutex_destroy(&acc);
  pthread_cond_destroy(&isfull);
  pthread_cond_destroy(&isempty);

}					  
product::~product(){
  clock_t t = clock();
  if((wait>maxW)){
    //cout<<"wait1"<<endl;
    maxW = wait;
  }
  if((minW>wait)){
    //cout<<"wait"<<endl;
    //fflush(stdout);
    minW = wait;
  }
  avgW+=wait;
  double tt = t/(CLOCKS_PER_SEC/1000.0) - timestamp/(CLOCKS_PER_SEC/1000.0);
  if((tt>maxT)){
    maxT = tt;

  }
  if((tt<minT)){
    minT = tt;
  }
  avgT+=tt;
}

void initStats(){
  totaltime,avgW,avgT = 0.0;
  minW=  100000000.0;
  minT = 100000000.0;
  maxT=0.0; maxW = 0.0;
  processtime = clock()/(CLOCKS_PER_SEC/1000.0);
  producerThroughputTime = processtime;
}

void printStats(){

  processtime = clock()/(CLOCKS_PER_SEC/1000.0)-processtime;
  cout<<"total time for processing all products = "<<processtime<<" milliseconds"<<endl;
  //cout<<"process time = "<<processtime<< "milliseconds"<<endl;
  cout<<"Minimum Turnover = "<<(double)minT<<" milliseconds"<<endl;
  cout<<"Maximum Turnover= "<<(double)maxT<<" milliseconds"<<endl;
  cout<<"Average Turnover= "<<(avgT/((double)parameters[2]))<<" milliseconds"<<endl;
  cout<<"Minimum wait = "<<(double)minW<<" milliseconds"<<endl;
  cout<<"Maximum wait= "<<(double)maxW<<" milliseconds"<<endl;
  cout<<"Average wait= "<<(avgW/((double)parameters[2]))<<" milliseconds"<<endl;
  cout<<"Consumer Throughput = "<<((double)parameters[2])/(processtime/1000.0/60.0)<<"products per minute"<<endl;
  cout<<"Producer Throughput= "<<((double)parameters[2])*60000.0/producerThroughputTime<<"products per minute"<<endl;
}

void * fcfs(void * id){
  product *p;
  int life;
  while(1){
    pthread_mutex_lock(&acc);
    while(q->isempty()){
      pthread_cond_wait(&isempty,&acc);
    }
    if(q->getConsumed()==parameters[2]){
      pthread_cond_broadcast(&isempty);
      
      pthread_mutex_unlock(&acc);
      break;
    }
    p = q->pop();
    life = p->getlife();
    double eat = clock()/(CLOCKS_PER_SEC/1000.0);
      for(int x = 0; x<life; x++){
	fn();
      }
      eat = clock()/(CLOCKS_PER_SEC/1000.0)-eat;
      totaltime+=eat;
      cout<<"consumer "<<*(int*)id<<" consumed product "<<p->getid()<<endl;
      delete p;
      q->consume();
      pthread_cond_signal(&isfull);
      fflush(stdout);
   
    pthread_mutex_unlock(&acc);
    usleep(100000);
  }
  // cout<<"exited"<<endl;
  pthread_exit(NULL);
  


}
