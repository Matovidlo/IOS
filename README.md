# IOS
operating systems FIT 1.BIT 2015-2018 Bacherol study.

1. Project - Bash script.

2. Project - Synchronization of processes in C.

1.Dirstat- Makes statistics of some directory (path).

Usage:

  without argument - Makes statistics of current folder.
  
  path - Makes statistics of path directory. If file is readed script is used as like no arguments is set.
  
  -i --ignore path/file/regex - ignores some directory/file or filter by regex.
  
2.Cars - Classic synchronization problem, where is used mutex and semaphores.

Usage:
  -h - help message
  P_Count C_Size [Passenger_wait] [Car_runtime] - 
  
      P_Count - passenger count , means how many people are going to be in queue.
      C_Size  - size of car, means how many seats is in car or how many people is 1 car able to transport.
      Passenger_wait - maximum period of generating process passenger/s [ms] (0 <= Passenger_wait < 5001).
      Car_runtime - maximum period of car running [ms] (0 <= Car_runtime < 50001)
                    
  output is available in file proj2.out if any error occurs it is displayed in file errcode
  
  errcodes- SYS_EXIT_CODE - system exit code == 2 , fail of system call (create semaphore, not enough memory to mmap some space)
            STD_EXIT_CODE - standard exit code == 1 , 
            SUCCESS_EXIT_CODE - success exit code == 0, everything is fine. Car finished and every resource is cleared.
