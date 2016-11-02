#include "ppos.h"
#include "ppos_data.h"
#include "queue.h"


void dispatcher_body ();
task_t *scheduler();

void ppos_init(){
    /* desativa o buffer da saida padrao (stdout), usado pela função printf */
    setvbuf (stdout, 0, _IONBF, 0) ;
    taskMain.prev = NULL;
    taskMain.next = NULL;
    taskMain.tid = 0;
    taskId = 0;
    currentTask = &taskMain;
    task_create(&dispatcher,(void*) dispatcher_body, NULL);
    #ifdef DEBUG
    	printf("task_init: inicializado com sucesso, tarefa corrente eh a %d\n", currentTask->tid);
    #endif
}

// gerência de tarefas =========================================================
//
// Cria uma nova tarefa. Retorna um ID> 0 ou erro.
int task_create(task_t *task,			    // descritor da nova tarefa
		void (*start_func)(void *),	    // funcao corpo da tarefa
		void *arg){			    // argumentos para a tarefa
    getcontext(&(task->taskContext));
    char *stack;
    stack = malloc (STACKSIZE) ;
	if (stack){
	    task->taskContext.uc_stack.ss_sp = stack ;
	    task->taskContext.uc_stack.ss_size = STACKSIZE;
	    task->taskContext.uc_stack.ss_flags = 0;
	    task->taskContext.uc_link = 0;
	    taskId++;
	    task->tid = taskId;
	    task->status = 1;
	    if(task != &dispatcher){
		task->estatico = 0;
		task->dinamico = task->estatico;
	    }
	}else{
	    perror ("Erro na criação da pilha: ");
	    return(-1);
	}
    makecontext (&(task->taskContext), (void*)(*start_func), 1, arg);
    if((task != &dispatcher)&&(task != &taskMain)){
	queue_append((queue_t **) &readyQueue, (queue_t *) task);
    }
    #ifdef DEBUG
    	printf("task_create: criou tarefa %d\n", task->tid);
    #endif
    return(taskId);
}

// retorna o identificador da tarefa corrente (main eh 0)
int task_id(){
    #ifdef DEBUG
    	printf("task_id: tarefa corrente eh a %d\n", currentTask->tid);
    #endif
    return(currentTask->tid);
}

// Termina a tarefa corrente, indicando um valor de status encerramento
void task_exit(int exitCode){
    #ifdef DEBUG
	printf("task_exit: exit da tarefa %d\n", currentTask->tid);
    #endif
    currentTask ->status = 0;
    if(currentTask == &dispatcher){
	task_switch(&taskMain);
    }else
	task_switch(&dispatcher);
}
// alterna a execução para a tarefa indicada
int task_switch(task_t *task){
    if (task != NULL){
	task_t *aux;
	aux = currentTask;
	currentTask = task;
	swapcontext (&aux->taskContext, &task->taskContext);
	#ifdef DEBUG
	   printf("task_switch: mudou a tarefa %d para a tarefa %d\n", aux->tid, task->tid);
	#endif
	return(0);
    }
    perror ("Erro na troca da pilha: ");
    return(-1);
}

void dispatcher_body () // dispatcher é uma tarefa
{
    #ifdef DEBUG
	printf("dispatcher_body: mudou para o dispatcher");
    #endif
    task_t *next;
    while ( ((queue_t *) readyQueue) > 0 )
    {
        next = scheduler() ;  // scheduler é uma função
        if (next)
        {
	    queue_remove((queue_t **) &readyQueue, (queue_t *) next);
	    task_switch (next) ; // transfere controle para a tarefa "next"
	    if (next->status == 0)
		free(next->taskContext.uc_stack.ss_sp);
	}
    }
    task_exit(0) ; // encerra a tarefa dispatcher
}

void task_yield(){
    if((currentTask != &taskMain)&&(currentTask != &dispatcher))
	queue_append((queue_t **)&readyQueue, (queue_t *)currentTask);
    #ifdef DEBUG
	printf("task_yield: a tarefa %d pediu yield", currentTask->tid);
    #endif
    task_switch(&dispatcher);
}

task_t *scheduler(){
    task_t *aux;
    task_t *menorPrioridade;
    aux = readyQueue;
    menorPrioridade = aux;
    do{
	aux = aux->next;
	if((aux->dinamico) < (menorPrioridade->dinamico))
	    menorPrioridade = aux;
	else if (aux->dinamico == (menorPrioridade->dinamico))
		if (aux->estatico < menorPrioridade->estatico)
		    menorPrioridade = aux;
    }while(aux!=readyQueue);
    aux = readyQueue;
    do{
	if (aux->dinamico > -20)
	    aux->dinamico--;
	aux = aux->next;
    }while(aux!=readyQueue);


    menorPrioridade->dinamico = menorPrioridade->estatico;
    return(menorPrioridade);
}

void task_setprio (task_t *task, int prio){
    if (task != NULL){
	task->estatico = prio;
	task->dinamico = prio;
    }else{
	task->estatico = prio;
	task->dinamico = prio;
    }
    #ifdef DEBUG
	printf("task_setprio: as prios da tarefa %d foram setadas", task->tid);
    #endif
}

int task_getprio (task_t *task){
    int retorno;
    if (task != NULL)
	retorno = task->estatico;
    else
	retorno = currentTask->estatico;
    return(retorno);
}
