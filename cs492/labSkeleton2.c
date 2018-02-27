#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h> 
#include <errno.h>

#define NUMP 5

pthread_mutex_t fork_mutex[NUMP];
int attempt;
pthread_mutex_t eaterlock;
pthread_cond_t cond;
int eater;
int main()  
{
  int i;
  pthread_t diner_thread[NUMP]; 
  int dn[NUMP];
  attempt =0;
  eater=0;
  pthread_cond_init(&cond,NULL);
  pthread_mutex_init(&eaterlock, NULL);
  void *diner();
  for (i=0;i<NUMP;i++)
    pthread_mutex_init(&fork_mutex[i], NULL);

  for (i=0;i<NUMP;i++){
    dn[i] = i;
    pthread_create(&diner_thread[i],NULL,diner,&dn[i]);
  }
  for (i=0;i<NUMP;i++)
    pthread_join(diner_thread[i],NULL);

  for (i=0;i<NUMP;i++)
    pthread_mutex_destroy(&fork_mutex[i]);
  pthread_mutex_destroy(&eaterlock);
  pthread_cond_destroy(&cond);
  pthread_exit(0);

}

void *diner(int *i)
{
  int v;
  int eating = 0;
  printf("I'm diner %d\n",*i);
  v = *i;
  while (eating < 5) {
    printf("%d is thinking\n", v);
    sleep( v/2);
    printf("%d is hungry\n", v);
    pthread_mutex_lock(&eaterlock);
    while(attempt>=4){
      //printf("diner %d more than 4 eaters,wait\n",v); 
      pthread_cond_wait(&cond, &eaterlock);
    }
   
    attempt++;
    //pthread_cond_signal(&cond);
    pthread_mutex_unlock(&eaterlock);
    
    pthread_mutex_lock(&fork_mutex[v]);
    pthread_mutex_lock(&fork_mutex[(v+1)%NUMP]);
    eater++;
    printf("%d is eating\n",v);
    eating++;
    pthread_mutex_lock(&eaterlock);
    attempt--;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&eaterlock);
   
    sleep(1);
    printf("%d is done eating\n", v);
    eater--;
    pthread_mutex_unlock(&fork_mutex[v]);
    pthread_mutex_unlock(&fork_mutex[(v+1)%NUMP]);
  }
  pthread_exit(NULL);
}
