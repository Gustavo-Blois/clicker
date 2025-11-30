#ifndef LIST_H
#define LIST_H

typedef struct node {
	void *data;
	struct node *next;
} list;

struct node* list_push_back(void* e, list** l);
void free_list(list** l);
void list_remove(struct node* it, list** l);

#endif
