Nikita Malhotra
Student No: V00790483
CSC 360: Operating Systems
Assignment #3: To implement utilities that perform operations on a simple file system, FAT12, used by MS-DOS.

The purpose of this assignment is to implement utilities that perform operations on a simple file system, FAT12, which is used by MS-DOS. There are 5 other documents, along with this one, enclosed in the p3.tar.gz file. The files are:

1. diskinfo.c
2. disklist.c
3. diskget.c
4. diskput.c
5. Makefile 

To run the all the .c files above, type make on the command line and this will invoke the makefile that will compile the all the .c files into executables:

1. diskinfo
2. disklist
3. diskget
4. diskput 

All the executable files can be executed as follows:
1. To run diskinfo.c -> type ./diskinfo disk2.IMA on the command line.
2. To run disklist.c -> type ./disklist disk2.IMA on the command line.
3. To run diskget.c -> type ./disklist disk2.IMA <filename.ext> on the command line, where
   <filename.ext> is any file that is on disk2.IMA
4. To run diskput.c -> type ./diskput disk2.IMA <filename.ext> on the command line, where 
   <filename.ext> is any file that is in the current directory which you would like to copy onto 
   disk2.IMA  

Note: The file diskput.c has been coded but it is not complete. Some of the code has been commented out to aid with the makefile. 