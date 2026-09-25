#include <stdio.h>
#include <stdlib.h>

typedef struct node {
    int data;
    struct node *link;
} node;

typedef struct {
    node *head;
    node *tail;
    int count;
} list;

node *createnode(int data) {
    node *newnode = malloc(sizeof(node));
    if (newnode != NULL) {
        newnode->data = data;
        newnode->link = NULL;
    }
    return newnode;
}

void printlist(node *head) {
    node *temp = head;
    printf("list: ");
    while (temp != NULL) {
        printf("%d -> ", temp->data);
        temp = temp->link;
    }
    printf("NULL\n");
}

list *createlist(void) {
    list *newlist = malloc(sizeof(list));
    if (newlist == NULL)
        return NULL;
    
    newlist->head = NULL;
    newlist->tail = NULL;
    newlist->count = 0;
    
    return newlist;
}

int insertfront(list *l, int data) {
    if (l == NULL)
        return -3;
        
    node * newnode = createnode(data);
    
    if (newnode == NULL)
        return -1;
        
    if (l->head == NULL) {
        l->head = newnode;
        l->tail = newnode;
    } else {
        newnode->link = l->head;
        l->head = newnode;
    }
    
    l->count++;
    return 0;
}

int insertend(list *l, int data) {
    if (l == NULL)
        return -3;
        
    node * newnode = createnode(data);
    
    if (newnode == NULL)
        return -1;
        
    if (l->head == NULL) {
        l->head = newnode;
        l->tail = newnode;
    } else {
        l->tail->link = newnode;
        l->tail = newnode;
    }
    
    l->count++;
    return 0;
}

int insertposition(list *l, int data, int position) {
    if (l == NULL)
        return -3;

    if (position < 1)
        return -2;

    if (position > l->count + 1)
        return -2;

    node *newnode = createnode(data);
    
    if (newnode == NULL)
        return -1;
        
    if (position == 1) {
        newnode->link = l->head;
        l->head = newnode;
        if (l->tail == NULL)
            l->tail = newnode;
    } else {
        node *temp = l->head;
        
        for (int i = 1; i < position - 1 && temp != NULL; i++) {
            temp = temp->link;
        }
        
        if (temp == NULL) {
            free(newnode);
            return -2;
        }
        
        newnode->link = temp->link;
        temp->link = newnode;
        if (newnode->link == NULL)
            l->tail = newnode;
    }

    l->count++;
    return 0;
}

int deletefront(node ** head) {
    if (head == NULL)
        return -3;
        
    if (*head == NULL)
        return -4;
        
    node * temp = *head;
    *head = (*head)->link;
    free(temp);
    
    return 0;
}

int deleteend(node ** head) {
    if (head == NULL)
        return -3;
        
    if (*head == NULL)
        return -4;
        
    if ((*head)->link == NULL) {
        free(*head);
        *head = NULL;
        return 0;
    }
    
    node *temp = *head;
    while (temp->link->link != NULL) {
        temp = temp->link;
    }
    free(temp->link);
    temp->link = NULL;
    
    return 0;
}

int deletepos(node ** head, int pos) {
    node * temp, * prev;
    
    if (head == NULL) 
        return -3;
        
    if (*head == NULL) 
        return -4;
        
    if (pos < 1)
        return -2;
        
    if (pos == 1) {
        temp = *head;
        *head = (*head)->link;
        free(temp);
        return 0;
    }
    
    prev = *head;
    temp = (*head)->link;
    
    for (int i = 2; i < pos && temp != NULL; i++) {
        prev = temp;
        temp = temp->link;
    }
    
    if (temp == NULL)
        return -3;
        
    prev->link = temp->link;
    free(temp);
    
    return 0;
}

int deletekey(node ** head, int key) {
    if (head == NULL)
        return -3;
        
    if (*head == NULL)
        return -4;
        
    node * temp;
    node * prev;
    
    if ((*head)->data == key) {
        temp = *head;
        *head = (*head)->link;
        free(temp);
        return 0;
    }
    
    prev = *head;
    temp = (*head)->link;
    
    while (temp != NULL) {
        if (temp->data == key) {
            prev->link = temp->link;
            free(temp);
            return 0;
        }
        prev = temp;
        temp = temp->link;
    }
    
    return -1; 
}

int deletelist(node ** head) {
    node * temp;
    
    if (head == NULL)
        return -3;
        
    if (*head == NULL)
        return -4;
        
    while (*head != NULL) {
        temp = *head;
        *head = (*head)->link;
        free(temp);
    }
    
    return 0;
}

int search(node * head, int key, int * pos) {
    node * temp;
    int position = 1;
    
    if (pos == NULL)
        return -3;
        
    if (head == NULL)
        return -4;
        
    temp = head;
    
    while (temp != NULL) {
        if (temp->data == key) {
            *pos = position;
            return 0;
        }
        temp = temp->link;
        position++;
    }
    
    return -1; 
}

int main() {
    list *mylist = createlist();
    if (mylist == NULL) {
        printf("failed to create list.\n");
        return 1;
    }

    insertfront(mylist, 20); 
    insertfront(mylist, 10); 
    insertend(mylist, 40);   
    insertposition(mylist, 30, 3);
    
    printlist(mylist->head);

    int position;
    if (search(mylist->head, 30, &position) == 0) {
        printf("key 30 found at position: %d\n", position);
    } else {
        printf("key 30 not found.\n");
    }

    deletefront(&(mylist->head));
    printf("after deletefront: ");
    printlist(mylist->head); 

    deleteend(&(mylist->head));
    printf("after deleteend: ");
    printlist(mylist->head); 

    deletekey(&(mylist->head), 20);
    printf("after deletekey (20): ");
    printlist(mylist->head); 

    deletelist(&(mylist->head));
    printlist(mylist->head); 
    
    free(mylist); 
    printf("memory freed successfully.\n");

    return 0;
}