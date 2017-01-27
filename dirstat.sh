#!/bin/sh

######################################
###Martin Vasko 1 FIT.BIT	    ##
###projekt 1 IOS	2015-2016   ##
##Vysoke uceni technicke v Brne     ##
## Regular expressions everywhere ! ##
######################################

exec 2> /dev/null
#seting Posix standard
LC_ALL="POSIX"
export LC_ALL

median()
{
pom=$1
num=$2
pop=$3
REGEX=$4
if [ "$5" != "" ];then
	EXT=$5
else
	EXT="*"
fi

if [ $pom -eq 1 ];then
    pom=$((num/2))
    pom=$((pom + 1))
    file=$(find $pop -type f| grep -Ev "$REGEX" | grep -E "[A-z0-9]+\.$EXT$")
	MS=$(ls -l $file | awk '{print $5}' | sort -n | tr '\n' '|' |cut -d'|' -f$pom)
else
    pom=$((num/2)) #middle-left number
	file=$(find $pop -type f | grep -Ev "$REGEX" |grep -E "[A-z0-9]+\.$EXT$")
	NUM=$(ls -l $file |awk '{print $5}'| sort -n| tr '\n' '|' |cut -d'|' -f$pom)
	pom=$((pom+1)) #middle-right number
	MS=$(ls -l $file | awk '{print $5}' |sort -n | tr '\n' '|'| cut -d'|' -f$pom)
	MS=$(echo "$NUM $MS" |awk '{print int(($1+$2)/2)}') #sum
fi
echo $MS
}
################################################################
DIRSTAT()
{
DIR=$1
if [ "$2" != "" ];then
	REGEX=$2
else
	REGEX=";"
fi
cd $DIR
ALLDIR=$(find . -type d)
ALLFILES=$(find . -type f)

# ignore regular expression dir
O_IFS=$IFS
IFS=$'\n'

ALLDIR=$(for path in $ALLDIR;do
	new=$(echo $path | tr '/' '\n')
	new=$(echo "$new" | grep -E "$REGEX")
	if [ "$new" == "" ];then
		echo $path
	fi
	done)

IFS=$O_IFS
## end of ingoring dir regular expression

SCAN_SUCCESS="$?"
if [ $SCAN_SUCCESS -eq 1 ] # fail
then
    echo "Error, could not find files!"
    exit 1
fi
###### declaration
ND=$(echo "$ALLDIR" | wc -l| tr -d ' ')
DD=$(echo "$ALLDIR" | awk -F/ '{print NF}' | sort -n | tail -1)

if [ -z "$ALLFILES" ];then
	AF=0
	NF=0
	MS=N/A
    AS=N/A
    LF=N/A
else
	AF=$(echo "$ALLFILES" | grep -Ev "$REGEX" |wc -l )
	AF=$(echo "$AF $ND" | awk '{print int($1/$2)}')
	NF=$(echo "$ALLFILES" | grep -Ev "$REGEX" | wc -l | tr -d ' ')

	#regular files
	pom=$((NF % 2)) #condition of Median
	MS=$(median $pom $NF $DIR $REGEX)
	#maximum size
	file=$(echo "$ALLFILES" | grep -Ev "$REGEX")
	LF=$(ls -l $file | awk '{print $5}'| sort -n | tail -1)
	#average size conversion to int
	AS=$(echo "$ALLFILES" | grep -Ev "$REGEX")
	AS=$(ls -l $AS | awk '{print $5}' | awk '{s+=$1} END {print s}')
	AS=$(echo "$AS $NF" | awk '{print int($1/$2)}')
  #end of condition
fi
#file extension filter

EL=$( echo "$ALLFILES" | grep -Ev "$REGEX" | grep -oE "([A-z0-9]\.([A-z0-9])+)$" | cut -d'.' -f2) #all ext

EL=$(echo "$EL" | sort -u)
EL=$(echo $EL | tr ' ' ',' )

#Output of script
echo "Root directory: $DIR"
echo "Directories: $ND"
echo "Max depth: $DD"
echo "Average no. of files: $AF"
echo "All files: $NF"
echo " Largest file: $LF"
echo " Average file size: $AS"
echo " File size median: $MS"
echo "File extensions: $EL"
EL=$(echo $EL | tr ',' ' ')
#extension echo
for EXT in $EL;do
	LEXT=0
	AEXT=0
	file=$(echo "$ALLFILES" | grep -Ev "$REGEX" | grep -E "[A-z0-9]+\.$EXT$")
	NEXT=$(echo $file | wc -w | tr -d ' ')
	if [ $NEXT -eq 0 ];then
		continue
	else
		for des in $file; do
			FILESIZE=$(ls -l $des | awk '{print $5}')
			AEXT=$((AEXT + FILESIZE))
		done
		file=$(echo "$ALLFILES" |grep -Ev "$REGEX" | grep -E "[A-z0-9]+\.$EXT$")
		LEXT=$(ls -l $file | awk '{print $5}' | sort -nr | head -n 1)
		pom=$((NEXT % 2))
		MEXT=$(median $pom $NEXT $DIR $REGEX $EXT)
		AEXT=$((AEXT / NEXT))
		echo "Files .$EXT: $NEXT"
		echo " Largest file .$EXT: $LEXT"
		echo " Average file size .$EXT: $AEXT"
		echo " File size median .$EXT: $MEXT"
	fi
done
EL=$(echo $EL | tr ' ' ',') # changing EL to echo state
}
####################################################
#arguments passing to script
if [ -d "$1" ];
	then
		DIR=$1
		DIRSTAT $DIR
elif [ -n "$1" ];
	then
		string1=-i
		string2=--ignore
		#-i | --ignore set as 1st argument
		if [[ "$1" == $string1 || "$1" == $string2 ]];then
			if [ -n "$2" ];then
				FILE_ERE=$2
			else
				echo "No regex set !"
				exit 1
			fi
			if [ -n "$3" ];then
				DIR=$3
			else
				DIR=$(pwd)
			fi
			if [ "$FILE_ERE" != "$cwd" ];then
				if [ "$FILE_ERE" == "$DIR" ];then
					echo "Could not ignore root file!"
					exit 1
				fi
				DIRSTAT $DIR $FILE_ERE
				exit 0
			else
				usage
				exit 1
			fi
		fi
#no arguments set
else
	DIR=$(pwd)
	DIRSTAT $DIR
fi
exit 0
#succesfull script
