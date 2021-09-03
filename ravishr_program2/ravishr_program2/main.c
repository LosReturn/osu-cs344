//
//  main.c
//  ravishr_program2
//  Inline comments and citations.
//

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdbool.h>
#include <time.h>

#define PREFIX "movies_"

/*
 * Used from project 1
 * I got this code and used it from the student example provided in assignment 1.
 * struct for movie information
 */
struct movie
{
   char *title;
   int year;
   char *languages;
   double rating_value;
   struct movie *next;
};

/*
 * Used from project 1
 * I got this code and used it from the student example provided in assignment 1.
 * Parse the current line which is space delimited and create a
 * movie struct with the data in this line
 */
struct movie *createMovie(char *currLine) {
   struct movie *currMovie = malloc(sizeof(struct movie));

   // For use with strtok_r
   char *saveptr;

   // The first token is the title
   char *token_char = strtok_r(currLine, ",", &saveptr);
   currMovie->title = calloc(strlen(token_char) + 1, sizeof(char));
   strcpy(currMovie->title, token_char);
    
   // The next token is the year
   int token_int = atoi(strtok_r(NULL, ",", &saveptr));
   currMovie->year = token_int;
   
   // The next token is the languages
   token_char = strtok_r(NULL, ",", &saveptr);
   currMovie->languages = calloc(strlen(token_char) + 1, sizeof(char));
   strcpy(currMovie->languages, token_char);
   
   // The next token is the rating value
   double token_double = strtod(saveptr, &saveptr);
   currMovie->rating_value = token_double;

   // Set the next node to NULL in the newly created movie entry
   currMovie->next = NULL;

   return currMovie;
}

/*
 * Used from project 1
 * I got this code and used it from the student example provided in assignment 1.
 * Return a linked list of movies by parsing data from
 * each line of the specified file.
 */
struct movie *processFile(char *filePath) {
    // Open the specified file for reading only
    FILE *movieFile = fopen(filePath, "r");

    char *currLine = NULL;
    size_t len = 0;
    ssize_t nread;
    //char *token;

    // The head of the linked list
    struct movie *head = NULL;
    // The tail of the linked list
    struct movie *tail = NULL;

    // Read the file line by line
    while ((nread = getline(&currLine, &len, movieFile)) != -1) {
        // Get a new movie node corresponding to the current line
        struct movie *newNode = createMovie(currLine);

        // Is this the first node in the linked list?
        if (head == NULL) {
            // This is the first node in the linked link
            // Set the head and the tail to this node
            head = newNode;
            tail = newNode;
        }
        else {
            // This is not the first node.
            // Add this node to the list and advance the tail
            tail->next = newNode;
            tail = newNode;
        }
    }
    free(currLine);
    fclose(movieFile);
    return head;
}

/*
 * Creates new directory
 * Referenced site below to figure out how to send in an array of chars
 * properly and return the value properly.
 * StackOverflow - 47
 * https://stackoverflow.com/questions/17131863/passing-string-to-a-function-in-c-with-or-without-pointers
 * Helper method - additional
 */
char createDirectory(char *string) {
   srand(time(NULL));
   // Referenced site below to figure out how to get a random
   // number within a range.
   // https://www.geeksforgeeks.org/generating-random-number-range-c/
   int random = (rand() % (99999 - 0 + 1));
   // Referenced the site below on how to use sprintf().
   // https://www.geeksforgeeks.org/sprintf-in-c/
   sprintf(string, "ravishr.movies.%d", random);
   // Create new directory
   // Referenced site below to find symbolic constants for mode.
   // https://man7.org/linux/man-pages/man2/open.2.html
   mkdir(string, S_IRWXU | S_IRGRP | S_IXGRP);
   return *string;
}

/*
 * Creates a file for each year a movie was released
 * Helper method - additional
 */
void createFiles(struct movie *list, char *file_name) {
   int fd;
   // Goes through each movie
   while (list != NULL) {
      // Creates XXXX.txt
      sprintf(file_name, "%d.txt", list->year);
      // Skips over first row
      if (strcmp(file_name, "0.txt") != 0) {
         fd = open(file_name, O_WRONLY | O_CREAT | O_APPEND,
                   S_IRUSR | S_IWUSR | S_IRGRP);
         // Inserts movie title into file
         write(fd, list->title, strlen(list->title));
         // New line
         write(fd, "\n", strlen("\n"));
         close(fd);
      }
      list = list->next;
   }
}

int main(int argc, const char * argv[]) {
   printf("1. Select file to process\n");
   printf("2. Exit the program\n");
   
   int choice;
   do {
      printf("\nEnter a choice 1 or 2: ");
      scanf("%d", &choice);
      
      if (choice < 1 || choice > 2) {
         printf("You entered an incorrect choice. Try again.\n");
      }
      else if (choice == 1) {
         int fileChoice;
         // Allows menu to keep looping until a file is found.
         bool fileFound = false;

         do {
            printf("\nWhich file you want to process?\n");
            printf("Enter 1 to pick the largest file\n");
            printf("Enter 2 to pick the smallest file\n");
            printf("Enter 3 to specify the name of a file\n");
            printf("\nEnter a choice from 1 to 3: ");
            scanf("%d", &fileChoice);
            
            DIR* currDir = opendir(".");
            struct dirent *aDir;
            struct stat directoryStat;
            char *ptr;
            int ch = '.';
            off_t currSize = 0;
            int i = 0;
            char entryName[256];
            
            // begin if--------------------------------------------------
            
            /* Find largest file with .csv extention and
             * starting with prefix movies_
             * Exploration: Directories -> Used example of stat function, that
             * showed how to find a file with a specific prefix.
             */
            if (fileChoice == 1) {
               while ((aDir = readdir(currDir)) != NULL) {
                  ptr = strrchr(aDir->d_name, ch);
                  // If file does not have an extension, skip check to
                  // avoid segmentation fault with null ptr.
                  if (ptr != NULL) {
                     if (strncmp(PREFIX, aDir->d_name, strlen(PREFIX)) == 0 &&
                         strcmp(".csv", ptr) == 0) {
                        fileFound = true;
                        stat(aDir->d_name, &directoryStat);
                        if (i == 0 || directoryStat.st_size > currSize) {
                           currSize = directoryStat.st_size;
                           memset(entryName, '\0', sizeof(entryName));
                           strcpy(entryName, aDir->d_name);
                        }
                        i++;
                     }
                  }
               }
               closedir(currDir);
               free(aDir);
               
               if (fileFound) {
                  printf("Now processing the chosen file named %s\n", entryName);
                  
                  // Can't be more than 21 if largest number is 5 digits.
                  char dir_name[21];
                  // Creates a new directory
                  createDirectory(dir_name);
                  printf("Created directory with name %s\n", dir_name);
                  
                  
                  struct movie *list = processFile(entryName);
                  char file_name[9];
                  currDir = opendir(".");
                  
                  /* Navigates to the newly created directory in which
                   * the files created for each year a movie was released
                   * will be put into.
                   */
                  while ((aDir = readdir(currDir)) != NULL) {
                     //printf("aDir: %s\n", aDir->d_name);
                     if (strncmp(dir_name, aDir->d_name,
                                 strlen(dir_name)) == 0) {
                        /*
                         * https://stackoverflow.com/questions/56031104/the-cd-command-in-c-and-use-of-chdir
                         * Referenced the first answer provided to use this
                         * in order to go into newly made directory.
                        */
                        chdir(dir_name);
                        break;
                     }
                  }
                  
                  createFiles(list, file_name);
                  
                  chdir("..");
                  closedir(currDir);

               }
               else
                  printf("The file %s was not found. Try again\n", entryName);
            }
            
            // begin else if------------------------------------------------
            
            /* Find smallest file with .csv extention and
             * starting with prefix movies_
             * Exploration: Directories -> Used example of stat function, that
             * showed how to find a file with a specific prefix.
             */
            else if (fileChoice == 2) {
               while ((aDir = readdir(currDir)) != NULL) {
                  ptr = strrchr(aDir->d_name, ch);
                  // If file does not have an extension, skip check to
                  // avoid segmentation fault with null ptr.
                  if (ptr != NULL) {
                     if (strncmp(PREFIX, aDir->d_name, strlen(PREFIX)) == 0 &&
                         strcmp(".csv", ptr) == 0) {
                        fileFound = true;
                        stat(aDir->d_name, &directoryStat);
                        if (i == 0 || directoryStat.st_size < currSize) {
                           currSize = directoryStat.st_size;
                           memset(entryName, '\0', sizeof(entryName));
                           strcpy(entryName, aDir->d_name);
                        }
                        i++;
                     }
                  }
               }
               closedir(currDir);
               free(aDir);
               
               if (fileFound) {
                  printf("Now processing the chosen file named %s\n", entryName);
                  
                  // Can't be more than 21 if largest number is 5 digits.
                  char dir_name[21];
                  createDirectory(dir_name);
                  printf("Created directory with name %s\n", dir_name);
                  
                  // Parse data and create a file for each year a movie
                  // was released.
                  struct movie *list = processFile(entryName);
                  char file_name[9];
                  currDir = opendir(".");
                   
                  while ((aDir = readdir(currDir)) != NULL) {
                     //printf("aDir: %s\n", aDir->d_name);
                     if (strncmp(dir_name, aDir->d_name,
                                 strlen(dir_name)) == 0) {
                        /*
                         * https://stackoverflow.com/questions/56031104/the-cd-command-in-c-and-use-of-chdir
                         * Referenced the first answer provided to use this
                         * in order to go into newly made directory.
                         */
                        chdir(dir_name);
                        break;
                     }
                  }
                  
                  createFiles(list, file_name);
                  
                  chdir("..");
                  closedir(currDir);
               }
               else
                  printf("The file %s was not found. Try again\n", entryName);
            }
            
            // begin else if ------------------------------------------------
            
            /* Referenced below to find file specified by user input.
             * Exploration: Directories -> Used example of stat function, that
             * showed how to loop through directory entries.
             */
            else if (fileChoice == 3) {
               char *filePath = calloc(256, sizeof(char));;
               printf("Enter the complete file name: ");
               scanf("%s", filePath);
               //printf("filePath: %s\n", filePath);

               while ((aDir = readdir(currDir)) != NULL) {
                  if (strcmp(aDir->d_name, filePath) == 0)
                     fileFound = true;
               }
               closedir(currDir);
               free(aDir);
            
               if (fileFound) {
                  printf("Now processing the chosen file named %s\n", filePath);
                  // Can't be more than 21 if largest number is 5 digits.
                  char dir_name[21];
                  createDirectory(dir_name);
                  printf("Created directory with name %s\n", dir_name);
                  
                  // Parse data and create a file for each year a movie
                  // was released.
                  struct movie *list = processFile(filePath);
                  char file_name[9];
                  currDir = opendir(".");
                   
                  while ((aDir = readdir(currDir)) != NULL) {
                     //printf("aDir: %s\n", aDir->d_name);
                     if (strncmp(dir_name, aDir->d_name,
                                 strlen(dir_name)) == 0) {
                        /*
                         * https://stackoverflow.com/questions/56031104/the-cd-command-in-c-and-use-of-chdir
                         * Referenced the first answer provided to use this
                         * in order to go into newly made directory.
                        */
                        chdir(dir_name);
                        break;
                     }
                  }
                  
                  createFiles(list, file_name);
                  
                  chdir("..");
                  closedir(currDir);
                  free(filePath);
               }
               else
                  printf("The file %s was not found. Try again\n", filePath);
            }
            
            // begin else----------------------------------------------------
            
            else
               printf("You did not enter a choice from 1 to 3.\n");
            
         } while (fileFound == false); // internal do-while loop
      } // end else if (within external do-while loop)
      
   } while (choice != 2); // external do while loop
   
   return 0;
}

