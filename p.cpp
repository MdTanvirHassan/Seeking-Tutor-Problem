#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
 
struct priority_queue
{
int priority_no;
int time;
};
 
int finished_student_no=0, finished_tutor_no = 0, request_no = 0, student_no=0, tutor_no=0, help = 0, total_chair = 0, occupied_chairs=0;
 
int visited[100];
struct priority_queue pq[100];
int priority[100];
int student_ids[100];
int tutor_ids[100];
 
sem_t student;
sem_t coordinator;
sem_t tutor[100];
sem_t mutex;
 
void *student_thread(void *student_id)
{
    int s_id=*(int*)student_id;
 
    while(1)
	{
        if(priority[s_id-1] == help)
		{
 
			sem_wait(&mutex);
            finished_student_no++;
			sem_post(&mutex);
 
            printf("\n\nstudent %d terminates\n\n",s_id);
 
            sem_post(&student);
            pthread_exit(NULL);
        }
 
 
		sem_wait(&mutex);
        if(occupied_chairs == total_chair)
		{
            printf("\nStudent: Student %d found no empty chair.\n",s_id);
			sem_post(&mutex);
            continue;
        }
 
        occupied_chairs++;
        request_no++;
        visited[s_id-1]=request_no;// the visited array is the time of a student has come , the lower the value is the fast that student had came.
        printf("\nStudent: Student %d takes a seat.\nStudent: Empty chairs = %d\n",s_id,total_chair-occupied_chairs);
		sem_post(&mutex);
 
 
        sem_post(&student);
		sem_wait(&tutor[s_id-1]);
 
        printf("\nStudent: Student %d received help.\n",s_id);
 
		sem_wait(&mutex);
        priority[s_id-1]++;
		printf("\nStudent: Student %d priority now is %d\n",s_id, priority[s_id-1]);
		sem_post(&mutex);
    }
}
 
 
void *tutor_thread(void *tutor_id)
{
       int t_id=*(int*)tutor_id;
 
    while(1)
	{
        if(finished_student_no==student_no)
		{
        sem_wait(&mutex);
        finished_tutor_no++;
        sem_post(&mutex);
 
        sem_wait(&mutex);
            printf("\n\ntutor %d terminates\n\n",t_id);
            if(finished_tutor_no == tutor_no)
            {
            printf("\n\ncoordinator terminates\n\n");
 
            }
            sem_post(&mutex);
 
            pthread_exit(NULL);
        }
 
 
        sem_wait(&coordinator);
 
       int max_request=student_no*help+1, max_priority = help-1 ,s_id = -1;//variable
		sem_wait(&mutex);
       for(int i=0;i<student_no;i++)
		{
            if(pq[i].priority_no>-1 && pq[i].priority_no<=max_priority)
			{
                    if (pq[i].time<max_request)
                            {
                               max_priority = pq[i].priority_no;//student's pariority jabe
                               max_request=pq[i].time;//student's arrival time
                               s_id=student_ids[i];//saving student id in the array
                              }
                          }
                        }
 
        if(s_id==-1) //kno tudent payni tai
		{
			sem_post(&mutex);
            continue;
        }
 
		pq[s_id-1].priority_no = -1;
		pq[s_id-1].time = -1;
 
        occupied_chairs--;
		sem_post(&mutex);
 
		sem_wait(&mutex);
        printf("\nTutor: Student %d tutored by Tutor %d\n",s_id,t_id);
		sem_post(&mutex);
 
		sem_post(&tutor[s_id-1]);//which ever student has done tutoring it will get a signal to not wait and continue programming
 
    }
}
 
 
void *coordinator_thread(void *arg)
{
    while(1)
	{
        if(finished_student_no==student_no)
		{
            for(int i=0;i<tutor_no;i++)
			{
	        sem_post(&coordinator);
            }
            pthread_exit(NULL);
        }
 
        sem_wait(&student);
 
		sem_wait(&mutex);
        for(int i=0;i<student_no;i++)
		{
            if(visited[i]>-1)
			{
                pq[i].priority_no = priority[i];// the priority queues arry no will be priority of the student
                pq[i].time = visited[i];//priority queue time will be the visited time
 
                printf("\nCoordinator: Student %d with priority %d in the queue.\n",student_ids[i],priority[i]);
                visited[i]=-1;
 
                sem_post(&coordinator);//tutor ke coordinator signal dicche
            }
        }
		sem_post(&mutex);
    }
}
 
 
 
int main()
{
	printf("Enter total student number: ");
	scanf("%d", &student_no);
	printf("Enter total tutor number: ");
	scanf("%d", &tutor_no);
	printf("Enter total chair number: ");
	scanf("%d", &total_chair);
	printf("Enter maximum help number: ");
	scanf("%d", &help);
 
    for(int i=0;i<student_no;i++)
	{
        visited[i]=-1;
        pq[i].priority_no = -1;
        pq[i].time = -1;
        priority[i]=0;
    }
 
    sem_init(&student,0,0);
    sem_init(&coordinator,0,0);
	sem_init(&mutex,0,1);// only for critical secion we have taken 1
	for(int i=0;i<student_no;i++)
	{
        sem_init(&tutor[i],0,0);
	}
 
    pthread_t students[student_no];
    pthread_t tutors[tutor_no];
    pthread_t coordinator;
 
    for(int i = 0; i < student_no; i++)
    {
        student_ids[i] = i + 1;
		if (pthread_create(&students[i], NULL, student_thread, (void*) &student_ids[i]) < 0)
        {
            perror("Error: thread cannot be created");
           exit(1);
        }
 
    }
 
    for(int i = 0; i < tutor_no; i++)
    {
        tutor_ids[i] = i + 1;
		if (pthread_create(&tutors[i], NULL, tutor_thread, (void*) &tutor_ids[i]) < 0)
        {
            perror("Error: thread cannot be created");
           exit(1);
        }
    }
 
	if(pthread_create(&coordinator,NULL,coordinator_thread,NULL) < 0)
    {
            perror("Error: thread cannot be created");
            exit(1);
    }
 
 
    for(int i =0; i < student_no; i++)
    {
        pthread_join(students[i],NULL);
    }
 
    for(int i =0; i < tutor_no; i++)
    {
        pthread_join(tutors[i],NULL);
    }
 
	pthread_join(coordinator, NULL);
 
    return 0;
}
 
//In the code, we initialized 3 threads, students, tutors, and coordinator.
