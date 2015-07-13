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

typedef struct DiskFile {
    char type;
    char* name;
    char* extension;
    int year, month, day, hours, minutes, filesize;
} DiskFile;

// ******************************************************************************
void PrintDiskList (DiskFile* file)
{
    printf ("%c ", file -> type);
    printf ("%10d ", file -> filesize);
    printf ("%16s.", file -> name);
    printf ("%s ", file -> extension);
    printf ("%d-%d-%d ", file -> year, file -> month, file -> day);
    printf ("%02d:%02d\n", file -> hours, file -> minutes);
}

// ******************************************************************************
void FileInfo (char* mmap)
{
    int base = 9728;  // the first byte of the root directory
    int current = base;   // point to the first byte of the current entry
    int offset = 32;  // Each entry has 32 bytes in root directory
    int* temp1 = malloc(sizeof(int));
    int* temp2 = malloc(sizeof(int));
    int counter = 0;
    int i;
    
    *temp1 = mmap[current];
    
    while(*temp1 != 0x00 && current < (33 * 512))
    {
        // Check if not empty --> 0xE5
        if (*temp1 != 0xE5)
        {
            *temp2 = mmap[current + 11];
            // CHeck if not a volume label --> 0x08
            if (*temp2 != 0x08)
            {
                // Check if not part of a long file name --> 0x0F
                if (*temp2 != 0x0F)
                {
                    DiskFile* file = calloc(1, sizeof(DiskFile));
                    file -> name = calloc(1, sizeof(char)*9); // Times 9 because 8 bytes + null
                    file -> extension = calloc(1, sizeof(char)*4); // Times 4 because 3 bytes + null
                    
                    if (*temp2 == 0x10) //0x10 --> subdirectory
                    {
                        file -> type = 'D';
                    } else
                    {
                        file -> type = 'F';
                    }
                    
                    // If the file name is a space then change to null termination character
                    for (i = 0; i < 8; i++)
                    {
                        file -> name[i] = mmap[current + i];
                        if (file -> name[i] == ' ')
                        {
                            file -> name[i] = '\0';
                            break;
                        }
                    }
                    
                    for (i = 8; i < 11; i++)
                    {
                        file -> extension[i - 8] = mmap[current + i];
                    }
                    
                    // Null termination
                    file -> name[8] = '\0';
                    file -> extension[3] = '\0';
                    // At this moment, we have found the name, type and extension of the file
                    // Get the filesize --> read the last four bytes
                    file -> filesize = (mmap[current + 28] & 0x00FF) + ((mmap[current + 29] & 0x00FF) << 8) + ((mmap[current + 30] & 0x00FF) << 16)
                                       +((mmap[current + 31] & 0x00FF) << 24);
                    
                    // Get the date --> read 2 bytes - 16,17
                    int date = (mmap[current + 16] & 0x00FF) + ((mmap[current + 17] & 0x00FF) << 8);
                    file -> year = ((date & 0xFE00) >> 9) + 1980;
                    file -> month = ((date & 0x01E0) >> 5);
                    file -> day = (date & 0x001F);
                    
                    // Get the time --> read 2 bytes - 14, 15
                    int time = (mmap[current + 14] & 0x00FF) + ((mmap[current + 15] & 0x00FF) << 8);
                    file -> hours =  ((time & 0xF800) >> 11);
                    file -> minutes = ((time & 0x07E0) >> 5);
                    
                    PrintDiskList(file);
                    free(file);
                }
            }
        }
        
        // Go to next entry in Root Directory
        current = current + offset;
        *temp1 = mmap[current];
    }
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
        
        FileInfo(p);
        
    } else
    {
        printf ("Fail to open the image file.\n");
    }
    
    // clean up
    close(fd);
    return 0;
}
