//
//  keygen.c
//  ravishr_program5
//
//

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

int main (int argc, char* argv[]) {
   srand(time(NULL));
   int rand_num;
   char characters[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
   int key_length = atoi(argv[1]);
   char key[key_length + 1]; // add 1 for newline character
   
   for (int i = 0; i < key_length; i++) {
      rand_num = rand() % 27;
      key[i] = characters[rand_num];
   }
   key[key_length] = '\0'; // strip off newline character
   printf("%s\n", key); // add newline character
}
