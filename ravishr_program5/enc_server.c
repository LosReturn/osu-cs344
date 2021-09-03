//
//  enc_server.c
//  ravishr_program5
//
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

// Error function used for reporting issues
void error(const char *msg) {
   perror(msg);
   exit(1);
}

// Set up the address struct for the server socket
void setupAddressStructInServer(struct sockaddr_in* address, int portNumber){
 
   // Clear out the address struct
   memset((char*) address, '\0', sizeof(*address));

   // The address should be network capable
   address->sin_family = AF_INET;
   // Store the port number
   address->sin_port = htons(portNumber);
   // Allow a client at any address to connect to this server
   address->sin_addr.s_addr = INADDR_ANY;
}

int convertChar(char c, char *characters) {
   int numVal;
   for (int i = 0; i < 27; i++) {
      if (c == characters[i]) {
         numVal = i;
         break;
      }
   }
   return numVal;
}

char *encryption(char *message, char *key, char *characters)
{
   int messageVal, keyVal, mandk, modVal, cipherVal;
   char *ciphertext = calloc(strlen(message) + 1, sizeof(char));
   for (int i = 0; i < strlen(message); i++) {
      messageVal = convertChar(message[i], characters);
      keyVal = convertChar(key[i], characters);
      mandk = messageVal + keyVal;
      if (mandk < 0)
         mandk -= 27;
      modVal = mandk % 27;
      cipherVal = characters[modVal];
      ciphertext[i] = cipherVal;
   }
   return ciphertext;
}

int main(int argc, char *argv[]){
   int encConnectionSocket, charsRead, charsWritten;
   char buffer[70000], plaintext[70000], key[70000], message[70000];
   char characters[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
   struct sockaddr_in serverAddress, clientAddress;
   socklen_t sizeOfClientInfo = sizeof(clientAddress);
   pid_t child_pid;

   // Check usage & args
   if (argc < 2) {
      fprintf(stderr,"USAGE: %s port\n", argv[0]);
      exit(1);
   }
   
   // Create the socket that will listen for connections
   int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
   if (listenSocket < 0) {
      error("ERROR opening socket");
   }

   // Set up the address struct for the server socket
   setupAddressStructInServer(&serverAddress, atoi(argv[1]));

   // Associate the socket to the port
   if (bind(listenSocket, (struct sockaddr *)&serverAddress,
            sizeof(serverAddress)) < 0){
      error("ERROR on binding");
   }

   // Start listening for connections. Allow up to 5 connections to queue up
   listen(listenSocket, 5);
   //printf("in server\n");
   // Accept a connection, blocking if one is not available until one connects
   while(1) {
      // Accept the connection request which creates a connection socket
      encConnectionSocket = accept(listenSocket, (struct sockaddr *)&clientAddress, &sizeOfClientInfo);
      if (encConnectionSocket < 0){
         error("ERROR on accept");
      }
      
      //printf("ENC_SERVER: Connected to client running at host %d port %d\n", ntohs(clientAddress.sin_addr.s_addr), ntohs(clientAddress.sin_port));
      
      // fork
      child_pid = fork();
      if (child_pid == -1) {
         error("fork() failed.");
         //break;
      }
      
      else if (child_pid == 0) {
         // Get the message from the client
         memset(buffer, '\0', sizeof(buffer));
         // Read the client's message from the socket
         charsRead = recv(encConnectionSocket, buffer, sizeof(buffer) - 1, 0);

         if (charsRead < 0){
            error("ERROR reading from socket");
         }
         
         char *token = strtok(buffer, ",");
         if (strcmp(token, "enc") != 0) {
            //fprintf(stderr, "ENC_SERVER: ERROR can communicate only with enc_client\n");
            send(encConnectionSocket, "ENC_SERVER", 10, 0);
         }
         
         else {
            token = strtok(NULL, ",");
            strcpy(plaintext, token);
            token = strtok(NULL, ",");
            strcpy(key, token);
            char *ciphertext = calloc(strlen(plaintext) + 1, sizeof(char));
            ciphertext = encryption(plaintext, key, characters);
            // Send a Success message back to the client
            //charsWritten = send(encConnectionSocket, ciphertext, strlen(ciphertext), 0);
            
            charsWritten = 0;
            while (charsWritten < strlen(ciphertext)) {
               if (charsWritten < 0){
                  error("ERROR writing to socket");
               }
               charsWritten += send(encConnectionSocket, ciphertext, strlen(ciphertext), 0);
            }
         }
         // Close the connection socket for this client
         close(encConnectionSocket);
         exit(0);
         break;
      } // end of 'else if' in while
      
      else {
         //break;
      }
      
      close(encConnectionSocket);
      
   } // end of while
   
   // Close the listening socket
   close(listenSocket);
   return 0;
}



