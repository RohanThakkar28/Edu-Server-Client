// Lab11 MsgQ Server process
// Compilation of this file
// gcc -o msgqsrv lab11_server.c -lrt
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <semaphore.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#define MSG_VAL_LEN  16
// For the client queue message
#define CLIENT_Q_NAME_LEN 16
// For the server queue message
#define MSG_TYPE_LEN 16
typedef struct{
	char msg_type[MSG_TYPE_LEN];
	char msg_val[MSG_VAL_LEN];
} server_msg_t;

typedef struct{
	int is_deleted;
	int course_id;
	int teacher_id;
} Courses;
typedef struct{
	char client_q[CLIENT_Q_NAME_LEN];
	char msg_val[MSG_VAL_LEN];
} client_msg_t;




typedef struct{
	int is_deleted;
	int teacher_id;
} Teachers;
sem_t bin_sem;
static client_msg_t client_msg;
int number_courses = 0;
int actual_teachers = 0;
int number_teachers = 0;

Courses Course_Rec[700];
Teachers Teacher_Rec[700];

#define SERVER_QUEUE_NAME   "/server_msgq"
#define QUEUE_PERMISSIONS 0660
#define MAX_MESSAGES 10
#define MAX_MSG_SIZE sizeof(client_msg_t)
#define MSG_BUFFER_SIZE (MAX_MSG_SIZE * MAX_MESSAGES) 

 
int teachers_duplicate(int teacher_id) {
	for (int i = 0; i < number_teachers; i++) {
		if (Teacher_Rec[i].teacher_id == teacher_id && Teacher_Rec[i].is_deleted == 0) {
			return 1;
		}
	}
	return 0;
}



int courses_duplicate(int course_id) {
	for (int i = 0; i < number_courses; i++) {
		if (Course_Rec[i].course_id == course_id && Course_Rec[i].is_deleted == 0) {

			return 1;
		}
	}
	return 0;
}
void* Thread_function_main(void* Thread) {

	while(1) {

		sem_wait(&bin_sem);

		for (int i = 0; i < number_courses; i++) {
			if (Course_Rec[i].is_deleted == 0) 
				printf("COURSE ID: %d, TEACHER ID: %d\n", Course_Rec[i].course_id, Course_Rec[i].teacher_id);
		}
		printf("\n");
		for (int j = 0; j < actual_teachers; j++) {
			if (Teacher_Rec[j].is_deleted == 0) 
				printf("TEACHER ID: %d\n", Teacher_Rec[j].teacher_id);
		}

		sem_post(&bin_sem);

		sleep(5);

	}

}







int oper(char *input_array[], int count) {

	char *ADD = "add";
	char *DEL = "del";
	char *COR = "course";
	char *TCH = "teacher";
	char *EXIT = "exit";
	if (*input_array[0] == *ADD) {

		if (*input_array[1] == *COR) {
			int pos = 2;
			while(input_array[pos]) {
				int value = strtol(input_array[pos], NULL, 10);
				int dup = courses_duplicate(value);
				pos++;
				if (dup == 1) {

				} else {

					int ID = strtol(input_array[pos], NULL, 10);
					Courses newCourse = {0, value, ID};
					int tempDup = teachers_duplicate(ID);
					if (tempDup == 1) {
						if (number_courses < 700) {
							Course_Rec[number_courses++] = newCourse;
						} else {
							for (int i = 0; i < 700; i++) {
								if (Course_Rec[i].is_deleted == 1) {
									Course_Rec[i] = newCourse;
								}
							}
						}
					}
				}
				pos++;
			}
		} else if(*input_array[1] == *TCH) {
			int pos = 2;
			while(input_array[pos]) {
				int value = strtol(input_array[pos], NULL, 10);
				int dup = teachers_duplicate(value);
				if (dup == 1) {

				} else {
					Teachers newTeacher = {0, value};
					if (number_teachers < 700) {
						Teacher_Rec[number_teachers++] = newTeacher;
						++actual_teachers;
					} else {
						for (int i = 0; i < 700; i++) {
							if (Teacher_Rec[i].is_deleted == 1) {
								Teacher_Rec[i] = newTeacher;
							}
						}
					}
				}
				pos++;
			}
		} else {
			printf("Invalid input!\n");
		}

	} else if (*input_array[0] == *DEL) {
		if (*input_array[1] == *COR) {
			int pos = 2;	
			while (input_array[pos]) {
				int n;
				int value = strtol(input_array[pos], NULL, 10);
				if (number_courses < 700) {
					n = number_courses;
				} else{
					n = 700;
				}
				for (int i = 0; i < n; i++) {
					if (Course_Rec[i].course_id == value && Course_Rec[i].is_deleted == 0) {
						Course_Rec[i].is_deleted = 1;
					}
				}
				pos++;
			}
		} else if(*input_array[1] == *TCH) {
			int pos = 2;
			while(input_array[pos]) {
				int value = strtol(input_array[pos], NULL, 10);
				int dup = teachers_duplicate(value);
				if(!dup)
				{
					printf("Teacher does not exist \n");
					pos++;

					continue;
				}

				for (int i = 0; i < number_teachers; i++) {
				if (Teacher_Rec[i].teacher_id == value) {
						Teacher_Rec[i].is_deleted = 1;
						number_teachers--;
					}
				}

				for(int i=0; i<number_courses; i++) {
					if(Course_Rec[i].teacher_id == value) {
						int random_teacher;
						if(number_teachers == 0) {
							random_teacher = -1;
						} else {
							random_teacher = rand() % number_teachers;
							while(Teacher_Rec[random_teacher].is_deleted == 1) {
								random_teacher = rand() % number_teachers;
							}
							Course_Rec[i].teacher_id = Teacher_Rec[random_teacher].teacher_id;
						}
					}
				}

				pos++;
			}
		} 
	return 0;
	} else if(*input_array[0] == *EXIT)
	{
		
		FILE *fp;
		fp = fopen("output.txt","w");
		if(fp == NULL)
		{
			printf("Error opening file");
			exit(1);
		}
		
		for (int i = 0; i < number_courses; i++) {
			if (Course_Rec[i].is_deleted == 0) 
				fprintf(fp,"COURSE ID: %d, TEACHER ID: %d\n", Course_Rec[i].course_id, Course_Rec[i].teacher_id);
		}
		printf("\n");
		for (int j = 0; j < number_teachers; j++) {
			if (Teacher_Rec[j].is_deleted == 0) 
				fprintf(fp,"TEACHER ID: %d\n", Teacher_Rec[j].teacher_id);
		}

		
	}
}


int main (int argc, char **argv)
{
    mqd_t qd_srv, qd_client;   // Server and Client Msg queue descriptors
    int num = 1, res_sem, sem;
    pthread_t thread;
    res_sem = sem_init(&bin_sem, 0, 1);
    if (res_sem != 0) {
    	printf("Semaphore creation has failed: %d \n", res_sem);
    	exit(1);
    }
    if((sem = pthread_create(&thread, NULL, &Thread_function_main, "Report summary thread"))) {
    	printf("Thread creation failed: %d \n", sem);
    	exit(1);
    }
    printf ("Server MsgQ: Welcome!!!\n");
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;
    if ((qd_srv = mq_open(SERVER_QUEUE_NAME, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS,

                           &attr)) == -1) {

        perror ("Server MsgQ: mq_open (qd_srv)");
        exit (1);
    }
    client_msg_t in_msg;
	char* val_client;
	char* quit = "exit";
    while (1) {
	
        if (mq_receive(qd_srv,(char *) &in_msg, MAX_MSG_SIZE, NULL) == -1) {
            perror ("Server msgq: mq_receive");
            exit (1);
        }
	val_client = in_msg.msg_val;
        printf ("%d: Server MsgQ: message received.\n", num);
        printf("Client msg q name = %s\n", in_msg.client_q);
        printf("Client msg val = %s\n", val_client);
        if (*val_client == *quit) {

			FILE *fp;
		fp = fopen("output.txt","w");
		if(fp == NULL)
		{
			printf("Error opening file");
			exit(1);
		}
		
		for (int i = 0; i < number_courses; i++) {
			if (Course_Rec[i].is_deleted == 0) 
				fprintf(fp,"COURSE ID: %d, TEACHER ID: %d\n", Course_Rec[i].course_id, Course_Rec[i].teacher_id);
		}
		printf("\n");
		for (int j = 0; j < number_teachers; j++) {
			if (Teacher_Rec[j].is_deleted == 0) 
				fprintf(fp,"TEACHER ID: %d\n", Teacher_Rec[j].teacher_id);
		}

        	printf("Exiting!\n");
        	break;
        }
    char *tokenizedString[100];
	int count = 0;
	tokenizedString[count++] = strtok(val_client, " ");
	while(tokenizedString[count - 1] != NULL) {
		tokenizedString[count++] = strtok(NULL, " "); 
	}

	int operation = oper(tokenizedString, count);
	char* out;
	if (operation == 0) {
		out = "Successful!\n";
	} else {
		out = "Failed to do the operation!\n";
	}
	printf("Courses: %d \n", number_courses);
	printf("Teachers: %d \n", number_teachers);
	server_msg_t out_msg; 
	strcpy(out_msg.msg_type, "Server msg");   // strcpy(destPtr, srcPtr)
	sprintf (out_msg.msg_val, "%s", out);    
		// Open the client queue using the client queue name received
        if ((qd_client = mq_open(in_msg.client_q, O_WRONLY)) == 1) {
            perror ("Server MsgQ: Not able to open the client queue");
            continue;
        }     	
        // Send back the value received + 10 to the client's queue           
        // int mq_send(mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned int msg_prio);
        if (mq_send(qd_client, (char *) &out_msg, sizeof(out_msg), 0) == -1) {
            perror ("Server MsgQ: Not able to send message to the client queue");
            continue;
        }  
        //sem_post(&bin_sem);    
    } // end of while(1) 

}  // end of main function




