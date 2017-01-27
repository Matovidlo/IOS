/* ************************************************** */
/* Martin Vasko , xvasko12 FIT 1.BIB 1.BIT VUTBR 	  */
/* Projekt 2, Operation systems. Synchronization	  */
/* Description : This is C-code of synchronization    */
/* problem called roller coaster. We have basic		  */
/* elements as passenger and car. Our task is to sync */
/* those elements in term of boarding unboarding	  */
/* writing in file, mutual exlusion and simulation	  */
/* of waiting and unboarding queue. I used POSIX sem  */
/* ************************************************** */
#include"proj2.h"
//error msg
void error_msg(const char *messg)
{
	fprintf(stderr,"%s",messg);
}

/* formated output of car:  @params output 	: output file proj2.out
									mutex	: semaphore which makes output in order
									message	: formated text that should be written in output file
									my_data	: is structure of shared_data which are mapped in memory
							description: Function prints on stdout file "proj2.out" exclusively	*/

void proj2_car_output(FILE *output, sem_t *mutex, const char *message, TShared_data *my_data)
{
	sem_wait(mutex);
	setbuf(output,NULL);
	fprintf(output,message,my_data->number_of_actions++);
	sem_post(mutex);
}

/* open file proj2.out to write state  if fails returns NULL pointer */
FILE *open_output(const char *name)
{
	FILE *output;
	output=fopen(name,"w");
	if(output==NULL)
		return NULL;
	return output;
}

/* unmap data in shared memory */

void dealoc_data(int identity,TShared_data *data)
{
	shmdt(data);
	shmctl(identity,IPC_RMID, NULL);
}

/* parameters filter : params argc, argv
 			every fail in filter argument exists with STD_EXIT_CODE == 1
 			else gives params struct in which are types of argument set
			fe: passenger_count = argv[1] ...RT = argv[4]	*/
Tparameters filter(int argc, char *argv[])
{
	/* Init */
	Tparameters params=
	{
		.passenger_count=0,
		.car_size=0,
		.PT=0,
		.RT=0,
	};
	/* setting buffering to  NULL*/
	setbuf(stderr,NULL);

	if (argc < 5)
	{
		error_msg("Not enough arguments\n");
		exit(STD_EXIT_CODE);
	}
	else if (argc == 2)
	{
		if(strcmp(argv[1],"-h")== 0)
		{
			error_msg(help_mes);
			exit(SUCCESS_EXIT_CODE);
		}
		else
			error_msg("Bad parameters was given!\n");
		exit(STD_EXIT_CODE);
	}
	else if (argc == 5)
	{
		/* Passenger count */
		if (isdigit(argv[1][0]))
		{
			params.passenger_count=strtol(argv[1],NULL,10);
			if (params.passenger_count == 0)
			{
				error_msg("Passenger count is zero!\n");
				exit(STD_EXIT_CODE);
			}
		}
		else
		{
			error_msg("Passenger count is invalid!\n");
			exit(STD_EXIT_CODE);
		}
		/* Car size */
		if (isdigit(argv[2][0]))
		{
			params.car_size=strtol(argv[2],NULL,10);
			if(params.car_size == 0)
			{
				error_msg("Size of car is zero!\n");
				exit(STD_EXIT_CODE);
			}
		}
		else
		{
			error_msg("Car size is invalid!\n");
			exit(STD_EXIT_CODE);
		}
		/* PT */
		if (isdigit(argv[3][0]))
		{
			params.PT=strtol(argv[3],NULL,10);
			if (params.PT < 0 || params.PT > 5000)
			{
				error_msg("Value out of range in generating new processes!\n");
				exit(STD_EXIT_CODE);
			}
		}
		else
		{
			error_msg("Value of passenger generator invalid!\n");
			exit(STD_EXIT_CODE);
		}
		/* RT */
		if (isdigit(argv[4][0]))
        {
            params.RT=strtol(argv[4],NULL,10);
            if (params.RT < 0 || params.RT > 5000)
            {
                error_msg("Value out of range in car running time!\n");
                exit(STD_EXIT_CODE);
            }
        }
        else
        {
            error_msg("Value of car runtime invalid!\n");
            exit(STD_EXIT_CODE);
        }
	}
	/* P is multiply of C and P is > than C */
	/* return STD_EXIT_CODE == 1 */
	if((params.passenger_count <= params.car_size) || ((params.passenger_count % params.car_size)!= 0))
	{
		error_msg("Passenger count does not fit the ");
		error_msg("specification or car_size is too big!\n");
		exit(STD_EXIT_CODE);
	}
	return params;
}
/* processes of car and paassengers */

/************************************************************/
/* main code: 	@param: argc - number of arguments 			*/
/* 				@param: argv - content of argument			*/
/* description: Starting with parsing arguments				*/
/* 				Then map a shared memory create semaphores	*/
/* 				fork processes and call functions above to 	*/
/* 				write output to file named "proj2.out"		*/
/************************************************************/
int main(int argc,char *argv[])
{
	Tparameters params = filter(argc,argv);
	setbuf(stderr,NULL);

	/* Alocation of shared memory and initialization of semaphores */
	int identity;
	key_t KEY=ftok(shmKEY,getpid());
	TShared_data *my_data;
	/* characteristic key of mine ID on merlin.fit.vutbr.cz */
	/* checking if KEY identity could be created or not */
	identity = shmget(KEY, sizeof(TShared_data), IPC_CREAT | 0666);
	if(identity == -1)
	{
		error_msg("Could not create identification for shared memory!\n");
		return SYS_EXIT_CODE;
	}
	my_data = (TShared_data*)shmat(identity,NULL,0);

	/* initialization of mapped struct */
	/* free place in car , size of car_size = second argument */
	my_data->number_of_actions = 1;
	my_data->free_place = params.car_size;
	my_data->board_order = 1;
	my_data->unboard_order = 1;
	/* semaphores initialization */
	sem_t *sem_started;
	sem_t *load;
	sem_t *board_queue;
	sem_t *unboard_queue;
	sem_t *all_aboard;
	sem_t *mutex;

	/* 	those sem_open codes is when semaphore could not be create and SEM_FAILED
		was given as exit code . Exiting by SYS_EXIT_CODE == 2 ; start = 1 load = 0, queue = 0 mutex = 1 */
	if ((sem_started = sem_open(START, O_CREAT, S_IRUSR | S_IWUSR, 1)) == SEM_FAILED)
	{
		dealoc_data(identity,my_data);
		error_msg(sem_error);
		return SYS_EXIT_CODE;
	}

	if ((load = sem_open(LOAD, O_CREAT, S_IRUSR | S_IWUSR, 0)) == SEM_FAILED)
	{
		dealoc_data(identity,my_data);
		sem_close(sem_started);
		sem_unlink(START);
		error_msg(sem_error);
		return SYS_EXIT_CODE;
	}

	if ((board_queue = sem_open(BOARD, O_CREAT, S_IRUSR | S_IWUSR, 0)) == SEM_FAILED)
	{
		dealoc_data(identity,my_data);
		sem_close(sem_started);
		sem_close(load);
		sem_unlink(START);
		sem_unlink(LOAD);
		error_msg(sem_error);
		return SYS_EXIT_CODE;
	}

    if ((unboard_queue = sem_open(UNBOARD, O_CREAT, S_IRUSR | S_IWUSR, 0)) == SEM_FAILED)
    {
		dealoc_data(identity,my_data);
		sem_close(sem_started);
		sem_close(load);
		sem_close(board_queue);
		sem_unlink(START);
		sem_unlink(LOAD);
		sem_unlink(BOARD);
		error_msg(sem_error);
		return SYS_EXIT_CODE;
    }

	if ((all_aboard = sem_open(RUN, O_CREAT, S_IRUSR | S_IWUSR, 1)) == SEM_FAILED)
    {
		dealoc_data(identity,my_data);
		sem_close(sem_started);
		sem_close(load);
		sem_close(board_queue);
		sem_close(unboard_queue);
		sem_unlink(START);
		sem_unlink(LOAD);
		sem_unlink(BOARD);
		sem_unlink(UNBOARD);
		error_msg(sem_error);
		return SYS_EXIT_CODE;
    }

	if((mutex = sem_open(MUTEX, O_CREAT, S_IRUSR | S_IWUSR, 1)) == SEM_FAILED)
	{
		dealoc_data(identity,my_data);
		sem_close(sem_started);
		sem_close(load);
		sem_close(board_queue);
		sem_close(unboard_queue);
		sem_close(all_aboard);
		sem_unlink(START);
		sem_unlink(LOAD);
		sem_unlink(BOARD);
		sem_unlink(UNBOARD);
		sem_unlink(RUN);
		error_msg(sem_error);
		return SYS_EXIT_CODE;
	}
	/* end of creation of semaphores */
	/* open output for processes to be used*/
	FILE *output=open_output("proj2.out");
	if(output==NULL)
	{
		error_msg("Could not open file for writing!\n");
		dealoc_data(identity,my_data);
		sem_close(sem_started);
		sem_close(load);
		sem_close(board_queue);
		sem_close(unboard_queue);
		sem_close(all_aboard);
		sem_close(mutex);
		sem_unlink(START);
		sem_unlink(LOAD);
		sem_unlink(BOARD);
		sem_unlink(UNBOARD);
		sem_unlink(RUN);
		sem_unlink(MUTEX);
		return STD_EXIT_CODE;
	}
	/* Initialization of values of fork */
	int rand;   /* Random time to sleep PT RT */
    pid_t car_pid;  /* ID of car */
    pid_t passenger[params.passenger_count]; /* ID processes of passengers */

    car_pid=fork();
	/* forked process code start */
	if(car_pid < 0)
	{
	    dealoc_data(identity,my_data);
        sem_close(sem_started);
        sem_close(load);
        sem_close(board_queue);
        sem_close(unboard_queue);
        sem_close(all_aboard);
		sem_close(mutex);
        sem_unlink(START);
        sem_unlink(LOAD);
        sem_unlink(BOARD);
       	sem_unlink(UNBOARD);
       	sem_unlink(RUN);
		sem_unlink(MUTEX);
		fclose(output);
		error_msg("Car could not be created!\n");
		return SYS_EXIT_CODE;
	}
	/* Car process was created */
	else if(car_pid==0) //process Car was created
	{
		/* cannot start when passengers inside or they started */
		sem_wait(all_aboard);
		sem_wait(sem_started); /* */

		proj2_car_output(output,mutex,"%d\t: C 1\t: started\n",my_data);

		sem_post(sem_started);
		/* started process of car */
		while((params.passenger_count/params.car_size))
		{
			fflush(output);
			/* load */
			proj2_car_output(output,mutex,"%d\t: C 1\t: load\n",my_data);

			sem_post(board_queue);
			/* boarding */
			sem_wait(all_aboard); /* Wait for Run */
			/* Running */
			fflush(output);
			proj2_car_output(output,mutex,"%d\t: C 1\t: run\n",my_data);

			if(params.RT != 0)
				rand = random() % params.RT + 1;
			else
				rand = 0;
			usleep(1000*rand); //Wait
			fflush(output);
			proj2_car_output(output,mutex,"%d\t: C 1\t: unload\n",my_data);

			/* Unboarding passengers */
			sem_post(unboard_queue); // enable unboard
			sem_wait(load);/* wait for load */

			my_data->board_order = 1;
			my_data->unboard_order = 1;
			params.passenger_count-=params.car_size; //next iteration
		}
		proj2_car_output(output,mutex,"%d\t: C 1\t: finished\n",my_data);
		/* end of car procces*/
		sem_close(sem_started);
		sem_close(load);
		sem_close(board_queue);
		sem_close(unboard_queue);
		sem_close(all_aboard);
		sem_close(mutex);
		sem_unlink(START);
		sem_unlink(LOAD);
		sem_unlink(BOARD);
		sem_unlink(UNBOARD);
		sem_unlink(RUN);
		sem_unlink(MUTEX);
		dealoc_data(identity,my_data);
		fclose(output);
		waitpid(car_pid, NULL ,0);//exiting proccess
		return SUCCESS_EXIT_CODE;
	}
	else if (car_pid > 0)
	{
		int i;
		/* loop for creating i passengers which are stored in array of pid_t passengers[i] */
		for (i = 0; i <= params.passenger_count; i++)
		{
			if(params.PT != 0)
			{
				rand = random() % params.PT + 1;
			}
			else
				rand = 0;
			usleep(rand*1000);
			pid_t generating_passengers=fork();
			/* 	fork failed , processes created before have to be terminated
				SIGTERM signal is sended */
			if(generating_passengers < 0)
			{
				/* killing processes*/
				for (int j = 0 ; j < params.passenger_count; j++)
					kill(passenger[j],SIGTERM);
				free(passenger);
				kill(car_pid,SIGTERM);
				/* error message and exiting all semaphores */
				sem_wait(mutex);
				error_msg("Generating of passengers failed!\n");
        		sem_post(mutex);
				/* mutex call */
				sem_close(sem_started);
        		sem_close(load);
        		sem_close(board_queue);
       			sem_close(unboard_queue);
       			sem_close(all_aboard);
				sem_close(mutex);
       			sem_unlink(START);
       			sem_unlink(LOAD);
        		sem_unlink(BOARD);
        		sem_unlink(UNBOARD);
        		sem_unlink(RUN);
				sem_unlink(MUTEX);
        		dealoc_data(identity,my_data);
				fclose(output);
				return SYS_EXIT_CODE;
			}
			else if(generating_passengers == 0)
			{
				/* while creating passengers wait for this code be executed */
				/* Starting semaphore with Started message */
				sem_wait(sem_started);
				/* exclusive output of started Passenger */
				sem_wait(mutex);
				setbuf(output,NULL);
				fprintf(output,"%d\t: P %d\t: started\n",my_data->number_of_actions++,i+1);
				sem_post(mutex);

				sem_wait(board_queue);
				sem_post(sem_started);

				sem_wait(mutex);
				fflush(output);
				fprintf(output,"%d\t: P %d\t: board\n",my_data->number_of_actions++,i+1);
				sem_post(mutex);
				/* car is free putting passengers into car in order and last */
				if(my_data->free_place > 1)
				{
					sem_wait(mutex);
					fflush(output);
					fprintf(output,"%d\t: P %d\t: board order %d\n",my_data->number_of_actions++, i+1, my_data->board_order++);
					sem_post(mutex);
				}
				else if(my_data->free_place == 1)
				{
					sem_wait(mutex);
					fflush(output);
					fprintf(output,"%d\t: P %d\t: board last\n",my_data->number_of_actions++,i+1);
					sem_post(mutex);
				}
				//decrement car size to 0
				my_data->free_place--;
				if(my_data->free_place == 0)
					sem_post(all_aboard);
				else
					sem_post(board_queue);

				sem_wait(unboard_queue); // Wait for unboard
				/* Started unboarding */
				sem_wait(mutex);
				fflush(output);
				fprintf(output,"%d\t: P %d\t: unboard\n", my_data->number_of_actions++, i+1);
				/* still passenger in car writing order or last */
				if ( my_data->free_place < params.car_size -1 )
				{
					fflush(output);
					fprintf(output,"%d\t: P %d\t: unboard order %d\n",my_data->number_of_actions++,i+1,my_data->unboard_order++);
				}
				else if (my_data->free_place == params.car_size -1)
				{
					fflush(output);
					fprintf(output,"%d\t: P %d\t: unboard last\n",my_data->number_of_actions++,i+1);
				}
				//set free car size
				my_data->free_place++;
				sem_post(mutex);
				/* post loading semaphore if all ashore or unboard queue */
				if(my_data->free_place == params.car_size)
					sem_post(load);
				else
					sem_post(unboard_queue);

				/* dealocation of all sources of forked processes */
				sem_close(sem_started);
				sem_close(load);
				sem_close(board_queue);
				sem_close(unboard_queue);
				sem_close(all_aboard);
				sem_close(mutex);
				sem_unlink(START);
				sem_unlink(LOAD);
				sem_unlink(BOARD);
				sem_unlink(UNBOARD);
				sem_unlink(RUN);
				sem_unlink(MUTEX);
				dealoc_data(identity,my_data);
				//killing generator of proccesses
				waitpid(generating_passengers, NULL, 0);
				return SUCCESS_EXIT_CODE;
				/* successfully ended passenger/s */
			}
			else //child created array of passengers fill with forked child "generating_passengers"
			{
				passenger[i] = generating_passengers;
				/* do nothing with this passenger */
			}
		}
	}
	/* fork was successfull all processes of car finished */

	for (int j = 0 ; j < params.passenger_count ; j++)
		waitpid(passenger[j], NULL,0); /* kill all passengers */
	for (int j = 0 ; j < params.passenger_count ; j++)
	{
		setbuf(output,NULL);
		fprintf(output,"%d\t: P %d\t: finished\n",my_data->number_of_actions++, j+1);
	}
	shmdt(my_data);
	fclose(output);
	return SUCCESS_EXIT_CODE; /* end of all processes finished and killed everything memory cleared */
	}


