// THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING
// A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - ANDREW CHOI

#include <stdio.h>
#include<stdlib.h>
#include <pthread.h>
#include "elevator.h"

//global variables:
typedef struct {
    Dllist list_of_people;
    pthread_cond_t * door_cond;
    int counter;
} Variables;



void initialize_simulation(Elevator_Simulation *es){
    Variables * var = malloc(sizeof(Variables));
    var->list_of_people = new_dllist();
    dll_empty(var->list_of_people);
    var->door_cond = malloc(sizeof(pthread_cond_t));
    pthread_cond_init(var->door_cond, NULL);
    var->counter = 0;
    es->v = var;
    //add void pointer
}

void initialize_elevator(Elevator *e){
    //add void pointer
    
}

void initialize_person(Person *e){
    //add void pointer
}

void wait_for_elevator(Person *p){


    pthread_mutex_lock(p->es->lock);
    ((Variables*)p->es->v)->counter++;
    dll_append(((Variables *)p->es->v)->list_of_people, new_jval_v(p));
    pthread_cond_signal(((Variables *)p->es->v)->door_cond);
    pthread_mutex_unlock(p->es->lock);
    
    pthread_mutex_lock(p->lock);
    pthread_cond_wait(p->cond, p->lock);
    pthread_mutex_unlock(p->lock);
}

void wait_to_get_off_elevator(Person *p){
    pthread_mutex_lock(p->e->lock);
    pthread_cond_signal(p->e->cond);
    pthread_mutex_unlock(p->e->lock);

    pthread_mutex_lock(p->lock);
    pthread_cond_wait(p->cond, p->lock);
    pthread_mutex_unlock(p->lock);
}

void person_done(Person *p){
    pthread_mutex_lock(p->e->lock);
    pthread_cond_signal(p->e->cond);
    pthread_mutex_unlock(p->e->lock);
}

void *elevator(void *arg){
    Elevator * e = (Elevator *)arg;
    Person * temp_p;
    Variables * var = (Variables *)(e->es->v);

    //while loop to continue until duration is over
    while(1){
        pthread_mutex_lock(e->es->lock);
        while (!var->counter){
            pthread_cond_wait(var->door_cond, e->es->lock);
        }

        if(!dll_empty(var->list_of_people)){
            temp_p = (Person*)jval_v(dll_val(dll_first(var->list_of_people)));
            dll_delete_node(dll_first(var->list_of_people));
        }
    
        pthread_mutex_unlock(e->es->lock);
        --var->counter;

        temp_p->from == e->onfloor?:move_to_floor(e, temp_p->from);
        //moving the elevator to the correct floor
        open_door(e);
        //adding person to the elevator
        temp_p->e = e;

        //signal person
        pthread_mutex_lock(temp_p->lock);
        pthread_cond_signal(temp_p->cond);
        pthread_mutex_unlock(temp_p->lock);

        //block elevator
        pthread_mutex_lock(e->lock);
        pthread_cond_wait(e->cond, e->lock);
        pthread_mutex_unlock(e->lock);

        //person gets on and elevator goes to correct floor
        close_door(e);
        move_to_floor(temp_p->e, temp_p->to);
        open_door(e);

        //signal person
        pthread_mutex_lock(temp_p->lock);
        pthread_cond_signal(temp_p->cond);
        pthread_mutex_unlock(temp_p->lock);
        
        //block elevator again
        pthread_mutex_lock(e->lock);    
        pthread_cond_wait(e->cond, e->lock);
        pthread_mutex_unlock(e->lock);

        //person gets off elevator and the door is closed
        close_door(e);
        
        //loop goes again (does not terminate)
    }

    return NULL;
}