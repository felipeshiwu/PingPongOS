#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "ppos.h"

int item;
int buffer[5];
task_t p1, p2, p3, c1, c2;
semaphore_t s_buffer, s_item, s_vaga;
int last = -1, first = 0;

void produtor (void * arg) {
    while (1) {
        task_sleep (1);
        item = random()%100;

        sem_down (&s_vaga);
        sem_down (&s_buffer);

        last = (last+1) % 5;
        buffer[last] = item;
        printf ("%s produziu %d\n", (char *) arg, item);

        sem_up (&s_buffer);
        sem_up (&s_item);
    }
    task_exit (0) ;
}

void consumidor (void * arg) {
    while (1) {
        sem_down (&s_item);
        sem_down (&s_buffer);

        item = buffer[first];
        first = (first+1) % 5;
        printf ("%s consumiu %d\n", (char *) arg, item);

        sem_up (&s_buffer);
        sem_up (&s_vaga);
        task_sleep (1);
    }
    task_exit (0) ;
}

int main (int argc, char *argv[])
{
    srand(time(NULL));
    printf ("Main INICIO\n");

    ppos_init () ;

    sem_create (&s_buffer, 1) ;
    sem_create (&s_item, 0) ;
    sem_create (&s_vaga, 5) ;

    task_create (&p1, produtor, "p1") ;
    task_create (&p2, produtor, "p2") ;
    task_create (&p3, produtor, "p3") ;
    task_create (&c1, consumidor, "                      c1") ;
    task_create (&c2, consumidor, "                      c2") ;

    sem_destroy (&s_buffer);
    sem_destroy (&s_item);
    sem_destroy (&s_vaga);

    task_yield () ;

    printf ("Main FIM\n");
    task_exit (0) ;
    exit (0) ;
}
