// Nikita Malhotra
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
int getSector (char* mmap, int currSector)
{
    int n = currSector; // logical number of the first sector in the Data Area
    int base = 512; // the first bye of the FAT table
    int* temp1 = malloc(sizeof(int));
    int* temp2 = malloc(sizeof(int));
    int result = 0;
    
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
void FileData (char* mmap, char* filename)
{
    int base = 9728;  // The first byte of the root directory
    int current = base;   // Point to the first byte of the current entry
    int offset = 32;  // Each entry has 32 bytes in root directory
    int* temp1 = malloc(sizeof(int));
    int* temp2 = malloc(sizeof(int));
    char* temp3 = malloc(sizeof(char)*2);
    char* check_name = malloc(sizeof(char)*14);
    DiskFile* file = calloc(1, sizeof(DiskFile));
    int counter = 0;
    
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
                    // for testing
                    // printf ("%s\n",whole_file_name);

                    if(!strcmp(whole_file_name, filename))
                    {
                        // Find the logical sector
                        file -> logical_sector = (mmap[current + 26] & 0x00FF) + ((mmap[current + 27] & 0x00FF) << 8);
                        file -> filesize = (mmap[current + 28] & 0x00FF) + ((mmap[current + 29] & 0x00FF) << 8) + ((mmap[current + 30] & 0x00FF) << 16) + ((mmap[current + 31] & 0x00FF) << 24);
                        strcpy(check_name, whole_file_name);
                        break; // Found the file, break out from the while loop
                    }
                }
            }
        }
        
        // Go to next entry in Root Directory
        current = current + offset;
        *temp1 = mmap[current];
        free(whole_file_name);
    }
    
    if(strcmp(check_name, filename))
    {
        printf("File not found\n");
        exit(0);
    }
    
    // trying to open a file that the program has found
    FILE* fp = fopen(filename, "w");
    int next = file -> logical_sector; // next logical sector
    int bytes_left = file -> filesize; // the remaining bytes left to read
    int bytes_to_read;
    
    for (; next < 0xFF8; next = (getSector(mmap,next) & 0xFFF), bytes_left -= 512)
    {
        int start_byte = 512 * (next + 31);
        if (bytes_left >= 512)
        {
            bytes_to_read = 512;
        } else
        {
            bytes_to_read = bytes_left;
        }
        fwrite (mmap + start_byte, sizeof(char), bytes_to_read, fp);
    }
    
    // Clean up
    free(file);
    free(temp1);
    free(temp2);
    free(temp3);
    free(check_name);
    fclose(fp);
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
        
        // Get the data of the file
        FileData(p, argv[2]);
    } else
    {
        printf ("Fail to open the image file.\n");
    }
    
    // Clean up
    close(fd);
    return 0;
}
