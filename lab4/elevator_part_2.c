// THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING
// A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - ANDREW CHOI

#include <stdio.h>
#include<stdlib.h>
#include <pthread.h>
#include "elevator.h"

//global variables:
typedef struct {
    Dllist * list_of_people;
    pthread_cond_t * door_cond;
    pthread_mutex_t *locks;
} Variables;



void initialize_simulation(Elevator_Simulation *es){
    Variables * var = malloc(sizeof(Variables));
    var->list_of_people = malloc((es->nfloors*2) * sizeof(Dllist));
    int i;
    for(i = 0; i < es->nfloors * 2; i++){
        var->list_of_people[i] = new_dllist();
    }
    var->door_cond = malloc(sizeof(pthread_cond_t));
    pthread_cond_init(var->door_cond, NULL);
    var->locks = malloc(es->nfloors * sizeof(pthread_mutex_t));
    for(i = 0; i < es->nfloors; i++)
    {
        pthread_mutex_init(&var->locks[i], NULL);
    }

    es->v = var; 
    
}

void initialize_elevator(Elevator *e){

    Dllist * on_elevator = malloc(sizeof(Dllist) * e->es->nfloors);
    for (int i = 0; i < e->es->nfloors; i++) {
        on_elevator[i] = new_dllist();
    }

    e->v = on_elevator;
    
}

void initialize_person(Person *e){
    //add void pointer
}

void wait_for_elevator(Person *p){

    Variables * var = (Variables *)p->es->v;

    //going up
    if(p->from < p->to){
        pthread_mutex_lock(&(var)->locks[p->from - 1]);
        dll_append(var->list_of_people[p->from - 1], new_jval_v(p));
        pthread_mutex_unlock(&(var)->locks[p->from - 1]);
    }
    //going down
    else{
        pthread_mutex_lock(&(var)->locks[p->from - 1]);
        dll_append(var->list_of_people[(p->es->nfloors) + p->from - 1], new_jval_v(p));
        pthread_mutex_unlock(&(var)->locks[p->from - 1]);
    }
    
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
    int dir = 0; //0 is up, 1 is down

    Elevator * e = (Elevator *)arg;
    Person * temp_p;
    Dllist *e_var = e->v;
    Variables * var = (Variables *)(e->es->v);

    //while loop to continue until duration is over
    while(1){
        
        //getting off the elevator
        while(e_var[e->onfloor-1]->flink  != e_var[e->onfloor -1]){
            //make sure door is open
            if (!e->door_open){
                open_door(e);
            }

            //person leaves
            temp_p = (Person*)jval_v(e_var[e->onfloor - 1]->flink->val);
            dll_delete_node(e_var[e->onfloor - 1]->flink);

            //signal person
            pthread_mutex_lock(temp_p->lock);
            pthread_cond_signal(temp_p->cond);
            pthread_mutex_unlock(temp_p->lock);

            // have the elevator wait
            pthread_mutex_lock(e->lock);
            pthread_cond_wait(e->cond, e->lock);
            pthread_mutex_unlock(e->lock);

        }

        //getting on the elevator
        //critical section; make sure to lock
        pthread_mutex_lock(&var->locks[e->onfloor - 1]);
        while(var->list_of_people[dir * e->es->nfloors + e->onfloor - 1]->flink != var->list_of_people[dir * e->es->nfloors + e->onfloor - 1]){
            //make sure door is open
            if (!e->door_open){
                open_door(e);
            }
            temp_p = (Person*)jval_v(var->list_of_people[dir * e->es->nfloors +  e->onfloor - 1]->flink->val);
            temp_p->e = e;
            dll_append(e_var[temp_p->to - 1], new_jval_v(temp_p));
            dll_delete_node(var->list_of_people[dir * e->es->nfloors + e->onfloor - 1]->flink);

            //signal person
            pthread_mutex_lock(temp_p->lock);
            pthread_cond_signal(temp_p->cond);
            pthread_mutex_unlock(temp_p->lock);

            // have the elevator wait
            pthread_mutex_lock(e->lock);
            pthread_cond_wait(e->cond, e->lock);
            pthread_mutex_unlock(e->lock);
        }
        pthread_mutex_unlock(&var->locks[e->onfloor - 1]);

        if (e->door_open){
            close_door(e);
        }
        
        move_to_floor(e, (e->onfloor + ( -2 * dir + 1 ))); 
        if (e->onfloor == 1){
            dir = 0;
        }
        else if (e->onfloor == e->es->nfloors){
            dir = 1; 
        }

        //loop goes again (does not terminate)
    }

    return NULL;
}