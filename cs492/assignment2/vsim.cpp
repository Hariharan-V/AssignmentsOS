#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iterator>
#include <list>
#include <queue>
#include <cmath>
//i Pledge my honor that I've abided by the stevens honor system,names: Hariharan Vijayachandran, Hanah Rodom
using namespace std;
void lru();//lru algorithm
void print_memory();//prints content of memory
void fifo();//first in first out algorithm
//global variables for p1, p2, p3
//p2: FIFO, LRU, CLOCK
int swaps = 0;
int p1; string  p2;
bool p3;//true if prepaging is turned on
string plist;
string ptrace;
unsigned long int global_time = 0;
int page_number;//largest page number, +1 will give u number of pages
int count = 0;
int * hand;
class table_entry{//table entry, holds page number, bit validity, and last accessed time (r bit for Clock)
public:
  int page_number;
  int valid_bit;
  unsigned long int last_accessed;
  bool operator<(const table_entry& x){
    return last_accessed<x.last_accessed;
  }
  int r;
};

vector <table_entry*>table_list;//page table list, each element is an array of table_entry, with the last entry with a page_number of -1, denoting the end of the entries
vector < queue <table_entry*> > fifo_data_structure;

int *main_memory; //holds the page number thats in memory
int index=0; //holds the position of a free, unused memory block in the main memory. index<512/p1 and
//should fill up all free spots in memory before using page swaps
////////////////////////////////////////////////////////
void clock_alg();//clock algorithm
bool is_valid(int process, int page);//checks if its a valid address
int prepaging_swap(int process,int page_needed, int page_not_needed1, int page_not_needed2);//returns the second page thats brought back, -1 if all pages in memory updates time, only used for lru 
void demand_swap(int process,int page_needed, int page_not_needed);//demand swaps, updates time
int main(int arg, char* args[]){
  if(arg<6){
    cout<<"not enough inputs"<<endl;
    return 0;
  }
  //global variables and table setup
  string str= args[3];
  
  p1 =  stoi(str);
  p2= string(args[4]);
  p3 = args[5][0]=='+';
  ptrace=args[2];
  plist=args[1];
  ifstream file;
  string s;
  file.open(plist+".txt");
   page_number = 0;
   ///table_list is a vector of arrays that hold table_entry objects, table_list[0][0] will return the first table_entry object stored at process 0
   ///each table entry has a valid bit, r bit, and last_accessed global time, which is an unsigned long
   //the tables are filled up so that the page number starts from 0 to n-1, n being the total number of pages
   //so for example if process 0 requests address 30, and ur page size is 10, you would go to table_list[0][(30-1)/10] to access the table entry.
   //the main_memory has 512/pagesize entries in total, and the number of initial pages are divided up evenly among the processes
   //main_memory array just holds the pages currently residing in memory. there is a good chance that some of the memories are not filled up, hence
   //they will be filled with -1's, the global variable index is what determines if there are any blank memory blocks left
   //if index is not equal to 512/pagesize, then that means main_memory[index] is currently holding a value of -1, meaning its empty. Before you do
   //any memory swaps, first check whether the main_memory is full, or else first fill it up. after filling it up, update the index to show the next
   //free memory or to show that mainmemory is full
   //prepaging_swap function takes in the process, the page needed, the first page thats not needed, and the second page thats not needed
   //this function handles the valid bits of the pages, swaps the contents on memory, and updates the time as well, and it returns the second page
   //number that it put in memory
   //demand_swap does the same except u only input the process, page_needed, and page not needed, and it returns nothing, it sets the valid bit,
   //updates time, and swaps the contents on main memory
   //the end of an array held in table_list is denoted by a table_entry with the value -1, valid bit 0, and r bit 0
   //so table_list[0]'s last object (ie table_list[0][n], n being the index of the last element) will contain the page number -1, this is done to
   //denote the end of a page table entry for a process, and IS NOT A VALID PAGE
  while(getline(file,s)&&s.length()>0){
    int size = stoi(s.substr(s.find_first_of(" ",0)));
    
    int pages = size/p1 + ((size%p1==0)?0:1);//rounds up the number of pages if theres any fractional amount
    table_entry* arr = new table_entry[pages+1];
    for(int x = 0; x<pages;x++){
      arr[x].page_number = page_number;//default value in the table is 0 for valid bit, last accessed, and r bit
      page_number++;
      arr[x].valid_bit=0;
      arr[x].last_accessed = 0;
      arr[x].r = 0;
    }
    arr[pages].page_number = -1;//the end of a table for a process is denoted by a page with -1 as the page number
    arr[pages].valid_bit = 0;
     table_list.push_back(arr);
  }
  fifo_data_structure = vector < queue < table_entry* > >(table_list.size());//set the size of the fifo (its a vector of queues)
  hand = new int[table_list.size()];//initialize the hands of the clock algorithm
  fill_n(hand,table_list.size(), 0);//set it all to 0
  file.close();
  //main memory is filled with initial values here
  // every process is given an equal amount of page space on the main memory
  main_memory = new int [512/p1];//main memory has 512/p1 pages
  fill_n(main_memory,512/p1, -1);//unfilled memory blocks will contain -1
  int size = (int)round(512.0/(float)p1/(float)table_list.size());
  size = 512/p1/table_list.size();//size allocated to every process
  if (size<2){
    p3 = false;//if each process isnt given at least 2 pages on memory, then only demand paging is valid
  }
  if(size<1){
    cout<<"impossible as no process gets any pages allotted"<<endl;
    return 0;
  }
  for(int x = 0; x<table_list.size(); x++){//load pages into memory
    int xx = 0;
    while(xx<size){
      if(table_list[x][xx].page_number==-1||index>=512/p1){
	break;
      }
      main_memory[index] = table_list[x][xx].page_number;
      table_list[x][xx].valid_bit=1;
      table_list[x][xx].last_accessed =global_time ;
      table_list[x][xx].r = 1;
      fifo_data_structure[x].push(&table_list[x][xx]);
      index++;
      xx++;
    }
  }
  global_time++;
  ///////////////////////////
  /*for(int x = 0; x<10;x++){

    int c = 0;
    while(table_list[x][c].page_number!=-1){
      cout<<table_list[x][c].page_number<<" "<<table_list[x][c].valid_bit<<endl;
	c++;
    }
    cout<<table_list[x][c].page_number<<" "<<table_list[x][c].valid_bit<<endl;
    }*/
  if(p2=="Clock"){//clock
    clock_alg();
  }else
  if(p2=="FIFO"){//FIFO
    fifo();
  }else
  if(p2=="LRU"){//LRU
    lru();
  }else{
    return 0;
  }
  print_memory();
  cout<<"number of swaps: "<<swaps<<endl;
}


void demand_swap(int process,int page_needed, int page_not_needed){//searches main memory, and swaps the page needed and page not needed, with 
  swaps++;//updated time //used for everything but LRU
  for(int counter = 0; counter<512/p1; counter++){//search contents on memory
    if(main_memory[counter]==page_not_needed){
      main_memory[counter]=page_needed;
      break;
    }
  }
  table_list[process][page_needed-table_list[process][0].page_number].valid_bit = 1;//set valid bits
  table_list[process][page_needed-table_list[process][0].page_number].last_accessed = global_time;//set time
  global_time++;
   table_list[process][page_not_needed-table_list[process][0].page_number].valid_bit = 0;
   // print_memory();
}
int prepaging_swap(int process,int page_needed,int page_not_needed1 ,int page_not_needed2){//only used for LRU, gives the same result as using demand swap twice for LRU but not for others, made to make code neater
  int counter1;
  int counter2;
  swaps++;
  int start = table_list[process][0].page_number;//set starting page of process
  for(int x = 0;x<512/p1;x++){//find contents in memory
    if(main_memory[x]==page_not_needed1){
      counter1 = x;
    }
    if(main_memory[x]==page_not_needed2){
      counter2 = x;
    }
  }
  int x = page_needed-table_list[process][0].page_number;
  x++;
  while(table_list[process][x].valid_bit==1||table_list[process][x].page_number==-1){//find contiguous page
    if(table_list[process][x].page_number==-1){
      x = 0;
      continue;
    }
    x++;
  }
  if(table_list[process][x].page_number==-1){
    cout<<"welp"<<endl;
  }
 
  table_list[process][page_needed-start].valid_bit = 1;//set valid bits and swap
  table_list[process][page_needed-start].last_accessed = global_time;
  table_list[process][page_not_needed1-start].valid_bit = 0;
  table_list[process][page_not_needed2-start].valid_bit = 0;
     table_list[process][x].valid_bit = 1;
     table_list[process][x].last_accessed = global_time;
     global_time++;
     main_memory[counter1] = page_needed;
     main_memory[counter2] = table_list[process][x].page_number;
     return main_memory[counter2];
   
  
}

void clock_alg(){
  ifstream file;
  string s;
  file.open(ptrace+".txt");
  while(getline(file,s)&&s.length()>0){//readline by line
    // count++;
    // print_memory();
    // cout<<count<<endl;
    int process = stoi(s.substr(0,s.find_first_of(" ",0)));//store process
    int address = stoi(s.substr(s.find_first_of(" ",0)));//store address
    address = (address-1)/p1;//translate address to index on table_list
    if(is_valid(process,address)){//if its a valid page, just updaate time set accessed bit to 1
      table_list[process][address].r=1;
      table_list[process][address].last_accessed = global_time;
      global_time++;
      continue;
    }
    /* if(index!=512/p1){//if its not a valid page but theres still space in main memory, then add the value to main_memory, set valid bit to 1, and 
      int page =  table_list[process][address].page_number;//increment index 
      table_list[process][address].valid_bit = 1;
      table_list[process][address].r = 1;
      table_list[process][address].last_accessed = global_time;
      global_time++;
      main_memory[index] = page;
      index++;
      continue;
      }*/
    if(p3){//if its not a valid page and theres no space in main memory, and prepaging is set
      int page =  table_list[process][address].page_number;
      
      
      /* int page_not_needed1;
      int page_not_needed2;
      int counter=hand[process];
      int flag=0;
      while(flag!=2){
	if(table_list[process][counter].page_number==-1){
	  counter = 0;
	  continue;
	}
	if(table_list[process][counter].valid_bit==0){
	  counter++;
	  continue;
	}
	if(table_list[process][counter].valid_bit==1&&table_list[process][counter].r==1){
	  table_list[process][counter].r=0;
	  counter++;
	  continue;
	}
	if(table_list[process][counter].valid_bit==1&&table_list[process][counter].r==0){
	  if(flag ==0){
	    flag = 1;
	    page_not_needed1 = table_list[process][counter].page_number;
	    table_list[process][counter].valid_bit=0;
	    counter++;
	    continue;
	  }
	  if(flag==1){
	    flag = 2;
	    page_not_needed2 = table_list[process][counter].page_number;
	   int  secondpage = prepaging_swap(process,page,page_not_needed1,page_not_needed2);
	    table_list[process][secondpage-table_list[process][0].page_number].r = 1;
	    table_list[process][page-table_list[process][0].page_number].r = 1;
	    counter++;
	    hand[process]= counter;
	  }
	}
	
	}*/
      int page_not_needed1; int page_not_needed2;//store two pages that aren't needed
      int counter = hand[process];//get the hand(initial value is 0)
      page_not_needed1 = -1;
      while(page_not_needed1 ==-1){//search for the first page thats not needed, loop through the table
	if(table_list[process][counter].page_number==-1){
	  counter = 0;
	  continue;
	}
	if(table_list[process][counter].valid_bit==0){
	  counter ++;
	  continue;
	}
	if(table_list[process][counter].valid_bit==1&&table_list[process][counter].r ==1){//if the r bit is 1, its 0 now
	  table_list[process][counter].r = 0;
	  counter++; continue;
	  
	}
	if(table_list[process][counter].valid_bit==1&&table_list[process][counter].r ==0){//if the r bit 0, get rid of the page and swap
	  page_not_needed1 = table_list[process][counter].page_number;
	  table_list[process][address].r=1;
	  demand_swap(process,page,page_not_needed1);
	  swaps--;//decrement swap to not count twice
	  counter++;
	}
      }
      page_not_needed2 = -1;//second page
      address++;
      while(table_list[process][address].valid_bit==1||table_list[process][address].page_number==-1){//find the second page you don't need
	if(table_list[process][address].page_number==-1){
	  address = 0;
	  continue;
	}
	address++;
      }
      page = table_list[process][address].page_number;
         while(page_not_needed2 ==-1){
	if(table_list[process][counter].page_number==-1){
	  counter = 0;
	  continue;
	}
	if(table_list[process][counter].valid_bit==0){
	  counter ++;
	  continue;
	}
	if(table_list[process][counter].valid_bit==1&&table_list[process][counter].r ==1){//if the r bit is 1, set to 0
	  table_list[process][counter].r = 0;
	  counter++; continue;
	  
	}
	if(table_list[process][counter].valid_bit==1&&table_list[process][counter].r ==0){//if the r bit is 0, get rid of the page and swap
	  page_not_needed2 = table_list[process][counter].page_number;
	  table_list[process][address].r=1;
	  demand_swap(process,page,page_not_needed2);//swap
	  counter++;
	  hand[process]=counter;//store the hand
	}
      }
      
    }else{//if prepaging is not set, same as prepaging except you only need one page swap
      int page =  table_list[process][address].page_number;//set page
      
      int counter = hand[process];//store hand
       int page_not_needed = -1;
       while(page_not_needed==-1){//loop through setting r bit of 1s to 0s and swapping the first one with the 0
	 if(table_list[process][counter].page_number==-1){
	   counter = 0;
	   continue;
	     
	 }
	 if(table_list[process][counter].valid_bit==0){
	   counter++;
	   continue;
	 }
	 if(table_list[process][counter].r==1&&table_list[process][counter].valid_bit==1){
	   table_list[process][counter].r = 0;
	   counter++;
	   continue;
	 }
	 if(table_list[process][counter].r==0&&table_list[process][counter].valid_bit==1){
	   page_not_needed =  table_list[process][counter].page_number;
	  
	   table_list[process][address].r = 1;
	   demand_swap(process,page,page_not_needed);
	   counter++;
	   hand[process]=counter;
	 }
       }
      
    }
  }
  file.close();
}

bool is_valid(int process, int page){//check if valid
  return table_list[process][page].valid_bit==1;

}


void print_memory(){//use this funciton to print out memory contents
  for(int x = 0; x<512/p1;x++){
    cout<<main_memory[x]<<endl;
  }
  cout<<"////////////////////////////"<<endl;

}

void fifo(){//first in first out
 ifstream file;
  string s;
  file.open(ptrace+".txt");
  while(getline(file,s)&&s.length()>0){//readline by line
    // count++;
    // print_memory();
    // cout<<count<<endl;
    int process = stoi(s.substr(0,s.find_first_of(" ",0)));//store process
    int address = stoi(s.substr(s.find_first_of(" ",0)));//store address
    address = (address-1)/p1;//translate address to index on table_list
    if(is_valid(process,address)){//if its a valid page, just updaate time set accessed bit to 1
      table_list[process][address].r=1;
      table_list[process][address].last_accessed = global_time;
      global_time++;
      continue;
    }
    /* if(index!=512/p1){//if its not a valid page but theres still space in main memory, then add the value to main_memory, set valid bit to 1, and 
      int page =  table_list[process][address].page_number;//increment index 
      table_list[process][address].valid_bit = 1;
      table_list[process][address].r = 1;
      table_list[process][address].last_accessed = global_time;
      global_time++;
      main_memory[index] = page;
      fifo_data_structure[process].push(&table_list[process][address]);
      index++;
      continue;
      }*/
    if(p3){//prepaging
      /*  int page =  table_list[process][address].page_number;
       int page_not_needed1 = fifo_data_structure[process].front()->page_number;
       fifo_data_structure[process].pop();
      int page_not_needed2 = fifo_data_structure[process].front()->page_number;
      fifo_data_structure[process].pop();
      int page_needed = prepaging_swap(process,page,page_not_needed1,page_not_needed2);
      fifo_data_structure[process].push(&table_list[process][address]);
      fifo_data_structure[process].push(&table_list[process][page_needed-table_list[process][0].page_number]);*/
      int page =  table_list[process][address].page_number;//store page that u want
      int page_not_needed = fifo_data_structure[process].front()->page_number;//store the page that u don't want
      fifo_data_structure[process].pop();//pop it out of the queue
      demand_swap(process,page,page_not_needed);//call demand swap
      fifo_data_structure[process].push(&table_list[process][address]);//push the page u want back into the queue
       address++;
       while(table_list[process][address].valid_bit!=0||table_list[process][address].page_number==-1){//find the second contiguous page
	 if(table_list[process][address].page_number==-1){
	   address = 0;
	   continue;
	 }
	 address++;
       }
       page =  table_list[process][address].page_number;//store the second contiguous page
       page_not_needed = fifo_data_structure[process].front()->page_number;//store second page you don't want
       fifo_data_structure[process].pop();//pop the second page u don't want
       demand_swap(process,page,page_not_needed);//swap //push the second contiguous page
       fifo_data_structure[process].push(&table_list[process][address]);
       swaps--;//demand swap actually increments swaps so i gotta decrement it once cuz i call it twice
       
       
    }else{
      int page =  table_list[process][address].page_number;//demand paging is the same as prepaging except i only swap one page as opposed to 2
       int page_not_needed1 = fifo_data_structure[process].front()->page_number;
       fifo_data_structure[process].pop();
       demand_swap(process,page,page_not_needed1);
       fifo_data_structure[process].push(&table_list[process][address]);
    }
    
  }
  file.close();


}

void lru(){
   ifstream file;
  string s;
  file.open(ptrace+".txt");
  while(getline(file,s)&&s.length()>0){//readline by line
    // count++;
    // print_memory();
    // cout<<count<<endl;
    int process = stoi(s.substr(0,s.find_first_of(" ",0)));//store process
    int address = stoi(s.substr(s.find_first_of(" ",0)));//store address
    address = (address-1)/p1;//translate address to index on table_list
    if(is_valid(process,address)){//if its a valid page, just updaate time set accessed bit to 1
      table_list[process][address].r=1;
      table_list[process][address].last_accessed = global_time;
      global_time++;
      continue;
    }
    /*  if(index!=512/p1){//if its not a valid page but theres still space in main memory, then add the value to main_memory, set valid bit to 1, and 
      int page =  table_list[process][address].page_number;//increment index 
      table_list[process][address].valid_bit = 1;
      table_list[process][address].r = 1;
      table_list[process][address].last_accessed = global_time;
      global_time++;
      main_memory[index] = page;
      index++;
      continue;
      }*/
    if(p3){//if its not a valid page and theres no space in main memory, and prepaging is set
       int page =  table_list[process][address].page_number;//increment index
       int counter = 1;
       table_entry* ent = &table_list[process][0];
       while(table_list[process][counter].page_number!=-1){//find the lowest/least accessed page by looping through and comparing the last accessed time
	 if(ent->valid_bit==0&&table_list[process][counter].valid_bit==1){
	   ent = &table_list[process][counter];
	   counter++;
	   continue;
	 }
	 if(ent->valid_bit==1&&table_list[process][counter].valid_bit==1){
	   if(ent->last_accessed >table_list[process][counter].last_accessed){//comare times, set ent to the swappable page, and stop looping
	     ent = &table_list[process][counter];
	     counter++;
	     continue;
	   }
	 }
	 counter++;

       }
       int page_not_needed1 = ent->page_number;//store page not needed 1
       ent->valid_bit =0;
       ent = &table_list[process][0];
       counter = 1;
       while(table_list[process][counter].page_number!=-1){//do the same as above but now find the second lowed/ least accessed page
	 
	 if(ent->valid_bit==0&&table_list[process][counter].valid_bit==1){
	   ent = &table_list[process][counter];
	   counter++;
	   continue;
	 }
	 if(ent->valid_bit==1&&table_list[process][counter].valid_bit==1){
	   if(ent->last_accessed >table_list[process][counter].last_accessed){
	     ent = &table_list[process][counter];
	     counter++;
	     continue;
	   }
	 }
	 counter++;

       }
       int page_not_needed2 = ent->page_number;//store the second least accessed page
       prepaging_swap(process, page, page_not_needed1, page_not_needed2);//swap, now this swap assumes you can swap the two pages at once, since it makes no difference in the algorithm itself. Regardless of whether you swap it one at a time or two at a time, the same two pages are picked to replace the contents on RAM.
       
    }else{//demand paging, same as prepaging except you only swap one page
       int page =  table_list[process][address].page_number;//increment index
       int counter = 1;
       table_entry* ent = &table_list[process][0];
       while(table_list[process][counter].page_number!=-1){//this is all the same as prepaging, except we only dealing with one page
	 
	 if(ent->valid_bit==0&&table_list[process][counter].valid_bit==1){
	   ent = &table_list[process][counter];
	   counter++;
	   continue;
	 }
	 if(ent->valid_bit==1&&table_list[process][counter].valid_bit==1){
	   if(ent->last_accessed >table_list[process][counter].last_accessed){//compare last accessed, and stop looping if u found the lowest
	     ent = &table_list[process][counter];
	     counter++;
	     continue;
	   }
	 }
	 counter++;

       }
       int page_not_needed1 = ent->page_number;
       demand_swap(process,page,page_not_needed1);//swap
      
    }
  }
  file.close();
}
