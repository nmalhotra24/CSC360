// Nikita Malhotra
// Student No: V00790483
// CSC 360: Operating Systems
// Assignment 3: To implement utilities that perfrom operations on a simple file
//               system, FAT12, used by MS-DOS.

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h> // mmap library
#include <fcntl.h> // open library
#include <string.h>
#include <errno.h>

typedef struct DiskFile {
    char* name;
    char* extension;
    int filesize, logical_sector;
} DiskFile;

// ******************************************************************************
int getTotalSize (char* mmap)
{
    int* temp1 = malloc(sizeof(int));
    int* temp2 = malloc(sizeof(int));
    int value;
    // mmap to the 19th and 20th byte
    *temp1 = mmap[19];
    *temp2 = mmap[20];
    
    // apply Little Endian
    value = *temp1 + ((*temp2) << 8);
    
    free(temp1);
    free(temp2);
    
    return value;
}

// ******************************************************************************
/*
void Replace_File (char* filename, char* mmap)
{
    int sector = (mmap[current + 26] & 0x00FF) + ((mmap[current + 27] & 0x00FF) << 8);
    int filesize = (mmap[current + 28] & 0x00FF) + ((mmap[current + 29] & 0x00FF) << 8) + ((mmap[current + 30] & 0x00FF) << 16) + ((mmap[current + 31] & 0x00FF) << 24);
    int counter = 0;
	int sectCount = 1;
	FILE* fp = fopen(fileName, "r");
	while(!feof(fp)){
        int base = 512*(sector + 31);
 while(counter < 512*sectCount && (!feof(fp))){
 //memcpy((void*)mmap[base+counter],(void*)fgetc(fp),1);
 //mmap[base + counter] = fgetc(fp);
 counter ++;
 }
 sectCount++;
 sector = getNextSector(mmap,sector);
	}
	fclose(fp);
 }
*/

// ******************************************************************************
/* 
void Write_File (char* mmap, char* arg_name)
 */

// ******************************************************************************
int getFreeSpace (char* mmap, int numSectors)
{
    int n = 2; // logical number of the first sector in the Data Area
    int base = 512; // the first bye of the FAT table
    int* temp1 = malloc(sizeof(int));
    int* temp2 = malloc(sizeof(int));
    int result = 0;
    int counter = 0;
    
    // The logical number for all the sectors in the Data Area is from 2 to 2484
    for (n = 2; n <= (numSectors - 32); n++)
    {
        // if the logical number is even
        if (n % 2 == 0)
        {
            // get all 8 bits
            *temp1 = mmap[base + 3 * n/2];
            *temp2 = mmap[base + 1 + 3 * n/2];
            *temp2 = *temp2 & 0x0F; // get the low 4 bits
            
            // apply "Little Endian"
            result = ((*temp2) << 8) + *temp1;
            
        } else // the logical number is odd
        {
            // get all 8 bits
            *temp1 = mmap[base + 3 * n/2];
            *temp2 = mmap[base + 1 + 3 * n/2];
            *temp1 = *temp1 & 0xF0; // get the low 4 bits
            
            // apply "Little Endian"
            result = ((*temp1) << 8) + *temp2;
        }
        
        // check if the value is 0x00 --> sector is free/unused
        if (result == 0x00)
        {
            counter++;
        }
    }
    
    free(temp1);
    free(temp2);
    
    return counter;
}

// ******************************************************************************
int getSector (char* mmap, int currSector)
{
    int n = currSector; // logical number of the first sector in the Data Area
    int base = 512; // the first bye of the FAT table
    int* temp1 = malloc(sizeof(int));
    int* temp2 = malloc(sizeof(int));
    int input, result = 0;
    
    // if the logical number is even
    if (n % 2 == 0)
    {
        // get all 8 bits
        *temp1 = (mmap[base + 3 * n/2] & 0x00FF);
        *temp2 = (mmap[base + 1 + 3 * n/2] & 0x00FF);
        *temp2 = *temp2 & 0x0F; // get the low 4 bits
        
        // apply "Little Endian"
        result = ((*temp2) << 8) + *temp1;
        
    } else // the logical number is odd
    {
        // get all 8 bits
        *temp1 = (mmap[base + 3 * n/2] & 0x00FF);
        *temp2 = (mmap[base + 1 + 3 * n/2] & 0x00FF);
        *temp1 = ((*temp1 & 0xF0) >> 4); // get the low 4 bits
        
        // apply "Little Endian"
        result = *temp1 + ((*temp2) << 4);
    }
    
    free(temp1);
    free(temp2);
    return result;
}

// ******************************************************************************
void Check_FileName (char* mmap, char* filename, char* arg_name)
{
    int base = 9728;  // The first byte of the root directory
    int current = base;   // Point to the first byte of the current entry
    int offset = 32;  // Each entry has 32 bytes in root directory
    int* temp1 = malloc(sizeof(int));
    int* temp2 = malloc(sizeof(int));
    DiskFile* file = calloc(1, sizeof(DiskFile));
    char input;
    
    *temp1 = mmap[current];
    while(*temp1 != 0x00 && current < (33 * 512))
    {
        char* whole_file_name = calloc(14,sizeof(char));
        // Check if temp1 is not empty --> 0xE5
        if (*temp1 != 0xE5)
        {
            *temp2 = mmap[current + 11];
            // Check if temp2 is not a volume label --> 0x08
            if (*temp2 != 0x08)
            {
                // Check if temp2 is not part of a long file name --> 0x0F
                if (*temp2 != 0x0F)
                {
                    file -> name = calloc(1, sizeof(char)*9); // Times 9 because 8 bytes + null
                    file -> extension = calloc(1, sizeof(char)*4); // Times 4 because 3 bytes + null
                    
                    int i = 0;
                    for (; i < 8; i++)
                    {
                        file -> name[i] = mmap[current + i];
                        // If the file name is a space then change to null termination character
                        if (file -> name[i] == ' ')
                        {
                            file -> name[i] = '\0';
                            break;
                        }
                    }
                    
                    for (; i < 11; i++)
                    {
                        file -> extension[i - 8] = mmap[current + i];
                    }
                    
                    // Null termination
                    file -> name[8] = '\0';
                    file -> extension[3] = '\0';
                    // At this moment, we have found the name, type and extension of the file
                    strcat(whole_file_name, file -> name);
                    strcat(whole_file_name, ".");
                    strcat(whole_file_name, file -> extension);
                    
                    if(!strcmp(whole_file_name, filename))
                    {
                       printf("The file with the same name in the disk will be replaced! Continue (Y/N)? ");
                        scanf("%c",&input);
                        while (input != 'N' && input != 'n' && input != 'Y' && input != 'y'){
                            printf("Please enter Y or N: ");
                            scanf(" %c",&input);
                        }
                        if (input == 'N' || input == 'n'){
                            exit(0);
                        }
                        if (input == 'Y' || input == 'y'){
                            // replace file
                            // Replace_File(fileName, mmap);
                            // break;
                        }
                    }
                    // else
                    // {
                        // write the file to the disk
                        // Write_File (mmap, arg_name)
                    // }
                }
            }
        }
        
        // Go to next entry in Root Directory
        current = current + offset;
        *temp1 = mmap[current];
        free(whole_file_name);
    }

    // Clean up
    free(file);
    free(temp1);
    free(temp2);
}

// ******************************************************************************
int main (int argc, char** argv)
{
    int fd;
    struct stat sf;
    char* p;
    
    if ((fd = open (argv[1], O_RDONLY)))
    {
        fstat(fd, &sf);
        p = mmap(NULL, sf.st_size, PROT_READ, MAP_SHARED, fd, 0);
        // get the size of the disk
        int size = getTotalSize(p);
        // get the free size of the disk
        int free_space = getFreeSpace(p, size);
        
        if (free_space == 0)
        {
            printf ("Not enough free space in the disk image\n");
            return 0;
        } else
        {
            // check the file name
            Check_FileName(p,argv[2],argv[1]);
        }
    } else
    {
        printf ("Fail to open the image file.\n");
    }
    
    // Clean up
    close(fd);
    return 0;
}
