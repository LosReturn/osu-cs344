The folder submitted, on canvas, is named ravishr_program5. Once you navigate into this folder: 

The files in this folder are:
	1. enc_server.c : this is the file with the encryption server code.
	2. enc_client.c : this is the file with the encryption client code.
	3. dec_server.c : this is the file with the decryption server code.
	4: dec_client.c : this is the file with the decryption client code.
	5. keygen.c : this is the file that creates the key file of specific length.
	6. compileall : this is the bash shell script that creates 5 executable programs for the files.
	7. p5testscript : this is the grading script.

The instructions to compile and run the program (main.c): 
	1. Open terminal and navigate into the ravishr_program5 directory.
	2. Run compileall and generate executable files for all my .c files : 
		$ ./compileall
	3. Run p5testscript with two random ports (numbers (between 55000 - 65000) are filled in for RANDOM_PORT1 and RANDOM_PORT2) :
		$ ./p5testscript RANDOM_PORT1 RANDOM_PORT2
