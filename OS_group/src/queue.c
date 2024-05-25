#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int empty(struct queue_t *q)
{
        if (q == NULL)
                return 1;
        return (q->size == 0);
}

void enqueue(struct queue_t *q, struct pcb_t *proc)
{
        /* TODO: put a new process to queue [q] */
        int q_size = q->size;
        if (q_size < MAX_QUEUE_SIZE && q_size >= 0)
        {
                q->proc[q_size] = proc;
                q->size = q_size + 1;
        }
}

struct pcb_t *dequeue(struct queue_t *q)
{
        /* TODO: return a pcb whose prioprity is the highest
         * in the queue [q] and remember to remove it from q
         * */

        int q_size = q->size;
        if (q_size == 1)
        {
                struct pcb_t *return_proc = q->proc[0];
                q->proc[0] = NULL;
                q->size = 0;
                return return_proc;
        }
        else if (q_size > 1 && q_size <= MAX_QUEUE_SIZE)
        {
                struct pcb_t *return_proc = q->proc[0]; // process will be returned
                uint32_t highest_prior = q->proc[0]->priority;
                int position = 0;
                int i;
                for (i = 0; i < q_size; i++)
                {
                        if (highest_prior < q->proc[i]->priority)
                        {
                                highest_prior = q->proc[i]->priority;
                                position = i;
                        }
                }
                return_proc = q->proc[position];
                if (position == (q_size - 1)) // highest at the end of queue
                {
                        q->proc[position] = NULL;
                        q->size = q_size - 1;
                        return return_proc;
                }
                else
                {
                        for (i = position + 1; i < q_size; i++)
                        {
                                q->proc[i - 1] = q->proc[i];
                        }
                        q->proc[q_size - 1] = NULL;
                        q->size = q_size - 1;
                        return return_proc;
                }
        }

        return NULL;
}
