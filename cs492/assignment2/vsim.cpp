#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iterator>
#include <list>
using namespace std;
void print_memory();
//global variables for p1, p2, p3
//p2: FIFO, LRU, CLOCK
int swaps = 0;
int p1; string  p2;
bool p3;//true if prepaging is turned on
string plist;
string ptrace;
unsigned long int global_time = 0;
int page_number;//largest page number, +1 will give u number of pages
class clock_alg_entry{
public:
  int page_number;
  int r;
};

vector <clock_alg_entry*> clock_data_structure;

class table_entry{
public:
  int page_number;
  int valid_bit;
  unsigned long int last_accessed;
  bool operator==(int x){
    return x==page_number;
  }
};
vector <table_entry*>table_list;//page table list, each element is an array of table_entry, with the last entry with a page_number of -1, denoting the end of the entries


int *main_memory; //holds the page number thats in memory
int index=0; //holds the position of a free, unused memory block in the main memory. index<512/p1 and
//should fill up all free spots in memory before using page swaps
////////////////////////////////////////////////////////
void clock_alg();
bool is_valid(int process, int page);
int prepaging_swap(int process,int page_needed, int page_not_needed1, int page_not_needed2);//returns the second page thats brought back, -1 if all pages in memory
void demand_swap(int process,int page_needed, int page_not_needed);
int main(int arg, char* args[]){
  //global variables and table setup
  string str= args[3];
  
  p1 =  stoi(str);
    p2=args[4];
  p3 = args[5][0]=='+';
  ptrace=args[2];
  plist=args[1];
  ifstream file;
  string s;
  file.open(plist+".txt");
   page_number = 0;
  while(getline(file,s)&&s.length()>0){
    int size = stoi(s.substr(s.find_first_of(" ",0)));
    
    int pages = size/p1 + ((size%p1==0)?0:1);
    table_entry* arr = new table_entry[pages+1];
    for(int x = 0; x<pages;x++){
      arr[x].page_number = page_number;
      page_number++;
      arr[x].valid_bit=0;
      arr[x].last_accessed = 0;
   
    }
    arr[pages].page_number = -1;
     table_list.push_back(arr);
  }
  file.close();
  main_memory = new int [512/p1];
  fill_n(main_memory,512/p1, -1);
  int size = 512/p1/table_list.size();
  
  for(int x = 0; x<table_list.size(); x++){
    int xx = 0;
    while(table_list[x][xx].page_number!=-1&&xx<size){
      main_memory[index] = table_list[x][xx].page_number;
      clock_alg_entry* ent = new clock_alg_entry;
      ///////clockalgorithm data structure, do not touch///
      ent->page_number = main_memory[index];
      ent->r = 0;
      clock_data_structure.push_back(ent);
	//////
      table_list[x][xx].valid_bit=1;
      table_list[x][xx].last_accessed =global_time ;//add the initial page data and time to your desired data structure here like i did, without doing anything else to the program
      index++;
      xx++;
    }
  }
  global_time++;
  ///////////////////////////
 
  if(p2[0]=='C'){
    clock_alg();
  }
  if(p2 =="FIFO"){
  }
  if(p2=="LRU"){

  }
  
   
  
}


void demand_swap(int process,int page_needed, int page_not_needed){
  for(int counter = 0; counter<512/p1; counter++){
    if(main_memory[counter]==page_not_needed){
      main_memory[counter]=page_needed;
      break;
    }
  }
  table_list[process][page_needed-table_list[process][0].page_number].valid_bit = 1;
  table_list[process][page_needed-table_list[process][0].page_number].last_accessed = global_time;
  global_time++;
   table_list[process][page_not_needed-table_list[process][0].page_number].valid_bit = 0;
   // print_memory();
}
int prepaging_swap(int process,int page_needed,int page_not_needed1 ,int page_not_needed2){
  int counter = page_needed-table_list[process][0].page_number+1;
  table_list[process][counter-1].valid_bit = 1;
  table_list[process][counter-1].last_accessed = global_time;
  while(table_list[process][counter].valid_bit==1||table_list[process][counter].page_number==-1){
    if(table_list[process][counter].page_number==-1){
      counter = 0;
      continue;
    }
    if(counter== page_needed-table_list[process][0].page_number){
      break;
    }
    counter++;
  }
  int c =(counter== page_needed-table_list[process][0].page_number)?-1:counter ;
   for( counter = 0; counter<512/p1; counter++){
    if(main_memory[counter]==page_not_needed1){
      main_memory[counter]=page_needed;
      table_list[process][page_not_needed1-table_list[process][0].page_number].valid_bit = 0;
    }
    if(c!=-1&&main_memory[counter]==page_not_needed2){
      main_memory[counter] = table_list[process][c].page_number ;
      table_list[process][c].valid_bit = 1;
      table_list[process][c].last_accessed= global_time;
      table_list[process][page_not_needed2-table_list[process][0].page_number].valid_bit = 0;
    }
    
  }
   global_time++;
   //print_memory();
   return (c!=-1)?table_list[process][c].page_number:c;
   
}

void clock_alg(){
  ifstream file;
  string s;
  file.open(ptrace+".txt");
  while(getline(file,s)&&s.length()>0){
    print_memory();
    int process = stoi(s.substr(0,s.find_first_of(" ",0)));
    int address = stoi(s.substr(s.find_first_of(" ",0)));
    address = address/p1;
    if(is_valid(process,address)){
      int page = table_list[process][address].page_number;
      for(int x = 0; x<clock_data_structure.size(); x++){
	if (clock_data_structure[x]->page_number==page){
	  clock_data_structure[x]->r=1;break;
	}
	
      }
      continue;
    }
    if(index!=512/p1){
      int page =  table_list[process][address].page_number;
      table_list[process][address].valid_bit = 1;
      main_memory[index] = page;
      index++;
      clock_alg_entry* ent = new clock_alg_entry;
      ent->page_number = page;
      ent->r = 1;
      clock_data_structure.push_back(ent);
      continue;
    }
    if(p3){
      int flag =0; int page_rem_1, page_rem_2; int counter = 0;int page =  table_list[process][address].page_number;
      while(flag!=2){
	if(clock_data_structure[counter%clock_data_structure.size()]->r==1){
	  clock_data_structure[counter%clock_data_structure.size()]->r=0;
	  counter ++;
	  continue;
	}
	if(clock_data_structure[counter%clock_data_structure.size()]->r==0){
	  if(flag==0){
	    flag = 1;
	    page_rem_1 = clock_data_structure[counter%clock_data_structure.size()]->page_number;
	    clock_data_structure[counter%clock_data_structure.size()]->page_number = page;
	    clock_data_structure[counter%clock_data_structure.size()]->r=1;
	    counter++;
	    continue;
	  }
	  if(flag==1){
	    flag =2;
	     page_rem_2 = clock_data_structure[counter%clock_data_structure.size()]->page_number;
	     clock_data_structure[counter%clock_data_structure.size()]->page_number = prepaging_swap(process,page,page_rem_1,page_rem_2);
	    clock_data_structure[counter%clock_data_structure.size()]->r=1;
	    counter++;
	    continue;

	  }
	}
      }
      

    }else{
      int flag =0; int page_rem_1; int counter = 0;int page =  table_list[process][address].page_number;
      while(flag!=1){
	if(clock_data_structure[counter%clock_data_structure.size()]->r==1){
	  clock_data_structure[counter%clock_data_structure.size()]->r=0;
	  counter ++;
	  continue;
	}
	if(clock_data_structure[counter%clock_data_structure.size()]->r==0){
	  
	    flag = 1;
	    page_rem_1 = clock_data_structure[counter%clock_data_structure.size()]->page_number;
	    clock_data_structure[counter%clock_data_structure.size()]->page_number = page;
	    clock_data_structure[counter%clock_data_structure.size()]->r=1;
	    demand_swap(process, page, page_rem_1);
	    counter++;
	    continue;
	  
	}
      }

    }
  }
  file.close();
}

bool is_valid(int process, int page){
  return table_list[process][page].valid_bit==1;

}


void print_memory(){
  for(int x = 0; x<512/p1;x++){
    cout<<main_memory[x]<<endl;
  }
  cout<<"////////////////////////////"<<endl;

}
