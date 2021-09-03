The folder submitted, on canvas, is named ravishr_program1. Once you navigate into this folder: 

The files in this folder are:
	1. main.c : this is the file with the program code.
	2. movies : this is the executable.
	3. movies_sample_1.csv : this is the csv file passed into the program that contains all the movie data.

The instructions to compile and run the program (main.c): 
	1. Open terminal and navigate into the ravishr_program1 directory.
	2. Compile main.c and generate executable file called movies by entering in terminal: 
		$ gcc —std=gnu99 -o movies main.c
	3. Run the executable and pass in the csv file by entering in the terminal:
		$ ./movies movies_sample_1.csv