#include "LList.h"
#include <cstdlib>
#include <iostream>

using namespace std;

//constructor for an empty linked listake
linkedList::linkedList()
{
    head = NULL;
    tail = NULL;
    length = 0;
}

//add an item to the end of the list
int linkedList::append(void *data)
{
    node_t *node = new node_t; //creates a node for the list

    node->data = data; //stores the requested data in the node
    node->next = NULL;

    if(head == NULL){ //checks if this is the first node in the list
        head = node; //Updates the had and tail addresses for the list
        tail = node;
    }else{
        tail->next = node; //updates the current tail node to point to the new node
        tail = node;//sets the new node as the tail node
    }

    length++; //Increments the length of the list

    return 0;
}

int linkedList::insert(int index, void *data)
{
    if(index > length - 1)
    {
        return(append(data));
    }
    node_t *newNode = new node_t;

    newNode->data = data; //stores the requested data in the node
    newNode->next = NULL;
    
    if(index == 0)
    {
        newNode->next = head;
        head = newNode;
        length++;

        return 0;
    }

    
    
    
    node_t *currentNode = head;
    for(int i = 0; i < index - 1; i ++)
    {
        currentNode = currentNode->next;
    }

    newNode->next = currentNode->next;
    currentNode->next = newNode;
    length++;
    
    return 0;
}

//remove a node from the list at a certain index
int linkedList::remove(int index)
{
    if(index >= length) //checks that the index exists in the list
    {
        return 1;
    }

    if(index == 0) //Checks if the node being removed is the head
    {
        //void *data = head->data;
        node_t *node_to_remove = head;
        head = head->next; //Sets the head of the list to the second node in the list

        //free(node_to_remove->data);
        delete(node_to_remove); //Frees the memory of the node being removed

        if(head == NULL){ //Checks if the list is now empty
            tail == NULL;
        }

        length--; //decrements the length counter
        return 0;
    }

    int current_index = 0;

    node_t* node = head;

    while(current_index < index - 1) //itertes through the list to the node before the one that was removed
    {
        current_index++;
        node = node->next;
    }

    node_t* node_to_remove = node->next;
    node->next = node_to_remove->next; //Updates the pointer in this node to the next node in the list
    //void *data = node_to_remove->data;

    if(index == length - 1) //Checks if the tail node was removed
    {
        tail = node; //Updates the pointer to the last node in the list
    }

    //free(node_to_remove->data);
    free(node_to_remove); //Freese the memory of the node being removed
    length--; //Decreases the length of the list

    return 0;
}

//returns the address of the first item of the list
node_t *linkedList::getHead()
{
    return head;
}

//returns the next item in the list when given the address of an itme in the list
node_t *linkedList::getNext(node_t *node)
{
    return node->next;
}

//return the length of the list
int linkedList::getLen()
{
    return length;
}

//get the address of the last itme in the list
node_t *linkedList::getTail()
{
    return tail;
}

//destructor for the linked list to free up all the memory reserved
linkedList::~linkedList()
{
    while(length > 0)
    {
        remove(length - 1); //Removes all the nodes before removing the list
    }
}