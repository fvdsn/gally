#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ga_list.h"


ga_list_t *ga_list_new(void){
	ga_list_t *l = (ga_list_t*)malloc(sizeof(ga_list_t));
	l->size = 0;
	l->first = NULL;
	l->last  = NULL;
	return l;
}
void	ga_list_add(ga_list_t *l, void *data){
	ga_node_t *n = (ga_node_t*)malloc(sizeof(ga_node_t));
	n->data = data;
	n->next = NULL;
	if(l->size == 0){
		l->first = n;
		l->last  = n;
		l->size = 1;
	}else{
		l->last->next = n;
		l->last = n;
		l->size++;
	}
}
int	ga_list_size(ga_list_t *l){
	return l->size;
}
void	ga_list_free(ga_list_t *l){
	ga_node_t* n = l->first;
	ga_node_t* next = l->first;
	while(n){
		next = n->next;
		free(n);
		n = next;
	}
	free(l);
}
void*	ga_list_find(ga_list_t *l, const char *name){
	ga_node_t * n = l->first;
	char *n_name = NULL;
	while(n){
		n_name = (char*)n->data;
		if(!strncmp(name,n_name,GA_NAME_LENGTH)){
			return n->data;
		}
		n = n->next;
	}
	return NULL;
}

		
