// Nikita Malhotra
// CSC 360: Operating Systems
// Assignment #1: To implement a realistic shell interpreter (RSI), interacting
//                with the operating system using system calls

#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>


typedef struct Node
{
    struct Node* next;
    struct Node* previous;
    
    pid_t pid;
    char* process_name;
    char* status;
    
} Node;

typedef struct list
{
    Node* head;
    Node* tail;
    int size;
    
} list;


// Global Variables
int counter = 0; //keep track of the number of tokens for background processes
list* linked_list;

// Name: char* getPrompt()
// Description: Retrieve the current working directory (See getcwd())
//              If successful, return the complete prompt with absolute
//              path = "RSI: [cwd] >"
char* getPrompt()
{
    char* cwd = malloc(sizeof(char) * 128);
    char* before = malloc(sizeof(char) * 256);
    char* after = malloc(sizeof(char) * 64);
    
    strcpy(before, "RSI: ");
    strcpy(after, "> ");

    getcwd(cwd, 128); //get the current working directory
    
    if (cwd != NULL)
    {
        strcat(before,cwd);
        strcat(before,after);
        
        free(cwd);
        free(after);

        return before;
        
    } else //check for an exception
    {
        perror ("getPrompt() error");
        return NULL;
    }
}


// Name: char** tokensize (char* reply)
// Description: Tokenize the input given by the user
char** tokenize (char* reply)
{
    counter = 0;
    char* token;
    char** string_return = calloc(counter + 1, sizeof(char*));
    
    //parsing the user input in reply using strtok and space as the delimiter
    token = strtok(reply, " ");
    while (token != NULL)
    {
        counter = counter + 1; //increase the counter
        //allocate memory the size of the char
        string_return = realloc(string_return, sizeof(char*) * (counter + 1));
        //allocate memeory for each individual token
        string_return[counter - 1] = calloc(64,sizeof(char));
        
        strcpy(string_return[counter - 1], token);
        token = strtok(NULL, " "); //get the next token
    }
    
    string_return[counter] = NULL; //null terminate the array of strings
    return string_return;
}


int main(int argc, char* argv[])
{
    int status, bailout = 1;
    char* prompt;
    char* reply;
    char* command_line = malloc(sizeof(char) * 10);
    char** tok;
    pid_t childpid,childpid2;
    int return_val,backvalue,forevalue;
    linked_list = calloc(1, sizeof(list)); //allocating memory for the linked_list
    
    while (bailout)
    {
        prompt = getPrompt();
        reply = readline(prompt); //get the user input
        
        //If the user quits, exit the loop
        if (!strcmp(reply, "quit"))
        {
            bailout = 0;
        } else
        {
            //If "cd", then change the directory using chdir()
            strncpy(command_line,reply,2);
            command_line[2] = '\0';
            
            if (!strcmp(command_line, "cd"))
            {
                reply = reply + 3;
                chdir (reply);
            } else
            {
                //If not "cd", then use fork() and execvp()
                tok = tokenize(reply);
                
                if (counter > 0) //the tok string is not empty
                {
                    if (!strcmp(tok[0], "kill")) //if the string contains kill remove from the list
                    {
                        Node* victim = linked_list -> head; //pointer to go through the list
                        int list_counter = 0;
                        while (list_counter < linked_list -> size)
                        {
                            if (victim -> pid == atoi (tok[1]))
                            {
                                //remove for the linked_list
                                printf("%d Terminated %s\n",victim -> pid, victim -> process_name);
                                
                                //case1: only one item in the list
                                if (linked_list -> size == 1)
                                {
                                    linked_list -> head = NULL;
                                    linked_list -> tail = NULL;
                                }
                                //case2: victim points to the head
                                else if (victim == linked_list -> head)
                                {
                                    linked_list -> head = victim -> next;
                                    linked_list -> head -> previous = NULL;
                                }
                                //case3: victim points to the tail
                                else if (victim == linked_list -> tail)
                                {
                                    linked_list -> tail = victim -> previous;
                                    linked_list -> tail -> next = NULL;
                                }
                                //case4: victim is somewhere in the list
                                else
                                {
                                    victim -> next -> previous = victim -> previous;
                                    victim -> previous -> next = victim -> next;
                                }
                                
                                linked_list -> size = linked_list -> size - 1;
                                free(victim -> status);
                                free(victim -> process_name);
                                break;
                            }
                            
                            victim = victim -> next;
                            list_counter = list_counter + 1;
                        } //while loop
                    }

                    if (!strcmp(tok[counter - 1], "&")) //if the string contains &
                    {
                        //do the background proccesses
                        childpid2 = fork();
                        tok[counter - 1] = NULL;
                        
                        if (childpid2 >= 0) //success
                        {
                            if (childpid2 == 0) //child process
                            {
                                return_val = execvp(tok[0], tok);
                                
                                if (return_val < 0) //check for an exception
                                {
                                    printf ("RSI: %s command not found \n", tok[0]);
                                    exit(0); //exit the child and go back to the parent
                                }
                            } else
                            {
                                //adding the proccesses to the linked_list
                                Node* node = calloc (1, sizeof(Node)); // allocating memory for the node
                                node -> process_name = calloc (128, sizeof(char));
                                node -> status = calloc (128, sizeof(char));
                                node -> pid = childpid2;
                                strcpy (node -> status, "Running");
                                strcpy (node -> process_name, tok[0]);
                                
                                //case1: if the list is empty
                                if (linked_list -> head == NULL)
                                {
                                    linked_list -> head = node;
                                    linked_list -> tail = node;
                                    linked_list -> size = 1;
                                } else
                                { //case2: list is the not empty
                                    node -> previous = linked_list -> tail;
                                    linked_list -> tail -> next = node;
                                    linked_list -> tail = node;
                                    linked_list -> size = linked_list -> size + 1;
                                }
                                
                                usleep(20000);
                                backvalue = waitpid(-1, &status, WNOHANG);
                                
                                if (backvalue == -1) //check for an exception
                                {
                                    perror ("waitpid() error");
                                    exit(0);
                                }
                            }
                        } else
                        {
                            //check for an exception
                            perror ("fork() error");
                            exit(0);
                        }
                    } else
                    {
                        //do the foreground proccesses
                        childpid = fork();
                        if (childpid >= 0) //success
                        {
                            if (childpid == 0) //child process
                            {
                                return_val = execvp(tok[0], tok);
                                
                                if (return_val < 0) //check for an exception
                                {
                                    printf ("RSI: %s command not found \n", tok[0]);
                                    exit(0); //exit the child and go back to the parent
                                }
                            } else
                            {
                                //wait for the child process to end
                                waitpid(-1, &status, 0);
                            }
                            usleep(20000); //this is to wait for the previous proccess to end
                        } else
                        {
                            //check for an exception
                            perror ("fork() error");
                            exit(0);
                        }
                    }
                }
                free(tok);
            }
        }
        free(prompt);
    } //while loop

    printf("RSI:  Exiting normally. \n");
    return(0);
}















