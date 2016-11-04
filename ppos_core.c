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
void tratador (int signum){
    currentTask->t_process++;
    tempo++;
    if(currentTask != &dispatcher){
        if(currentTask->quantum != 0){
            currentTask->quantum--;
	    }else{
            task_yield();
	    }
    }
}

void ppos_init(){
    /* desativa o buffer da saida padrao (stdout), usado pela função printf */
    setvbuf (stdout, 0, _IONBF, 0) ;
    tempo = 0;
    // inicialização da main
    taskMain.prev = NULL;
    taskMain.next = NULL;
    taskMain.tid = 0;
    taskMain.t_exec = systime();
    taskMain.activ = 0;
    taskMain.quantum = 20;
    init_process = 0;
    end_process = init_process;
    taskId = 0;
    currentTask = &taskMain;

    // cria o dispatcher
    task_create(&dispatcher,(void*) dispatcher_body, NULL);

    // registra a para o sinal de timer SIGALRM
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
    task_yield();
}

// Cria uma nova tarefa. Retorna um ID> 0 ou erro.
int task_create(task_t *task,			    // descritor da nova tarefa
		        void (*start_func)(void *), // funcao corpo da tarefa
		        void *arg){			        // argumentos para a tarefa
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
    if(task != &dispatcher){
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
    task_t *aux;
    #ifdef DEBUG
	    printf("task_exit: exit da tarefa %d\n", currentTask->tid);
    #endif
    currentTask->status = 0;
    currentTask->t_exec = systime() - currentTask->t_exec;
    printf("Task %d exit: execution time %d ms, processor time %d ms, %d activations\n",
	    currentTask->tid, currentTask->t_exec, currentTask->t_process, currentTask->activ);
    while ( (currentTask->dependQueue) != NULL ){
        aux = currentTask->dependQueue->next;
        aux->exitCode = exitCode;
        queue_append((queue_t **)&readyQueue, queue_remove((queue_t **) &currentTask->dependQueue, (queue_t *) aux));
    }
    if(currentTask == &dispatcher){
	    task_switch(&taskMain);
    }else
	    task_switch(&dispatcher);
}

// alterna a execução para a tarefa indicada
int task_switch(task_t *task){
	end_process = systime();
	currentTask->t_process += end_process - init_process;
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

void wakeupQueue (){
    if (sleepQueue != NULL){
        task_t *aux, *next_a;
        aux = sleepQueue;
        do{
	        if((aux->wakeup) <= systime()){
                next_a = aux->next;
                aux->status = 1;
                queue_append((queue_t **)&readyQueue, queue_remove((queue_t **) &sleepQueue, (queue_t *) aux));
                aux = next_a;
            }
            else
                aux = aux->next;
        }while((aux!=sleepQueue) && (sleepQueue != NULL));
    }
}

// dispatcher é uma tarefa
void dispatcher_body (){     
    #ifdef DEBUG
        printf("dispatcher_body: mudou para o dispatcher");
    #endif
    task_t *next;
    while (( ((queue_t *) readyQueue) != NULL ) || ((queue_t *) sleepQueue) != NULL ){
        if((queue_t *) readyQueue != NULL){
	        next = scheduler() ;  // scheduler é uma função
	        if (next){
	            queue_remove((queue_t **) &readyQueue, (queue_t *) next);
	            init_process = systime();
	            task_switch (next) ; // transfere controle para a tarefa "next"
	            if (next->status == 0)
		            free(next->taskContext.uc_stack.ss_sp);
	        }
        }
        if((queue_t *) sleepQueue != NULL)
            wakeupQueue(); 
    }
    task_exit(0) ; // encerra a tarefa dispatcher
}

// switch para o dispatcher
void task_yield(){
    if(currentTask != &dispatcher)
	    queue_append((queue_t **)&readyQueue, (queue_t *)currentTask);
    #ifdef DEBUG
	    printf("task_yield: a tarefa %d pediu yield", currentTask->tid);
    #endif
    task_switch(&dispatcher);
}

// scheduler
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

// seta a prioridade
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

// reseta a prioridade
int task_getprio (task_t *task){
    int retorno;
    if (task != NULL)
	    retorno = task->estatico;
    else
	    retorno = currentTask->estatico;
    return(retorno);
}

// coloca a tarefa como dependente
int task_join (task_t *task){
    if((task != NULL) && (task->status != 0)){
	    currentTask->status = 2;    //0 - finalizada 1-pronta 2 - suspensa
        queue_append((queue_t **)&task->dependQueue, (queue_t *)currentTask);
        task_switch(&dispatcher);
        return(currentTask->exitCode);
    }
    return (-1);
}

void task_sleep (int t){
	currentTask->status = 2;    //0 - finalizada 1-pronta 2 - suspensa
    currentTask->wakeup = systime() + t*1000;
    queue_append((queue_t **)&sleepQueue, (queue_t *)currentTask);
    task_switch(&dispatcher);
}

int sem_create (semaphore_t *s, int value){
    s->semaphoreQueue = NULL;
    s->semaphoreValue = value;
    if(s->semaphoreValue == value)
        return 0;
    return -1;
}

int sem_down (semaphore_t *s){
	if(s == NULL)
        return(-1);
    s->semaphoreValue--;
    if(s->semaphoreValue < 0){
        currentTask->status = 2;    //0 - finalizada 1-pronta 2 - suspensa
        queue_append((queue_t **)&s->semaphoreQueue, (queue_t *)currentTask);
        task_switch(&dispatcher);
    }
    return(0);
}

int sem_up (semaphore_t *s){
	if(s == NULL)
        return(-1);
    s->semaphoreValue++;
    if(s->semaphoreQueue != NULL){
        task_t *aux = s->semaphoreQueue;
	    queue_append((queue_t **)&readyQueue, queue_remove((queue_t **) &s->semaphoreQueue, (queue_t *) aux));
    }
    return(0);

}

int sem_destroy (semaphore_t *s){
    task_t *aux;
    while ( (s->semaphoreQueue) != NULL ){
        aux = s->semaphoreQueue->next;
	    queue_append((queue_t **)&readyQueue, queue_remove((queue_t **) &s->semaphoreQueue, (queue_t *) aux));
    }
    if (s->semaphoreQueue == NULL)
        return 0;
    return -1;
}
