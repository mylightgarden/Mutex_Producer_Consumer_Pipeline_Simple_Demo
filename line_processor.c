/*-----------------------------------------
Author: Sophie Zhao
-----------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <err.h>
#include <errno.h>
#include <string.h>

#define BUFF_LINE_SIZE 1000               //size buffers to hold 50 lines of 1000 characters each
#define NUM_LINES 50

int metSTOP = 0;

// Buffer 1, shared resource between input thread and line separator thread
char buffer_1[BUFF_LINE_SIZE * NUM_LINES];
int count_1 = 0;                            // Number of items in the buffer
int prod_idx_1 = 0;                         // Index where the input thread will put the next item
int con_idx_1 = 0;                          // Index where the consumer thread will pick up the next item
pthread_mutex_t mutex_1 = PTHREAD_MUTEX_INITIALIZER;    // Initialize the mutex for buffer 1
pthread_cond_t full_1 = PTHREAD_COND_INITIALIZER;       // Initialize the condition variable for buffer 1
int stopped_1 = 0;


// Buffer 2, shared resource between line separator thread and plus sign thread
char buffer_2[BUFF_LINE_SIZE * NUM_LINES];
int count_2 = 0;                            // Number of items in the buffer
int prod_idx_2 = 0;                         // Index where the input thread will put the next item
int con_idx_2 = 0;                          // Index where the consumer thread will pick up the next item
pthread_mutex_t mutex_2 = PTHREAD_MUTEX_INITIALIZER;    // Initialize the mutex for buffer 2
pthread_cond_t full_2 = PTHREAD_COND_INITIALIZER;       // Initialize the condition variable for buffer 2
int stopped_2 = 0;

// Buffer 3, shared resource between plus sign thread and output thread
char buffer_3[BUFF_LINE_SIZE * NUM_LINES];
int count_3 = 0;                            // Number of items in the buffer
int prod_idx_3 = 0;                         // Index where the input thread will put the next item
int con_idx_3 = 0;                          // Index where the consumer thread will pick up the next item
pthread_mutex_t mutex_3 = PTHREAD_MUTEX_INITIALIZER;    // Initialize the mutex for buffer 3
pthread_cond_t full_3 = PTHREAD_COND_INITIALIZER;       // Initialize the condition variable for buffer 3
int stopped_3 = 0;

/*
 Put an item in buff_1
*/
void put_buff_1(char item){  
  pthread_mutex_lock(&mutex_1);             // Lock the mutex before putting the item in the buffer  
  buffer_1[prod_idx_1] = item;              // Put the item in the buffer  
  prod_idx_1 = prod_idx_1 + 1;              // Increment the index where the next item will be put.
  count_1++;  
  pthread_cond_signal(&full_1);             // Signal to the consumer that the buffer is no longer empty  
  pthread_mutex_unlock(&mutex_1);           // Unlock the mutex
}

/*
 Put an item in buff_2
*/
void put_buff_2(char item){ 
    pthread_mutex_lock(&mutex_2);             // Lock the mutex before putting the item in the buffer  
    buffer_2[prod_idx_2] = item;              // Put the item in the buffer
    prod_idx_2 = prod_idx_2 + 1;              // Increment the index where the next item will be put.
    count_2++; 
    pthread_cond_signal(&full_2);             // Signal to the consumer that the buffer is no longer empty  
    pthread_mutex_unlock(&mutex_2);           // Unlock the mutex
}

/*
 Put an item in buff_3
*/
void put_buff_3(char item){ 
  pthread_mutex_lock(&mutex_3);             // Lock the mutex before putting the item in the buffer  
  buffer_3[prod_idx_3] = item;              // Put the item in the buffer
  prod_idx_3 = prod_idx_3 + 1;              // Increment the index where the next item will be put.
  count_3++;  
  pthread_cond_signal(&full_3);             // Signal to the consumer that the buffer is no longer empty  
  pthread_mutex_unlock(&mutex_3);           // Unlock the mutex
}

/*
Get the next item from buffer 1
*/
char get_buff_1(){  
  pthread_mutex_lock(&mutex_1);                 // Lock the mutex before checking if the buffer has data
  while (count_1 == 0) {
    if (metSTOP){                               // If reached STOP
        stopped_1 = 1;
        return 0;
    }
    pthread_cond_wait(&full_1, &mutex_1);       // Buffer is empty. Wait for the producer to signal that the buffer has data
  }

  char item = buffer_1[con_idx_1];
  con_idx_1 = con_idx_1 + 1;                    // Increment the index from which the item will be picked up
  count_1--;
  
  pthread_mutex_unlock(&mutex_1);               
  return item;
}

/*
Get the next item from buffer 2
*/
char get_buff_2(){  
  pthread_mutex_lock(&mutex_2);                 // Lock the mutex before checking if the buffer has data
  while (count_2 == 0){
    if (stopped_1){                             // If reached STOP and the previous buffers are stopped
        pthread_cond_signal(&full_3);
        stopped_2 = 1;
        return 0;
    }
    pthread_cond_wait(&full_2, &mutex_2);       // Buffer is empty. Wait for the producer to signal that the buffer has data
  }      

  char item = buffer_2[con_idx_2];
  con_idx_2 = con_idx_2 + 1;                    // Increment the index from which the item will be picked up
  count_2--;
  
  pthread_mutex_unlock(&mutex_2);               
  return item;
}

/*
Get the next item from buffer 3
*/
char get_buff_3(){  
  pthread_mutex_lock(&mutex_3);                 // Lock the mutex before checking if the buffer has data
  while (count_3 == 0) {
    if (stopped_2){                             // If reached STOP and the previous buffers are stopped
        stopped_3 = 1;
        return 0;
    }
    pthread_cond_wait(&full_3, &mutex_3);       // Buffer is empty. Wait for the producer to signal that the buffer has data
  }   
    
  char item = buffer_3[con_idx_3];
  con_idx_3 = con_idx_3 + 1;                    // Increment the index from which the item will be picked up
  count_3--;
  
  pthread_mutex_unlock(&mutex_3);               
  return item;
}

/*
 Function that the input thread will run.
 Get input from the user.
 Put the item in the buffer shared with the line separater thread.
*/
void *getInput(void *args)
{
    for (int i = 0; i < NUM_LINES; i++)
    {
        // Get the user input, and report error if any
        char line_content[BUFF_LINE_SIZE];
        if ((fgets(line_content, BUFF_LINE_SIZE, stdin) == NULL) && ferror(stdin)){
            fprintf(stderr, "Error: fgets, when read stdin %s \n",  errno);
        }
        
        // Loop until receive the stop-processing line
        if(strcmp(line_content, "STOP\n") == 0){        // When reached STOP
            metSTOP = 1;
            return NULL;
        }
        
        //Add each char to buff_1
        for (int i = 0; i < strlen(line_content); i++){
            put_buff_1(line_content[i]);  
        } 
        for (int i = 1; i> 100000000; i++){
            int j = i;
        }
    }
    return NULL;
}

/*
 Function that the lineSeparator thread will run.
 replace new line with space and put data into buffer_2
*/
void *lineSeparator(void *args){
    //Go though buffer 2 to replace new line with space
    for (int i = 0; i < BUFF_LINE_SIZE * NUM_LINES; i++){
        char temp = get_buff_1();
        if (stopped_1){                         // If reached STOP and the previous buffers are stopped
            break;
        }

        if (temp == '\n'){
            temp = ' ';
            }
        put_buff_2(temp);        
    }
    return NULL;
}

/*
 Function that the plusSign thread will run.
 replace ++ with ^ and put data into buffer_2
*/
void *plusSign(void *args){
    //Go though buffer 2 to replace the previous '+' with '^'
    for (int i = 0; i < BUFF_LINE_SIZE * NUM_LINES; i++){
        char temp = get_buff_2();
        char temp2;

        if (stopped_2){                         // If reached STOP and the previous buffers are stopped
            break;
        }

        if(temp == '+'){
            // Replace the previous '+' with '^'
            temp2 = get_buff_2();
            if( temp2 == '+') 
                {
                    put_buff_3('^');                    
                }
            else{
                put_buff_3(temp);
                put_buff_3(temp2);
            }                     
        }
        else{
            put_buff_3(temp);
        }
    }
    return NULL;
}

/*
 Function that the outPut thread will run.
 write to stdout in lines of 80 charaters
*/
void *outPut(void *args){
    char curr_line[81];

    //Go though buffer 3 to print out lines of 80 char
    for(int i=0; i<BUFF_LINE_SIZE * NUM_LINES; i++){
        char temp = get_buff_3();
        if (stopped_3){                 // If reached STOP and the previous buffers are stopped
            break;
        } 

        else{                   
            curr_line[i%80] = temp;     //80 charaters per line
            if((i+1) % 80 == 0){                
                printf("%s\n", curr_line);
                fflush(stdout);                
            }
        }            
    }
    return NULL;
}

/*
Main:
 - Create threads
 - Wait for threads to terminate
*/
int main()
{
    srand(time(0));

    pthread_t input_t, lineSeparator_t, plusSign_t, outPut_t;

    // Create the threads
    pthread_create(&input_t, NULL, getInput, NULL);
    pthread_create(&lineSeparator_t, NULL, lineSeparator, NULL);
    pthread_create(&plusSign_t, NULL, plusSign, NULL);
    pthread_create(&outPut_t, NULL, outPut, NULL);

    // Wait for the threads to terminate
    pthread_join(input_t, NULL);
    pthread_join(lineSeparator_t, NULL);
    pthread_join(plusSign_t, NULL);
    pthread_join(outPut_t, NULL);

    return EXIT_SUCCESS;    
}

