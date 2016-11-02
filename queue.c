#include "queue.h"
#include <stdio.h>

//-----------------------------------------------------------------------
// Insere um elemento no final da fila.
// Condicoes a verificar, gerando msgs de erro:
// - a fila deve existir
// - o elemento deve existir
// - o elemento nao deve estar em outra fila

void queue_append (queue_t **queue, queue_t *elem){
    if(queue == NULL)
        puts("Fila inexistente\n");
    else{									//fila existe
        if(elem != NULL){							//elemento existe		
     	    if((elem->prev == NULL) && (elem->next) == NULL){			//elemento não esta em outra fila
	        if(*queue == NULL){						//inserir elemento em fila vazia
	            elem->next = elem;
		    elem->prev = elem;
		    *queue = elem;
	    	}else{								//inserir elemento em fila não vazia
		    elem->prev = (*queue)->prev;
		    elem->next = *queue;
		    (*queue)->prev->next = elem;
		    (*queue)->prev = elem;
    		}
	    }else
		puts("Elemento pertence a outra fila");
	}else
            puts("Elemento inexistente\n");
    }
}

//-----------------------------------------------------------------------
// Remove o elemento indicado da fila, sem o destruir.
// Condicoes a verificar, gerando msgs de erro:
// - a fila deve existir
// - a fila nao deve estar vazia
// - o elemento deve existir
// - o elemento deve pertencer a fila indicada
// Retorno: apontador para o elemento removido, ou NULL se erro

queue_t *queue_remove (queue_t **queue, queue_t *elem){
    queue_t *aux;
    int naFila = 0;
    if(queue == NULL)
        puts("Fila inexistente\n");
    else{	                                                                //fila existe
        if(elem != NULL){							//elemento existe
	    if(*queue == NULL)
	        puts("Fila vazia\n");
	    else{								//fila não vazia
	        aux = *queue;
	        do{								//procura se o elemento está na fila
	            if(aux == elem){						//elemento pertence a fila indicada
	               	naFila = 1;
		        if(aux == aux->next){					//remove elemento único da fila
		    	    aux->next = NULL;
		            aux->prev = NULL;
		    	    *queue = NULL;	
		    	}else{							//remove elemento não único da fila
		    	    if(*queue == elem)					//verifica se o elemento a ser removido é o primeiro
		    	        *queue = (*queue)->next;
		    	    aux->prev->next = aux->next;
		    	    aux->next->prev = aux->prev;
		    	    aux->next = NULL;
		    	    aux->prev = NULL;
		    	}
		    	return(aux);
		    }
		    aux = aux->next;
		}while((aux != *queue) && (naFila == 0));
	        if(naFila == 0)
	            puts("Elemento nao pertence a fila indicada");
    	    }
	}else
	    puts("Elemento inexistente\n");
    }
    return(NULL);
}

//-----------------------------------------------------------------------
// Conta o numero de elementos na fila
// Retorno: numero de elementos na fila

int queue_size (queue_t *queue){
    int tam=0;
    queue_t *aux;
    if(queue != NULL){								//fila não vazia
        aux = queue;
	do{
	    tam++;
	    aux = aux->next;
	}while(aux != queue);
    }
    return(tam);
}

//-----------------------------------------------------------------------
// Percorre a fila e imprime na tela seu conteúdo. A impressão de cada
// elemento é feita por uma função externa, definida pelo programa que
// usa a biblioteca. Essa função deve ter o seguinte protótipo:
//
// void print_elem (void *ptr) ; // ptr aponta para o elemento a imprimir

void queue_print (char *name, queue_t *queue, void print_elem(void*)){
    int i=0;
    queue_t *aux;
    aux = queue;
    printf("%s: [", name);
    if(queue != NULL){								//fila não vazia
        aux = queue;
	do{		
	    print_elem(aux);
	    aux = aux->next;
	    i++;
	    if(aux != queue)
		printf(" ");
	}while(aux != queue);
    }
    printf("]\n");
}
