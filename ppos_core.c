#include "ppos.h"
#include "ppos_data.h"

void ppos_init(){
    /* desativa o buffer da saida padrao (stdout), usado pela função printf */
    setvbuf (stdout, 0, _IONBF, 0) ;
    taskMain.prev = NULL;
    taskMain.next = NULL;
    taskMain.tid = 0;
    taskId = 0;
    currentTask = &taskMain;
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
	}else{
	    perror ("Erro na criação da pilha: ");
	    return(-1);
	}
    makecontext (&(task->taskContext), (void*)(*start_func), 1, arg);
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
    task_switch(&taskMain);
    currentTask = &taskMain;
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
