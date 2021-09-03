The folder submitted, on canvas, is named ravishr_program3. Once you navigate into this folder: 

The files in this folder are:
	1. main.c : this is the file with the program code.
	2. smallsh : this is the executable.
	3. p3testscript : this is the script file

The instructions to compile and run the program (main.c): 
	1. Open terminal and navigate into the ravishr_program3 directory.
	2. Compile main.c and generate executable file called smallsh by entering in terminal: 
		$ gcc —std=gnu99 -o smallsh main.c
	3. Run the script (this calls smallsh)
		$ ./p3testscript 2>&1