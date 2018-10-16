/* Tests cetegorical mutual exclusion with different numbers of threads.
 * Automatic checks only catch severe problems like crashes.
 */
#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "lib/random.h" //generate random numbers

#define BUS_CAPACITY 3
#define SENDER 0
#define RECEIVER 1
#define NORMAL 0
#define HIGH 1

//Added


int busDirection, threadsOnBus;
struct lock lock;

struct condition prioSender, sender, prioReceiver, receiver;

//struct semaphore threadsOnBus;

//queueSenders
//queueReceivers

/*
 *	initialize task with direction and priority
 *	call o
 * */
typedef struct {
	int direction;
	int priority;
} task_t;

void batchScheduler(unsigned int num_tasks_send, unsigned int num_task_receive,
        unsigned int num_priority_send, unsigned int num_priority_receive);

void senderTask(void *);
void receiverTask(void *);
void senderPriorityTask(void *);
void receiverPriorityTask(void *);


void oneTask(task_t task);/*Task requires to use the bus and executes methods below*/
void getSlot(task_t task); /* task tries to use slot on the bus */
void transferData(task_t task); /* task processes data on the bus either sending or receiving based on the direction*/
void leaveSlot(task_t task); /* task release the slot */



/* initializes semaphores */ 
void init_bus(void){ 

    
    random_init((unsigned int)123456789); 

    //sema_init(&threadsOnBus, 0);
    threadsOnBus = 0;


    lock_init(&lock);

    cond_init(&prioSender);
    cond_init(&sender);
    cond_init(&prioReceiver);
    cond_init(&receiver);
   // sema_init(busCapacitySemaphore, BUS_CAPACITY);

}

/*
 *  Creates a memory bus sub-system  with num_tasks_send + num_priority_send
 *  sending data to the accelerator and num_task_receive + num_priority_receive tasks
 *  reading data/results from the accelerator.
 *
 *  Every task is represented by its own thread. 
 *  Task requires and gets slot on bus system (1)
 *  process data and the bus (2)
 *  Leave the bus (3).
 */

void batchScheduler(unsigned int num_tasks_send, unsigned int num_task_receive,
        unsigned int num_priority_send, unsigned int num_priority_receive)
{
    //int priority = 0; //NORMAL

    int i;
    for(i = 0; i < num_tasks_send; i++) {
            thread_create("Sender", 0, senderTask, 0);
    }

    for(i = 0; i < num_priority_send; i++) {
        thread_create("PrioSender", 1, senderPriorityTask, 0);
    }

    for(i = 0; i <num_task_receive; i++) {
        thread_create("Receiver", 1, receiverTask, 0);
    }

    for(i = 0; i < num_priority_receive; i++) {
        thread_create("PrioReceiver", 1, receiverPriorityTask, 0);
    }

}
   

/* Normal task,  sending data to the accelerator */
void senderTask(void *aux UNUSED){
        task_t task = {SENDER, NORMAL};
        oneTask(task);
}

/* High priority task, sending data to the accelerator */
void senderPriorityTask(void *aux UNUSED){
        task_t task = {SENDER, HIGH};
        oneTask(task);
}

/* Normal task, reading data from the accelerator */
void receiverTask(void *aux UNUSED){
        task_t task = {RECEIVER, NORMAL};
        oneTask(task);
}

/* High priority task, reading data from the accelerator */
void receiverPriorityTask(void *aux UNUSED){
        task_t task = {RECEIVER, HIGH};
        oneTask(task);
}

/* abstract task execution*/
void oneTask(task_t task) {
  getSlot(task);
  transferData(task);
  leaveSlot(task);
}


/* task tries to get slot on the bus subsystem */
void getSlot(task_t task) 
{
    while(1) {
        if(threadsOnBus < 3 && busDirection == task.direction) {
            threadsOnBus++;
            return;
        } else { 
            if(task.direction == SENDER) {
                if(task.priority == HIGH) {
                    cond_wait(&prioSender, &lock);
                } else {
                    cond_wait(&sender, &lock);
                }
            } else {
                //Receiver
                if(task.priority == HIGH) {
                    cond_wait(&prioReceiver, &lock);
                } else {
                    cond_wait(&receiver, &lock);
                }
            }
        }

    }
         

}

/* task processes data on the bus send/receive */
void transferData(task_t task) 
{
    /*
        sleep(); //wait
    */
}

/* task releases the slot */
void leaveSlot(task_t task) 
{   



    /*
        threadsOnBus--

        if(this.Thread == SENDER) {
            is(prioSenderWaiting.empty()) {
                if(normalSenderWaiting.empty()) {
                    //No more senders
                    switchBusDirection();
                } else {
                    cond_signal(senders, NULL);;                    
                }

            } else {
                 cond_signal(prioSenders, NULL);;  
            }

        } else { //Receiver
            if(prioReceiverWaiting.empty() ) {
                if(normalSenderWaiting.empty()) {
                    //No more receivers
                    switchBusDirection();
                } else {
                    cond_signal(receivers, NULL);; 
                }
            } else {
                cond_signal(prioReceivers, NULL);; 
            }
        }

    */
}
