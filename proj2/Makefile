#########################################
## 	Martin Vasko 1.BIT FIT VUTBR	#####
#	Date of creation: 24.4.2016		#####
#	projekt c 2 IOS = roller coaster ####
##	usage= make, make clean, make all ###
#########################################

GNUC=-std=gnu99 -Wall -Wextra -Werror -pedantic

all:proj2

#-pthread is used for working with semaphores
proj2: proj2.c proj2.h
	gcc $(GNUC) proj2.c -o proj2 -pthread

clean:
	rm -f proj2

zip:
	zip xvasko12.zip *.c *.h Makefile


