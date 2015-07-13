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

// ******************************************************************************
void getOSName (char* osname, char* mmap)
{
    int i;
    for (i = 0; i < 8; i++)
    {
        osname[i] = mmap[3 + i];
    }
}

// ******************************************************************************
void getLabel (char* label, char* mmap)
{
    int base = 9728; // the first byte of the root sector
    int current = base;
    int offset = 32; // every entry is 32 bytes
    int* temp1 = malloc(sizeof(int));
    int* temp2 = malloc(sizeof(int));
    int i;
    
    *temp1 = mmap[current];
    
    // does not equal to free space and no more files and still inside root directory
    while (*temp1 != 0x00 && current < (33 * 512))
    {
        // check for files
        if (*temp1 != 0xE5)
        {
            *temp2 = mmap[current + 11];
            if (*temp2 == 0x08) // volume lable
            {
                for (i = 0; i < 8; i++)
                {
                    label[i] = mmap[current + i];
                }
                break;
            }
        }
        // move to next entry in the root directory
        current = current + offset;
        *temp1 = mmap[current];
    }
    
    free(temp1);
    free(temp2);
}

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
int getFatCopy (char* mmap)
{
    int return_value = mmap[16]; // map to byte 16
    return return_value;
}

// ******************************************************************************
int sectors (char* mmap)
{
    int* temp1 = malloc(sizeof(int));
    int* temp2 = malloc(sizeof(int));
    int value;
    
    // mmap to the 22nd and 23rd byte
    *temp1 = mmap[22];
    *temp2 = mmap[23];
    
    // apply Little Endian
    value = *temp1 + ((*temp2) << 8);
    
    free(temp1);
    free(temp2);
    
    return value;
}

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
int countRootDirFiles(char* mmap)
{
    int base = 9728;  // the first byte of the root directory
    int current = base;   // point to the first byte of the current entry
    int offset = 32;  // Each entry has 32 bytes in root directory
    int* temp1 = malloc(sizeof(int));
    int* temp2 = malloc(sizeof(int));
    int counter = 0;
   
    *temp1 = mmap[current];
    
    while(*temp1 != 0x00 && current < (33 * 512))
    {
        // 0xE5 -> empty
        if (*temp1 != 0xE5)
        {
            *temp2 = mmap[current + 11];
            // check to see if not part of a: long filename: 0x0F, subdirectory: 0x10 and volume label: 0x08
            // then it is a file
            if (*temp2 != 0x0F && *temp2 != 0x08 && *temp2 != 0x10)
            {
                counter ++;
            }
        }
        
        // Go to next entry in Root Directory
        current = current + offset;
        *temp1 = mmap[current];
    }
    free(temp1);
    free(temp2);
    
    return counter;
}

// ******************************************************************************
int main (int argc, char** argv)
{
    int fd;
    struct stat sf;
    char* p;
    char* osname = malloc(sizeof(char) * 8);
    char* label = malloc(sizeof(char) * 8);
    
    if ((fd = open (argv[1], O_RDONLY)))
    {
        fstat(fd, &sf);
        p = mmap(NULL, sf.st_size, PROT_READ, MAP_SHARED, fd, 0);
        
        // get the OSName
        getOSName(osname,p);
        printf ("OS Name: %s\n", osname);
        
        // get the label of the disk
        getLabel(label,p);
        printf ("Lable of the disk: %s\n", label);
        
        // get the total size of the disk
        int size = getTotalSize(p);
        printf ("Total size of the disk: %d bytes\n", size * 512);
    
        
        // get the free size of the disk
        int free_size = getFreeSpace(p, size);
        printf ("Free size of the disk: %d bytes\n", free_size * 512);
        printf ("\n==============\n");
        
        // get the number of files in the root directory
        int num_root_dir_files = countRootDirFiles(p);
        printf ("The number of files in the root directory (not including subdirectories): %d\n", num_root_dir_files);
        printf ("\n==============\n");

        // get the number of FAT copies
        int fat_copy = getFatCopy(p);
        printf ("Number of FAT copies: %d \n", fat_copy);
        
        // get the number of sectors per FAT
        int num_sectors = sectors(p);
        printf ("Sectors per FAT: %d\n\n", num_sectors);
        
    } else
    {
        printf ("Fail to open the image file.\n");
    }
    
    // clean up
    free(osname);
    free(label);
    close(fd);
    return 0;
}
