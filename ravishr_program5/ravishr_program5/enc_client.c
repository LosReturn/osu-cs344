//
//  enc_client.c
//  ravishr_program5
//
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()

// Set up the address struct
void setupAddressStructInClient(struct sockaddr_in* address, int portNumber,
                        char* hostname) {
 
   // Clear out the address struct
   memset((char*) address, '\0', sizeof(*address));

   // The address should be network capable
   address->sin_family = AF_INET;
   // Store the port number
   address->sin_port = htons(portNumber);

   // Get the DNS entry for this host name
   struct hostent* hostInfo = gethostbyname(hostname);
   
   if (hostInfo == NULL) {
      fprintf(stderr, "ENC_CLIENT: ERROR, no such host\n");
      exit(0);
   }
   // Copy the first IP address from the DNS entry to sin_addr.s_addr
   memcpy((char*) &address->sin_addr.s_addr, hostInfo->h_addr_list[0],
          hostInfo->h_length);
}

bool checkForBadChar(char *file, char* characters) {
   bool badChar = true;
   for (int i = 0; i < strlen(file); i++) {
      for (int j = 0; j < strlen(characters); j++) {
         if (file[i] == characters[j]) {
            badChar = false;
            break;
         }
      }
      if (badChar)
         break;
   }
   return badChar;
}

int main(int argc, char *argv[]) {
   int socketFD, portNumber, charsWritten, charsRead;
   struct sockaddr_in serverAddress;
   FILE *fplaintext, *fkey;
   char plaintext[70000], key[70000], buffer[70000], ciphertext[70000];
   char characters[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
   bool plaintextBadChar, keyBadChar;
   
   // Referenced --> Exploration: stdin, stdout, stderr & C I/O library
   fplaintext = fopen(argv[1], "r");
   fkey = fopen(argv[2], "r");
   
   if (fplaintext == NULL) {
      // send appropriate error text to stderr
      fprintf(stderr, "ERROR opening plaintext file\n");
      exit(1);
   }
   if (fkey == NULL) {
      // send appropriate error text to stderr
      // using fprintf instead of perror since it kept appending a '%'
      fprintf(stderr, "ERROR opening key file\n");
      exit(1);
   }
   fgets(plaintext, sizeof(plaintext), fplaintext);
   plaintext[strcspn(plaintext, "\n")] = '\0';
   fgets(key, sizeof(key), fkey);
   key[strcspn(key, "\n")] = '\0';
   
   if (strlen(key) < strlen(plaintext)) {
      fprintf(stderr, "ERROR key is shorter than plaintext\n");
      exit(1);
   }
   plaintextBadChar = checkForBadChar(plaintext, characters);
   keyBadChar = checkForBadChar(key, characters);
   if (plaintextBadChar || keyBadChar) {
      fprintf(stderr, "ERROR bad character\n");
      exit(1);
   }
   
   // Create a socket
   socketFD = socket(AF_INET, SOCK_STREAM, 0);
   if (socketFD < 0){
      fprintf(stderr, "ENC_CLIENT: ERROR opening socket\n");
      exit(1);
   }

   // Set up the server address struct
   setupAddressStructInClient(&serverAddress, atoi(argv[3]), "localhost");
   // Connect to server
   if (connect(socketFD, (struct sockaddr*)&serverAddress,
               sizeof(serverAddress)) < 0){
      fprintf(stderr, "ENC_CLIENT: ERROR connecting\n");
      exit(1);
   }

   // Clear out the buffer array
   memset(buffer, '\0', sizeof(buffer));
   // Stores 'enc_client', plaintext and key into buffer
   // http://www.cplusplus.com/reference/cstdio/sprintf/
   sprintf(buffer, "enc,%s,%s,", plaintext, key);
   // Send message to server
   // Write to the server
   //charsWritten = send(socketFD, buffer, strlen(buffer), 0);
   charsWritten = 0;
   while (charsWritten < strlen(buffer)) {
      if (charsWritten < 0){
         fprintf(stderr, "ENC_CLIENT: ERROR writing to socket\n");
         exit(1);
      }
      charsWritten += send(socketFD, buffer, strlen(buffer), 0);
      //fprintf(stderr, "In while loop\n");
   }
   
   if (charsWritten < sizeof(buffer)){
      fprintf(stderr, "ENC_CLIENT: WARNING: Not all data written to socket!\n");
      exit(1);
   }
   // Get return message from server
   // Clear out the buffer again for reuse
   memset(buffer, '\0', sizeof(buffer));
   
   // Read data from the socket, leaving \0 at end
   charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0);
   /*
   charsRead = 0;
   while (charsRead < strlen(plaintext)) {
      if (charsRead < 0){
         fprintf(stderr, "ENC_CLIENT: ERROR reading from socket\n");
         exit(1);
      }
      charsRead += recv(socketFD, buffer, sizeof(buffer) - 1, 0);
      strcat(ciphertext, buffer);
      memset(buffer, '\0', sizeof(buffer));
   }
    */
   if (strcmp(buffer, "DEC_SERVER") == 0) {
      fprintf(stderr, "ENC_CLIENT: ERROR connecting to wrong server at port %d\n",
              ntohs(serverAddress.sin_port));
      exit(2);
   }
   
   //printf("%s\n", ciphertext);
   printf("%s\n", buffer);
   // Close the socket
   close(socketFD);
   return 0;
}



