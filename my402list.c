//
// Created by surajhs04 on 9/17/16.
//
#ifndef _MY402LIST_C_
#define _MY402LIST_C_

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include "my402list.h"


/*
 * Returns TRUE if the list is empty
 */
int My402ListEmpty(My402List *list) {
    if (list->num_members == 0)
        return TRUE;

    return FALSE;
}


/*
 * Returns the number of members in list
 */
int My402ListLength(My402List *list) {
    return list->num_members;
}


/*
 * Returns the first list element or NULL if the list is empty.
 */
My402ListElem *My402ListFirst(My402List *list) {
    int isListEmpty = My402ListEmpty(list);
    if (isListEmpty) {
        return NULL;
    }
    return list->anchor.next;
}


/*
 * Returns the last list element or NULL if the list is empty
 */
My402ListElem *My402ListLast(My402List *list) {
    int isListEmpty = My402ListEmpty(list);
    if (isListEmpty) {
        return NULL;
    }
    return list->anchor.prev;
}


/*
 * Returns elem->next or NULL if elem is the last item on the list. Do not check if elem is on the list.
 */
My402ListElem *My402ListNext(My402List *list, My402ListElem *elem) {
    if (elem->next == &(list->anchor)) {
        return NULL;
    } else {
        return elem->next;
    }
}


/*
 * Returns elem->prev or NULL if elem is the first item on the list. Do not check if elem is on the list.
 */
My402ListElem *My402ListPrev(My402List *list, My402ListElem *elem) {
    if (elem->prev == &(list->anchor)) {
        return NULL;
    } else {
        return elem->prev;
    }
}


/*
 * Returns the list element elem such that elem->obj == obj. Returns NULL if no such element can be found.
 */
My402ListElem *My402ListFind(My402List *list, void *obj) {
    My402ListElem *my402ListElem = NULL;
    for (my402ListElem = My402ListFirst(list); my402ListElem != NULL;
         my402ListElem = My402ListNext(list, my402ListElem)) {
        if (my402ListElem->obj == obj) {
            return my402ListElem;
        }
    }
    return NULL;
}


/*
 * If list is empty, just add obj to the list. Otherwise, add obj after Last().
 * This function returns TRUE if the operation is performed successfully and returns FALSE otherwise
 */
int My402ListAppend(My402List *list, void *obj) {

    My402ListElem *listElem = NULL;
    My402List *my402List = NULL;

    my402List = list;

    listElem = malloc(sizeof(My402ListElem));
    if (listElem == NULL) {
        fprintf(stderr, "ERROR : Couldn't allocate memory = %s", strerror(errno));
        return FALSE;
    }

    if (My402ListEmpty(list)) {
        my402List->num_members = 1;
        listElem->obj = obj;
        my402List->anchor.next = listElem;
        listElem->next = &(my402List->anchor);
        my402List->anchor.prev = listElem;
        listElem->prev = &(my402List->anchor);
    } else {
        my402List->num_members++;
        listElem->obj = obj;
        listElem->next = &(my402List->anchor);
        listElem->prev = my402List->anchor.prev;
        my402List->anchor.prev->next = listElem;
        my402List->anchor.prev = listElem;
    }

    return TRUE;
}

/*
 * If list is empty, just add obj to the list. Otherwise, add obj before First().
 * This function returns TRUE if the operation is performed successfully and returns FALSE otherwise.
 */
int My402ListPrepend(My402List *list, void *obj) {

    My402ListElem *listElem = NULL;
    My402List *my402List = NULL;

    my402List = list;

    listElem = malloc(sizeof(My402ListElem));
    if (listElem == NULL) {
        fprintf(stderr, "ERROR : Couldn't allocate memory = %s", strerror(errno));
        return FALSE;
    }

    if (My402ListEmpty(list)) {
        my402List->num_members = 1;
        listElem->obj = obj;
        my402List->anchor.next = listElem;
        listElem->next = &(my402List->anchor);
        my402List->anchor.prev = listElem;
        listElem->prev = &(my402List->anchor);
    } else {
        my402List->num_members++;
        listElem->obj = obj;
        listElem->next = my402List->anchor.next;
        listElem->prev = &(my402List->anchor);
        my402List->anchor.next->prev = listElem;
        my402List->anchor.next = listElem;
    }

    return TRUE;
}


/*
 * Insert obj between elem and elem->next. If elem is NULL, then this is the same as Append().
 * This function returns TRUE if the operation is performed successfully and returns FALSE otherwise.
 * Do not check if elem is on the list.
 */
int My402ListInsertAfter(My402List *list, void *obj, My402ListElem *my402ListElem) {

    if (my402ListElem == NULL) {
        My402ListAppend(list, obj);
    } else {
        My402ListElem *newElem = NULL;
        newElem = malloc(sizeof(My402ListElem));
        if (newElem == NULL) {
            fprintf(stderr, "ERROR : Couldn't allocate memory = %s", strerror(errno));
            return FALSE;
        }

        newElem->obj = obj;
        newElem->next = my402ListElem->next;
        newElem->prev = my402ListElem;
        my402ListElem->next->prev = newElem;
        my402ListElem->next = newElem;
        list->num_members++;
    }

    return TRUE;
}

/*
 * Insert obj between elem and elem->prev. If elem is NULL, then this is the same as Prepend().
 * This function returns TRUE if the operation is performed successfully and returns FALSE otherwise.
 * Do not check if elem is on the list.
 */
int My402ListInsertBefore(My402List *list, void *obj, My402ListElem *my402ListElem) {

    if (my402ListElem == NULL) {
        My402ListPrepend(list, obj);
    } else {
        My402ListElem *newElem = NULL;
        newElem = malloc(sizeof(My402ListElem));
        if (newElem == NULL) {
            fprintf(stderr, "ERROR : Couldn't allocate memory = %s", strerror(errno));
            return FALSE;
        }

        newElem->obj = obj;
        newElem->next = my402ListElem;
        newElem->prev = my402ListElem->prev;
        my402ListElem->prev->next = newElem;
        my402ListElem->prev = newElem;

        list->num_members++;
    }

    return TRUE;
}


/*
 * Unlink and delete elem from the list.
 * Do not delete the object pointed to by elem and do not check if elem is on the list.
 */
void My402ListUnlink(My402List *list, My402ListElem *elem) {
    My402ListElem *previousElem = elem->prev;
    My402ListElem *nextElem = elem->next;
    previousElem->next = nextElem;
    nextElem->prev = previousElem;
    free(elem);
    list->num_members--;
}


/*
 * Unlink and delete all elements from the list and make the list empty.
 * Do not delete the objects pointed to be the list elements.
 */
void My402ListUnlinkAll(My402List *list) {
    My402ListElem *my402ListElem = NULL;

    for (my402ListElem = My402ListFirst(list); my402ListElem != NULL;
         my402ListElem = My402ListNext(list, my402ListElem)) {

        My402ListUnlink(list, my402ListElem);
    }
}


/*
 * Initialize the list into an empty list.
 * Returns TRUE if all is well and returns FALSE if there is an error initializing the list
 */
int My402ListInit(My402List *list) {
    list->anchor.obj = NULL;
    list->anchor.next = list->anchor.prev = NULL;
    list->num_members = 0;
    return TRUE;
}

#endif


