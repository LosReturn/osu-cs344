//
//  main.c
//  ravishr_program1
//
// References/Citing Work:
// 1. Exploration: Arrays and Structures
// 2. Exploration: Pointers
// 3. Exploration: Memory Allocation
// 4. Exploration: Strings
// 5. Documentation regarding definition and use of various string functions
//    https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.2.0/com.ibm.zos.v2r2.bpxbd00/strstr.htm
//    I also came across strstr() in this site.
// 6. I used the provided example program with student data and modified it
//    for createMovie(), processFile(), when processFile() is called in main(),
//    and kept the structure of printStudent() in mind when creating my print
//    methods for this project.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* struct for movie information */
struct movie
{
   char *title;
   int year;
   char *languages;
   double rating_value;
   struct movie *next;
};

/* Parse the current line which is space delimited and create a
*  movie struct with the data in this line
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
* Return a linked list of movies by parsing data from
* each line of the specified file.
*/
struct movie *processFile(char *filePath, int *count) {
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
   while ((nread = getline(&currLine, &len, movieFile)) != -1)
   {
      // Get a new movie node corresponding to the current line
      struct movie *newNode = createMovie(currLine);

      // Is this the first node in the linked list?
      if (head == NULL)
      {
         // This is the first node in the linked link
         // Set the head and the tail to this node
         head = newNode;
         tail = newNode;
      }
      else
      {
         // This is not the first node.
         // Add this node to the list and advance the tail
         tail->next = newNode;
         tail = newNode;
      }
      // Will keep count of number of movies
      *count = *count + 1;
   }
   free(currLine);
   fclose(movieFile);
   return head;
}

/*
* Print the movie title's released in the specified year
* or a message indicating no movies were releaed that year.
*/
void printMovieYear(struct movie *list, int year) {
   // Used to determine if any movies were released in the specified year
   bool isData = false;
   while (list != NULL)
   {
      if (list->year == year)
      {
         isData = true;
         printf("%s\n", list->title);
      }
      list = list->next;
   }
   if (!isData) {
      printf("No data about movies released in the year %d\n", year);
   }
}

/*
* Print the movie's year, rating value and title of which had the
* highest rating of the year.
*/
void printMovieHighestRating(struct movie *list) {
   // Points to 'head' of the linked list 'list'
   struct movie *firstMovie = list;
   // Allocates space in heap
   struct movie *theMovie = malloc(sizeof(struct movie));
   double highestRating;

   for (int i = 1900; i <= 2021; i++) {
      highestRating = 0.0;
      while (list != NULL) {
         
         // Enters 'if' only if year matches and the movie's rating
         // is higher than value currently stored in highestRating.
         if (list->year == i && list->rating_value > highestRating) {
            highestRating = list->rating_value;
            // Deep copy of values in list into theMovie
            theMovie->title = calloc(strlen(list->title) + 1, sizeof(char));
            strcpy(theMovie->title, list->title);
            theMovie->year = list->year;
            theMovie->languages = calloc(strlen(list->languages) + 1, sizeof(char));
            strcpy(theMovie->languages, list->languages);
            theMovie->rating_value = list->rating_value;
            theMovie->next = NULL;
         }
         list = list->next;
      }
      
      // Using theMovie->title to check whether a high rating was found
      // for that year and a deep copy was made.
      // This will be NULL if no data is found.
      if (theMovie->title != NULL) {
         printf("%d  %.1f  %s\n", theMovie->year,
                theMovie->rating_value,
                theMovie->title);
      }
      // 'Resets' so that list references the 'head' of the linked list
      // of movies.
      list = firstMovie;
      //Resets theMovie to be ready for next year's check.
      theMovie->title = NULL;
   }
   free(firstMovie);
   free(theMovie);
}

/*
* Prints the movies that have been released in the specified language.
* Otherwise, prints a message indicating that there were no movies found.
*/
void printMovieLanguage(struct movie *list, char *language) {
   char *substring, *copy;
   int val;
   // Used to determine if any movies were released in the specified year
   bool isData = false;
   while (list != NULL) {
      // strst() finds whether parameter 2 (language) is a substring in
      // parameter 1 (list->languages).
      // Stores the first occurance found in 'substring'
      substring = strstr(list->languages, language);

      // If a substring was located, it means the language exists and
      // it won't be NULL.
      
      if (substring != NULL) {
         // Allocate space for copy and copy substring into 'copy'
         copy = calloc(strlen(substring) + 1, sizeof(char));
         strcpy(copy, substring);
         // Create token of what's in copy before ';'
         char *token = strtok(copy, ";]");
         // Compare token and user input 'language'
         val = strcmp(token, language);
         // If it is not an EXACT match, do not print data of movie
         if (val == 0) {
            isData = true;
            printf("%d  %s\n", list->year, list->title);
            
         }
      }
      list = list->next;
   }
   if (!isData) {
      printf("No data about movies released in %s\n", language);
   }
}


/*
*   Process the file provided as an argument to the program to
*   create a linked list of student structs and print out the list.
*   Compile the program as follows:
*       gcc --std=gnu99 -o students main.c
*/
int main(int argc, char *argv[])
{
   if (argc < 2)
   {
      printf("You must provide the name of the file to process\n");
      printf("Example usage: ./students student_info1.txt\n");
      return EXIT_FAILURE;
   }
   
   int count = 0;
   // Passing count by reference to get number of movies in csv file
   struct movie *list = processFile(argv[1], &count);
   
   printf("\nProcessed file %s and parsed data for %d movies\n\n", argv[1], count-1);
   printf("1. Show movies released in the specific year\n");
   printf("2. Show highest rated movie for each year\n");
   printf("3. Show the title and year of release of all movies in a specific language\n");
   printf("4. Exit from the program\n");
   
   int choice;
   do {
      printf("\nEnter a choice from 1 to 4: ");
      scanf("%d", &choice);
      if (choice < 1 || choice > 4) {
         printf("You entered an incorrect choice. Try again.\n");
      }
      else {
         if (choice == 1) {
            int year;
            printf("Enter the year for which you want to see the movies: ");
            scanf("%d", &year);
            printMovieYear(list, year);
         }
         else if (choice == 2) {
            printMovieHighestRating(list);
         }
         else if (choice == 3) {
            char *language = calloc(21, sizeof(char));
            printf("Enter the language for which you want to see the movies: ");
            scanf("%s", language);
            printMovieLanguage(list, language);
            
         }
      }
   } while(choice != 4);
   
   return EXIT_SUCCESS;
}
