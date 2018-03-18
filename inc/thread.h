#include "header.h"
#include <pthread.h>
  
void *thread(void *arg) {
    
}
int createThread() {
    pthread_t id;
    int ret = pthread_create(&id, NULL, thread, NULL);
    if(ret!=0){
        printf ("Create pthread error!\n"); 
    }
        
    return ret;
}