OBJ = user_process.o Initialization.o queue.o kernel.o iprocesses.o User_API.o RTX.o
CC  = gcc

all: RTX-G30 KB CRT

RTX-G30: $(OBJ)
	$(CC) $(OBJ) -o "RTX-G30" 

CRT: CRT.o 
	$(CC) CRT.c -o "CRT"
	
KB: keyboard.o  
	$(CC) keyboard.c -o "KB"

Initialization.o : Initialization.c Initialization.h globals.h 
	$(CC) -c Initialization.c
	
RTX.o : RTX.c globals.h Initialization.h
	$(CC) -c RTX.c

queue.o : queue.c queue.h globals.h User_API.h iprocesses.h
	$(CC) -c queue.c
	
CRT.o : CRT.c globals.h
	$(CC) -c CRT.c 

keyboard.o : keyboard.c globals.h
	$(CC) -c keyboard.c

user_process.o : user_process.c user_process.h globals.h  
	$(CC) -c user_process.c

iprocesses.o : iprocesses.c iprocesses.h globals.h 
	$(CC) -c iprocesses.c

kernel.o : kernel.c kernel.h globals.h queue.h 
	$(CC) -c kernel.c

User_API.o : User_API.c User_API.h globals.h 
	$(CC) -c User_API.c
	
clean:
	rm $(OBJ) CRT.o keyboard.o RTX-G30 KB CRT 
	