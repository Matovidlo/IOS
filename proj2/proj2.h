/* basic libraries */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
/* fork */
#include<unistd.h>
/* mmap munmap */
#include<sys/shm.h>
#include<sys/types.h>

/* semaphores */
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/wait.h>
#include<sys/sem.h>
#include<semaphore.h>
#include<signal.h>
#include<ctype.h>

#define SYS_EXIT_CODE 2
#define STD_EXIT_CODE 1
#define SUCCESS_EXIT_CODE 0

#define START "START"
#define LOAD "LOAD"
#define BOARD "BOARD"
#define UNBOARD "UNBOARD"
#define RUN "RUN"
#define MUTEX "MUTEX"

/* Semaphore creation failed */
const char *sem_error=
"Semaphore creation failed!\n";

/* HELP message */
const char *help_mes=
"Program: Roller Coaster\n"
"Author:Martin Vasko , xvasko12 FIT VUTBR 1.BIT BIB"
"Usage: proj2 -h\n"
"		proj2 P C PT RT\n"
"Description of paramters:P	= number of processes of passengers(P > 0)\n"
"						  C	= capacity of car (C > 0 P > C, P have to be multiple of C)\n"
"						  PT= maximum period of generating passenger/s [ms] (0 <= PT < 5001)\n"
"						  RT= maximum period of car running [ms] (0 <= RT < 5001)\n";

/* Structure of shared data */
typedef struct data{
int number_of_actions;
int free_place;
int board_order;
int unboard_order;
}TShared_data;

typedef struct params
{
int passenger_count;
int car_size;
int PT;
int RT;
}Tparameters;

/* Key and size for mapping shared memory blocks */
#define shmKEY "xvasko12"
#define dataSIZE sizeof(TShared_data)

/* Algorithm of roller coaster */
void dealoc_data(int identity,TShared_data *data);
void passengers();//generating passengers
void car();
Tparameters filter(int argc,char *argv[]);
/* working with files */
void error_msg(const char *messg);
FILE *open_output(const char *name);
