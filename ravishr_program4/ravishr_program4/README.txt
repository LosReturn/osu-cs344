The folder submitted, on canvas, is named ravishr_program4. Once you navigate into this folder: 

The files in this folder are:
	1. main.c : this is the file with the program code.
	2. line_processor : this is the executable.

The instructions to compile and run the program (main.c): 
	1. Open terminal and navigate into the ravishr_program4 directory.
	2. Compile main.c and generate executable file called line_processor by entering in terminal: 
		$ gcc —std=gnu99 -pthread -o line_processor main.c
	3. Run the executable by entering in the terminal:
		1. For standard input from keyboard and standard output to terminal:
			$ ./line_processor
		2. For standard input from file and standard output to terminal, (example txt file
		   name used below):
			$ ./line_processor < input1.txt
		3. For standard input from keyboard and standard output to file, (example txt file
		   name used below):
			$ ./line_processor > output1.txt
		4. For standard input from file and standard output to file, (example txt file
		   names used below):
			$ ./line_processor < input1.txt > output1.txt