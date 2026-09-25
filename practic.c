#include<stdio.h>
#include<stdlib.h>
#include<string.h>

typedef struct node{
    int data;
    struct node *link;
} node;
typedef struct list{
    node *head;
    node *tail;
    int count;
} list;
//node creation
node *createnode(int data){
    node *newnode=malloc(sizeof(node));
    if(newnode==NULL){
        printf("memory allocation failed");
        return NULL;
    }
    newnode->data=data;
    newnode->link=NULL;
    return newnode;
}
//list creation
list *createlist(){
    list *newlist=malloc(sizeof(list));
    if(newlist==NULL){
        printf("failed to allocate the memory");
        return NULL;
    }
    newlist->head=NULL;
    newlist->tail=NULL;
    newlist->count=0;
    return newlist;
}

int insertfront(list *l,int data){
    if (l==NULL) return -3;
    node *newnode=createnode(data);
    if(newnode==NULL){
        return -1;

    }
    if(l->head==NULL){
        l->head=newnode;
        l->tail=newnode;
    }
    else {
        newnode->link=l->head;
        l->head=newnode;
    }
    l->count++;
    return 0;
}
int insertposition(list *l,int data,int p){
    if (l==NULL) return -3;
    if (p<1 || p>l->count+1) return -2;

    if (p==1) {
        return insertfront(l, data);
    }

    node *previous=l->head;
    for (int i=1; i<p-1; i++) {
        previous=previous->link;
    }

    node *newnode=createnode(data);
    if (newnode==NULL) return -1;

    newnode->link=previous->link;
    previous->link=newnode;
    if (newnode->link==NULL) l->tail=newnode;
    l->count++;
    return 0;
}

int main(void){
    list *numbers=createlist();
    if (numbers==NULL) return 1;

    insertfront(numbers, 20);
    insertfront(numbers, 10);
    insertposition(numbers, 15, 2);

    printf("List: ");
    for (node *current=numbers->head; current!=NULL; current=current->link) {
        printf("%d", current->data);
        if (current->link!=NULL) printf(" -> ");
    }
    printf("\nCount: %d\n", numbers->count);

    node *current=numbers->head;
    while (current!=NULL) {
        node *next=current->link;
        free(current);
        current=next;
    }
    free(numbers);
    return 0;
}