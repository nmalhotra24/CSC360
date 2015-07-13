// Nikita Malhotra
// CSC 360: Operating Systems
// Assingment #2: To stimulate a priority queueing system (PQS).

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

typedef struct {
    int customer_id;
    float arrival_time;
    float service_time;
    int priority_number;
    pthread_mutex_t mutex;
    pthread_cond_t convar;
} customer_info;

pthread_mutex_t clerk_mutex;
pthread_cond_t clerk_convar;

// Count for the number of customers waiting to be served
int customers_waiting = 0;

// Id for the current customer being servved by the clerk
int customer_being_served = 0;

typedef struct node {
    customer_info* customer;
    struct node* next;
} list;

list* head = NULL;


void print_list ()
{
    list* cur = head;
    while (cur != NULL)
    {
        printf ("%2d\t", cur -> customer -> customer_id);
        cur = cur -> next;
    }
    printf ("\n");
}


void enqueue_customer (customer_info* customer)
{
    list* prev = NULL;
    list* current = head;
    
    // allocate memory for node
    list* node = (list*) malloc(sizeof(list));
    if (node == NULL) // check for exception
    {
        perror ("Unable to allocate memory for node. \n");
        exit(0);
    }
    node -> customer = customer;
    node -> next = NULL;
    
    while (current != NULL)
    {
        if (current -> customer -> priority_number == customer -> priority_number &&
            current -> customer -> arrival_time ==  customer -> arrival_time &&
            current -> customer -> service_time == customer -> service_time)
        {
            // case1: the current customers id is less than the arriving customers id
            if (current -> customer -> customer_id < customer -> customer_id)
            {
                prev = current;
                current = current -> next;
            } else
            {
                break;
            }
        } else if (current -> customer -> priority_number == customer -> priority_number &&
                   current -> customer -> arrival_time == customer -> arrival_time)
        {
            // case2: the current customers service time is less than the arriving customers
            // service time
            if (current -> customer -> service_time < customer -> service_time)
            {
                prev = current;
                current = current -> next;
            } else
            {
                break;
            }
        } else if (current -> customer -> priority_number < customer -> priority_number)
        {
            // case3: the current customer priority number is less than the arriving
            // customers priority number
            break;
        } else
        {
            prev = current;
            current = current -> next;
        }
    }
    
    if (prev)
    {
        node -> next = prev -> next;
        prev -> next = node;
    } else
    {
        node -> next = head;
        head = node;
    }
}


customer_info* dequeue_customer(void)
{
    list* current = head;
    customer_info* customer;
    
    if (head == NULL)
    {
        return NULL;
    }
    head = head -> next;
    customer = current -> customer;
    free(current);
    return customer;
}


customer_info* clerk_readyto_serve(void)
{
    customer_info* customer;
    pthread_mutex_lock(&clerk_mutex);
    
    // wait for the customer to arrive
    while (customers_waiting == 0)
    {
        customer_being_served = 0;
        pthread_cond_wait(&clerk_convar, &clerk_mutex);
    }
    
    // remove the customer from the queue
    customer = dequeue_customer();
    --customers_waiting; // decrease the number of customers waiting
    customer_being_served = customer -> customer_id;
    pthread_mutex_unlock(&clerk_mutex);
    
    return customer;
}


long long current_timestamp()
{
    struct timeval te;
    gettimeofday(&te, NULL); // get the current time
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // calculate the milliseconds
    
    return milliseconds;
}


void* clerk_thread (void* customer_count)
{
    int* count = (int*)customer_count;
    int customer_done = 0;
    customer_info* customer;
    long long now;
    long long start = current_timestamp();
    
    while (customer_done != *count)
    {
        customer = clerk_readyto_serve();
        if (customer == NULL)
        {
            return 0;
        }
        
        now = current_timestamp();
        printf ("The clerk starts serving customer %2d at time %.2f. \n",
                customer -> customer_id, ((float)(now - start))/1000);
        
        // serving the arriving customer
        usleep(customer -> service_time * 1000000);
        
        now = current_timestamp();
        printf("The clerk finishes the service to customer %2d at time %.2f. \n",
               customer -> customer_id, ((float)(now - start))/1000);
        
        ++customer_done;
        // send a signal to the customer that the clerk is done
        pthread_mutex_lock(&customer -> mutex);
        pthread_cond_signal(&customer -> convar);
        pthread_mutex_unlock(&customer -> mutex);
    }
    pthread_exit(NULL);
}


void* customer_thread (void* info)
{
    customer_info* customer = (customer_info*)info;
    
    // initialize the customer mutex and the conditional variables
    pthread_mutex_init(&customer -> mutex, NULL);
    pthread_cond_init(&customer -> convar, NULL);
    
    usleep(customer -> arrival_time * 1000000);
    
    // arrival of the customer
    printf ("customer %2d arrives: arrival time (%.2f), service time (%.1f), priority (%2d). \n",
            customer -> customer_id, customer -> arrival_time, customer -> service_time, customer -> priority_number);
    
    pthread_mutex_lock(&clerk_mutex);
    
    if (customer_being_served != 0)
    {
        printf ("customer %2d waits for the finish of customer %2d. \n",
                customer -> customer_id, customer_being_served);
    }
    
    // add customer to the queue and notify the clerk
    enqueue_customer(customer);
    ++customers_waiting; // increase the number of customers waiting
    pthread_cond_signal(&clerk_convar);
    
    pthread_mutex_unlock(&clerk_mutex);
    
    // the customer waiting will be served
    pthread_mutex_lock(&customer -> mutex);
    pthread_cond_wait(&customer -> convar, &customer -> mutex);
    pthread_mutex_unlock(&customer -> mutex);
    
    pthread_mutex_destroy(&customer -> mutex);
    pthread_cond_destroy(&customer -> convar);
    pthread_exit(NULL);
}


int main (int argc, char* argv[])
{
    int i, no_of_customers = 0;
    char line[256];
    FILE *fp;
    customer_info* customer;
    
    if (argc != 2)
    {
        printf ("usage: %s filename", argv[0]);
        exit(0);
    } else
    {
        // open the file with "read" mode
        fp = fopen (argv[1], "r");
        if (fp == NULL) //check for exception
        {
            perror ("Error while opening the file. \n");
            exit(0);
        }
    }
    // read one line at a time from the file, stopping at EOF
    fgets(line,sizeof(line),fp);
    no_of_customers = atoi(line); // converting the string to integer
    if (no_of_customers == 0) // check for exception
    {
        perror ("Invalid entry. \n");
        exit(0);
    }
    
    // allocating memory for customer
    customer = (customer_info *) malloc(sizeof(customer_info) * (no_of_customers));
    if (customer == NULL) // check for memory allocation
    {
        perror ("Failed to allocate memory for customer. \n");
        exit(0);
    }
    
    // parsing the input file using fscanf into the structure
    for (i = 0; i < no_of_customers; i++)
    {
        fscanf(fp,"%d:%f,%f,%d",&(customer[i].customer_id),&(customer[i].arrival_time),&(customer[i].service_time),&(customer[i].priority_number));
        customer[i].arrival_time = customer[i].arrival_time/10;
        customer[i].service_time = customer[i].service_time/10;
    }
    
    // allocating memory for pthreads -> (number of customers + 1)
    pthread_t* threads = (pthread_t *) malloc(sizeof(pthread_t) * (no_of_customers+1));
    if (threads == NULL) // check for exception
    {
        perror ("Failed to allocate memory for pthreads. \n");
        exit(0);
    }
    
    // initialize the clerk mutex and conditional variables
    pthread_mutex_init(&clerk_mutex, NULL);
    pthread_cond_init(&clerk_convar, NULL);
    
    // creating the threads in a joinable state
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    
    // create the threads => (number of customers + 1)
    for (i = 0; i <= no_of_customers; i++)
    {
        if (i == 0)
        {
            // create the clerk thread
            pthread_create(&threads[i], &attr, clerk_thread, (void *)&no_of_customers);
        } else
        {
            // create the customer threads
            pthread_create(&threads[i], &attr, customer_thread, (void *)&customer[i-1]);
        }
    }
    
    // wait for all the threads to complete
    for (i = 0; i <= no_of_customers; i++)
    {
        pthread_join(threads[i], NULL);
    }
    
    free(threads);
    free(customer);
    
    pthread_attr_destroy(&attr);
    pthread_mutex_destroy(&clerk_mutex);
    pthread_cond_destroy(&clerk_convar);
    pthread_exit(NULL);
    fclose(fp);
    return 0;
}

