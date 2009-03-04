#ifndef __GA_LIST_H__
#define __GA_LIST_H__
#define GA_NAME_LENGTH 64
typedef struct ga_node_s{
	void *data;
	struct ga_node_s *next;
}ga_node_t;

typedef struct ga_list_s{
	int size;
	ga_node_t *first;
	ga_node_t *last;
}ga_list_t;

ga_list_t *ga_list_new(void);
void	ga_list_add(ga_list_t *l, void *data);
int	ga_list_size(ga_list_t *l);
void*	ga_list_find(ga_list_t *l, const char *name);

#endif


