// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.1 -- Julho de 2016
//
// Estruturas de dados internas do sistema operacional

#ifndef __PPOS_DATA__
#define __PPOS_DATA__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ucontext.h>
#include <signal.h>
#include <sys/time.h>

#define STACKSIZE 32768	    /* tamanho de pilha das threads */

// Estrutura que define uma tarefa
typedef struct task_t
{
  // preencher quando for necessário
    struct task_t *prev, *next;
    int tid;
    ucontext_t taskContext;
    int estatico;
    int dinamico;
    int status;
    int quantum;
    int	t_exec;
    int t_process;
    int activ;
    int exitCode;
    int wakeup;
    struct task_t *dependQueue;
} task_t ;

// estrutura que define um semáforo
typedef struct
{
    task_t *semaphoreQueue;
    int semaphoreValue;
} semaphore_t ;

// estrutura que define um mutex
typedef struct
{
  // preencher quando for necessário
} mutex_t ;

// estrutura que define uma barreira
typedef struct
{
  // preencher quando for necessário
} barrier_t ;

// estrutura que define uma fila de mensagens
typedef struct
{
    void *msgQueue;
    int max_msgs, msg_size, valid;
    int first, last, msg_cont;
    semaphore_t s_buffer, s_item, s_vcny;
} mqueue_t ;

task_t taskMain, dispatcher;
task_t *currentTask, *readyQueue, *sleepQueue;
int taskId;
int tempo, init_process, end_process;

#endif
