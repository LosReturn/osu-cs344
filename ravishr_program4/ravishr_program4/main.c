//
//  main.c
//  ravishr_program4
//  Inline comments and citations (for provided example program).
//

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

/*
 * Size of buffers.
 * Referenced example program provided for this assignment.
*/
#define LINES 50
#define CHARS 1000

/*
 * Creating buffers 1, 2, 3 to share resources between threads.
 * Referenced and used example program provided for this assignment
 * and only modified the type and size of buffer.
 * Kept inline comments provided in example program as well.
*/
// Buffer 1, shared resource between input thread and line separator thread
char buffer_1[LINES][CHARS];
// Number of items in the buffer
int count_1 = 0;
// Index where the input thread will put the next item
int prod_idx_1 = 0;
// Index where the line separator thread will pick up the next item
int con_idx_1 = 0;
// Initialize the mutex for buffer 1
pthread_mutex_t mutex_1 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 1
pthread_cond_t full_1 = PTHREAD_COND_INITIALIZER;

// Buffer 2, shared resource between line separator thread and plus signs thread
char buffer_2[LINES][CHARS];
// Number of items in the buffer
int count_2 = 0;
// Index where the line separator thread will put the next item
int prod_idx_2 = 0;
// Index where the output thread will pick up the next item
int con_idx_2 = 0;
// Initialize the mutex for buffer 2
pthread_mutex_t mutex_2 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 2
pthread_cond_t full_2 = PTHREAD_COND_INITIALIZER;

// Buffer 3, shared resource between plus signs thread and output thread
char buffer_3[LINES][CHARS];
// Number of items in the buffer
int count_3 = 0;
// Index where the plus signs thread will put the next item
int prod_idx_3 = 0;
// Index where the output thread will pick up the next item
int con_idx_3 = 0;
// Initialize the mutex for buffer 2
pthread_mutex_t mutex_3 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 2
pthread_cond_t full_3 = PTHREAD_COND_INITIALIZER;


/*
 * Get input from user.
 * Referenced and used example program provided for this assignment
 * and modified it slightly to allow for type of input.
*/
char *get_user_input(){
   char *input = NULL;
   size_t len = 0;
   printf("Enter something: ");
   getline(&input, &len, stdin);
   return input;
}

/*
 * Put input in buff_1.
 * Referenced and used example program provided for this assignment
 * and only modified the way input is put into buffer 1 (using strcpy).
 * Kept inline comments provided in example program as well.
*/
void put_buff_1(char *input){
   // Lock the mutex before putting the item in the buffer
   pthread_mutex_lock(&mutex_1);
   // Put the item in the buffer
   strcpy(buffer_1[prod_idx_1], input);
   // Increment the index where the next item will be put.
   prod_idx_1 = prod_idx_1 + 1;
   count_1++;
   // Signal to the consumer that the buffer is no longer empty
   pthread_cond_signal(&full_1);
   // Unlock the mutex
   pthread_mutex_unlock(&mutex_1);
}

/*
 * Function that the input thread will run.
 * Put the item in the buffer shared with the line_separator thread.
 * Referenced and used example program provided for this assignment
 * and modified by checking for stop processing line.
*/
void *get_input(void *args)
{
   for (int i = 0; i < LINES; i++){
      // Get input from the user
      char *input = get_user_input();
      // If stop processing line received, stop processing inputs
      if (strcmp(input, "STOP\n") == 0) {
         put_buff_1(input);
         return NULL;
      }
      put_buff_1(input);
      free(input);
   }
   return NULL;
}

/*
 * Get the next item from buffer 1.
 * Referenced and used example program provided for this assignment
 * and only modified type of input to get from buffer 1 (char*).
*/
char *get_buff_1(){
  // Lock the mutex before checking if the buffer has data
  pthread_mutex_lock(&mutex_1);
  while (count_1 == 0)
    // Buffer is empty. Wait for the producer to signal that the buffer has data
    pthread_cond_wait(&full_1, &mutex_1);
  char *input = buffer_1[con_idx_1];
  // Increment the index from which the item will be picked up
  con_idx_1 = con_idx_1 + 1;
  count_1--;
  // Unlock the mutex
  pthread_mutex_unlock(&mutex_1);
  // Return the item
  return input;
}

/*
 * Put an item in buff_2.
 * Referenced and used example program provided for this assignment
 * and only modified the way input is put into buffer 2 (using strcpy).
 * Kept inline comments provided in example program as well.
*/
void put_buff_2(char *input){
  // Lock the mutex before putting the item in the buffer
  pthread_mutex_lock(&mutex_2);
  // Put the item in the buffer
  strcpy(buffer_2[prod_idx_2], input);
  // Increment the index where the next item will be put.
  prod_idx_2 = prod_idx_2 + 1;
  count_2++;
  // Signal to the consumer that the buffer is no longer empty
  pthread_cond_signal(&full_2);
  // Unlock the mutex
  pthread_mutex_unlock(&mutex_2);
}

/*
 * Function that the line separator thread will run.
 * Replace any line separator (\n) with a space.
 * Put the item in the buffer shared with the plus signs thread.
 * Referenced and used example program provided for this assignment
 * and modified the function's task.
*/
void *replace_line_separator(void *args)
{
   char *input;
   for (int i = 0; i < LINES; i++) {
      input = get_buff_1();
      // If stop processing line received, stop processing inputs
      if (strcmp(input, "STOP\n") == 0) {
         put_buff_2(input);
         return NULL;
      }
      // Loop through entire input to check for line separator
      for (int j = 0; input[j] != '\0'; j++) {
         if (input[j] == '\n') {
            input[j] = ' ';
         }
      }
      put_buff_2(input);
   }
   return NULL;
}

/*
 * Get the next item from buffer 2
 * Referenced and used example program provided for this assignment
 * and only modified type of input to get from buffer 2 (char*).
*/
char *get_buff_2(){
   // Lock the mutex before checking if the buffer has data
   pthread_mutex_lock(&mutex_2);
   while (count_2 == 0)
    // Buffer is empty. Wait for the producer to signal that the buffer has data
    pthread_cond_wait(&full_2, &mutex_2);
   char *input = buffer_2[con_idx_2];
   // Increment the index from which the item will be picked up
   con_idx_2 = con_idx_2 + 1;
   count_2--;
   // Unlock the mutex
   pthread_mutex_unlock(&mutex_2);
   // Return the item
   return input;
}

/*
 * Put an item in buff_3.
 * Referenced and used example program provided for this assignment
 * and only modified the way input is put into buffer 3 (using strcpy).
 * Kept inline comments provided in example program as well.
*/
void put_buff_3(char *input){
  // Lock the mutex before putting the item in the buffer
  pthread_mutex_lock(&mutex_3);
  // Put the item in the buffer
  strcpy(buffer_3[prod_idx_3], input);
  // Increment the index where the next item will be put.
  prod_idx_3 = prod_idx_3 + 1;
  count_3++;
  // Signal to the consumer that the buffer is no longer empty
  pthread_cond_signal(&full_3);
  // Unlock the mutex
  pthread_mutex_unlock(&mutex_3);
}

/*
 * Function that the plus signs thread will run.
 * Replace any pair of + with ^.
 * Put the item in the buffer shared with the output thread.
 * Referenced and used example program provided for this assignment
 * and modified the function's task.
*/
void *replace_plus_signs(void *args)
{
   char *input;
   for (int i = 0; i < LINES; i++) {
      input = get_buff_2();
      if (strcmp(input, "STOP\n") == 0) {
         put_buff_3(input);
         return NULL;
      }
      // Loop through entire input to check for pair of plus signs
      for (int j = 0; input[j] != '\0'; j++) {
         if (input[j] == '+' && input[j+1] == '+') {
            input[j] = '^';
            // Move back all characters, starting at second plus sign, by one
            for (int k = j+1; k < CHARS; k++) {
               input[k] = input[k+1];
            }
         }
      }
      put_buff_3(input);
   }
   return NULL;
}

/*
 * Get the next item from buffer 3
 * Referenced and used example program provided for this assignment
 * and only modified type of input to get from buffer 3 (char*).
*/
char *get_buff_3(){
   // Lock the mutex before checking if the buffer has data
   pthread_mutex_lock(&mutex_3);
   while (count_3 == 0)
    // Buffer is empty. Wait for the producer to signal that the buffer has data
    pthread_cond_wait(&full_3, &mutex_3);
   char *input = buffer_3[con_idx_3];
   // Increment the index from which the item will be picked up
   con_idx_3 = con_idx_3 + 1;
   count_3--;
   // Unlock the mutex
   pthread_mutex_unlock(&mutex_3);
   // Return the item
   return input;
}

/*
 * Function that the output thread will run.
 * Consume an item from the buffer shared with the plus signs thread.
 * Write processed data, as lines of 80 characters, to standard output.
 * Referenced and used example program provided for this assignment
 * and modified the function's task to implement output as lines
 * of 80 characters.
*/
void *write_output(void *args)
{
   // An array of strings (80 characters in length)
   char **char_array;
   char_array = calloc(626, sizeof(char));
   for (int i = 0; i < 626; i++) {
      char_array[i] = calloc(81, sizeof(char));
   }
   char *input;
   // Current index of char in user input
   int input_index;
   // Current index of char in string in char_array
   int char_index = 0;
   // Current index of string in char_array
   int output_index = 0;
   for (int i = 0; i < LINES; i++) {
      input_index = 0;
      input = get_buff_3();
      if (strcmp(input, "STOP\n") == 0) {
         return NULL;
      }
      // Loop until end of current input
      while (input[input_index]) {
         // Store char, from input, in char_array
         char_array[output_index][char_index] = input[input_index];
         // If 80 chars in string reached, print to standard output
         if (char_index == 79) {
            char_array[output_index][80] = '\0';
            printf("\nOutput: %s\n", char_array[output_index]);
            // Increment 'string' index to start storing next 80 chars
            output_index++;
            // Reset char index in string to 0.
            char_index = 0;
            fflush(stdout);
         }
         else {
            // If current string does not have 80 chars, increment char_index
            char_index++;
         }
         // Increment to next char in user input
         input_index++;
      }
   }
   return NULL;
}

int main(int argc, const char * argv[]) {
   pthread_t input_t, line_separator_t, plus_signs_t, output_t;
   // Create the threads
   pthread_create(&input_t, NULL, get_input, NULL);
   pthread_create(&line_separator_t, NULL, replace_line_separator, NULL);
   pthread_create(&plus_signs_t, NULL, replace_plus_signs, NULL);
   pthread_create(&output_t, NULL, write_output, NULL);
   // Wait for the threads to terminate
   pthread_join(input_t, NULL);
   pthread_join(line_separator_t, NULL);
   pthread_join(plus_signs_t, NULL);
   pthread_join(output_t, NULL);
   return EXIT_SUCCESS;
}
