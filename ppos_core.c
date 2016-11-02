#include "ppos.h"
#include "ppos_data.h"
#include "queue.h"


void dispatcher_body ();
task_t *scheduler();

// estrutura que define um tratador de sinal (deve ser global ou static)
struct sigaction action ;

// estrutura de inicialização to timer
struct itimerval timer;

unsigned int systime(){
    return(tempo);
}

// tratador do sinal
void tratador (int signum)
{
    if(currentTask != &dispatcher){
        if(currentTask->quantum != 0){
            currentTask->quantum--;
	}else{
	    end_process = systime();
	    currentTask->t_process += end_process - init_process;
            task_yield();
	}
    }
    tempo++;
}

void ppos_init(){
    /* desativa o buffer da saida padrao (stdout), usado pela função printf */
    setvbuf (stdout, 0, _IONBF, 0) ;
    tempo = 0;
    taskMain.prev = NULL;
    taskMain.next = NULL;
    taskMain.tid = 0;
    taskMain.t_exec = systime();
    taskMain.activ = 0;
    init_process = 0;
    end_process = init_process;
    taskId = 0;
    currentTask = &taskMain;
    task_create(&dispatcher,(void*) dispatcher_body, NULL);

    // registra a a��o para o sinal de timer SIGALRM
    action.sa_handler = tratador ;
    sigemptyset (&action.sa_mask) ;
    action.sa_flags = 0 ;
    if (sigaction (SIGALRM, &action, 0) < 0){
	perror ("Erro em sigaction: ") ;
	exit (1) ;
    }

    // ajusta valores do temporizador
    timer.it_value.tv_usec = 1000 ;      // primeiro disparo, em micro-segundos
    timer.it_value.tv_sec  = 0 ;      // primeiro disparo, em segundos
    timer.it_interval.tv_usec = 1000 ;   // disparos subsequentes, em micro-segundos
    timer.it_interval.tv_sec  = 0 ;   // disparos subsequentes, em segundos

    // arma o temporizador ITIMER_REAL (vide man setitimer)
    if (setitimer (ITIMER_REAL, &timer, 0) < 0){
	perror ("Erro em setitimer: ") ;
	exit (1) ;
    }

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
	task->t_exec = systime();
	task->activ = 1;
	if(task != &dispatcher){
	    task->estatico = 0;
	    task->dinamico = task->estatico;
	    task->quantum = 20;
	    taskMain.activ = 0;
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
    currentTask->status = 0;
    currentTask->t_exec = systime() - currentTask->t_exec;
    printf("Task %d exit: execution time %d ms, processor time %d ms, %d activations\n",
	    currentTask->tid, currentTask->t_exec, currentTask->t_process, currentTask->activ);
    if(currentTask == &dispatcher){
	task_switch(&taskMain);
    }else
	task_switch(&dispatcher);
    }

// alterna a execução para a tarefa indicada
int task_switch(task_t *task){
    currentTask->activ++;
    if (task != NULL){
        task_t *aux;
	aux = currentTask;
	currentTask = task;
	currentTask->quantum = 20;
	swapcontext (&aux->taskContext, &task->taskContext);
	init_process = systime();
	#ifdef DEBUG
	    printf("task_switch: mudou a tarefa %d para a tarefa %d\n", aux->tid, task->tid);
	#endif
	return(0);
    }
    perror ("Erro na troca da pilha: ");
    return(-1);
}

void dispatcher_body (){ // dispatcher é uma tarefa{
    #ifdef DEBUG
        printf("dispatcher_body: mudou para o dispatcher");
    #endif
    task_t *next;
    while ( ((queue_t *) readyQueue) > 0 ){
	next = scheduler() ;  // scheduler é uma função
	if (next){
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
	currentTask->estatico = prio;
	currentTask->dinamico = prio;
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