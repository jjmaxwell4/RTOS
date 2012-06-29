Please do not copy or reuse any part of this code. 
Please abide by Policy 71.

This real time operating system was created for MTE 241 (Real-Time Operating Systems) at the University of Waterloo. It is written in C.

This project attempts to simulate an operating system. It runs on top of the Linux kernel within a linux OS. A priority based scheduler was used, with simulated "hardward interrupts" provided by Linux. It performs context switching with setjump and longjmp.

A perfect mark was achieved on this project. 

Completed By:
James Maxwell - Team Lead
Dylan Drover
Patrick Ellsworth
Brendan Chwyl



CRT.c -62 Lines  				- Runs CRT process

globals.h	-182 Lines			- Global Header file

init_table	-8 lines			- Table used to Initialize PCB's

Initialization.c  -279 Lines	- Hold functions that Initialize iRTX

Initialization.h  -30 Lines 	- Initialization.c header file

iprocesses.c	-148 Lines		- Hold i_process functions	

iprocesses.h	-24 Lines		- i_processes.c Header File

RTX.c		-124 Lines			- Main Program. 

kernel.c	-317 Lines			- Holds kernel primitives

kernel.h	-46 Lines			- kernel.c Header File

keyboard.c	-64 Lines			- Runs KB process

Makefile	-44 Lines			- Used to compile and link files and create an executable

queue.c		-179 Lines			- Queue related functions

queue.h		-25 Lines			- queue.c Header File

README		-64 Lines			- Description/README File		

User_API.c	-155 Lines			- User API of Kernel functions

User_API.h	-39 Lines			- User_API.c header file

user_process.c	-217 Lines		- Holds Process_P

user_process.h	-23 Lines		- user_process.c Header file





Makefile: "make clean" removes object (.o) files in the directory
	
	"make" creates three executables: RTX-G30, KB, CRT
	
	"./RTX-G30" Initialize and run the RTX
