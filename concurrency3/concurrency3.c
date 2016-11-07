// Concurrency 3 Jake Smith & Steven Silvers

#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<unistd.h>
#include<pthread.h>
#include<string.h>




//define contents of linked list objects
struct list_obj{
	int object_ID;
	struct list_obj* next;	
};


//frame for linked list
struct list{
	int objects;
	struct list_obj* head;
	struct list_obj* current;
};


//global variables

//used as the locks in the various threads
int num_searchers;
int num_inserters;
int num_deleters;
//the list being acted upon by the threads
struct list the_list;




//functions

//searcher function, takes ID of desired object as argument
void *searcher(void *item){
	//
	struct list_obj *desired_obj = the_list.head;
	int desired_ID = *((int *) item);

	//check for deleters
	while(num_deleters != 0){
		printf("search waiting on deleter to exit\n");
		sleep(3);
	}

	//safe to assume no more deleters
	++num_searchers;
	printf("Started a search\n");

	//check if desired ID is out of scope
	if(desired_ID > the_list.objects){
		printf("Object with desired ID number not found\n");
		--num_searchers;
		return;
	}

	//go through the list
	int i;
	for(i = 0; i < desired_ID; i++){
		desired_obj = desired_obj->next;
	}
	printf("Object %d found\n",desired_obj->object_ID);
	--num_searchers;
	return;
}

void *inserter(void){
	struct list_obj new_obj;

	//wait for active inserters and deleters to exit
	while(num_deleters != 0 && num_inserters != 0){
		printf("waiting for inserts and deletes to exit\n");
		sleep(3);
	}

	//begin inserting
	printf("Insert has begun\n");
	++num_inserters;
	the_list.objects = the_list.objects + 1;
	new_obj.object_ID = the_list.objects;


	//find the end of the list
	while(the_list.current->next != NULL){
		the_list.current = the_list.current->next;
	}

	//add new object to end of list
	the_list.current->next = &new_obj;
	the_list.current = the_list.current->next;
	the_list.current->next = NULL;


	printf("New item %d added to list\n",new_obj.object_ID );

	--num_inserters;
	return;
}


void *deleter(void *item){

	struct list_obj *obj_to_del = the_list.head;
	struct list_obj *temp;
	int desired_ID = *((int *) item);

	//wait for active deletes
	while(num_deleters != 0){
		printf("waiting for delete to exit\n");
		sleep(3);
	}
	//lock out other deleters
	++num_deleters;

	//wait for searches and inserts
	while( num_inserters != 0 && num_searchers != 0){
		printf("waiting on searches and inserts\n");
		sleep(3);
	}

	printf("Delete has started\n");

	//protect the head from deletion
	if(desired_ID == 0){
		printf("Requested delete is the head, delete will not occur\n");
		--num_deleters;
		return;
	}

	//check if requested delete exists on list
	if(desired_ID > the_list.objects){
		printf("the ID requested doesn't exist in list\n");
		--num_deleters;
		return;
	}

	//find object to remove from list
	int i;
	for(i = 0; i < desired_ID; i++){
		if(i ==(desired_ID-1)){
			temp = obj_to_del;
		}
		obj_to_del = obj_to_del->next;
	}

	//remove object
	temp->next = obj_to_del->next;
	free(obj_to_del);
	--the_list.objects;


	//reassign ID values
	the_list.current = the_list.head;
	i = 0;
	for(i = 0; i < the_list.objects; i++){

	the_list.current->object_ID = i;
	the_list.current = the_list.current->next;	
	}

	printf("delete finished\n");
	--num_deleters;
	return;
}

int main(){

	//initialize list
	struct list_obj init_head;
	init_head.next = NULL;
	init_head.object_ID = 0;
	the_list.head = &init_head;

	num_deleters = 0;
	num_inserters = 0;
	num_searchers = 0;

	//create threads
	pthread_t thread0, thread1, thread2, thread3, thread4;
	int *i = malloc(sizeof(*i));
	i = 2;
	pthread_create(&thread0, NULL, inserter, NULL);
	pthread_create(&thread1, NULL, inserter, NULL);
	pthread_create(&thread2, 0, searcher, (void *) i);
	pthread_create(&thread3, 0, deleter, (void *) i);
	pthread_create(&thread4, 0, searcher, (void *) i);

	pthread_join(thread0,NULL);
	pthread_join(thread1,NULL);
	pthread_join(thread2,NULL);
	pthread_join(thread3,NULL);
	pthread_join(thread4,NULL);



	//run forever
	for(;;){

	}
}