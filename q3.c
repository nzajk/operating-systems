/*
 * q3.c
 *
 * A program that implements threads, mutex locks, and semaphores.
 * 
 * To run the program use the commands `make` and `make run` respectively.
 *
 * Nicholas Zajkeskovic
 * October 27, 2024
 * COMPSCI 3SH3
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

// Constraints for "The Sleeping TA" problem
#define NUM_STUDENTS 5
#define NUM_CHAIRS 3
#define MAX_HELP_REQUESTS 5  // Max amount of questions per student

// Shared variables
sem_t ta_sem;
sem_t student_sem;
pthread_mutex_t chair_mutex;
pthread_mutex_t help_count_mutex;

// Track waiting students
int waiting_students = 0;
int total_help_requests = 0; 
int students_done = 0; // Track number of students who are done asking questions

// Function for TA's behavior
void *ta_thread(void *arg) {
    while (1) {
        // Wait for a student to notify the TA
        sem_wait(&ta_sem);

        // Check if all students are done and the help requests are completed
        pthread_mutex_lock(&help_count_mutex);
        if (students_done == NUM_STUDENTS) {
            pthread_mutex_unlock(&help_count_mutex);
            break;
        }
        pthread_mutex_unlock(&help_count_mutex);

        pthread_mutex_lock(&chair_mutex);
        if (waiting_students > 0) {
            waiting_students--;
            printf("Helping a student for %d seconds, waiting students = %d\n", rand() % 5 + 1, waiting_students);
            pthread_mutex_unlock(&chair_mutex);

            // Simulate time taken to help a student with rand()
            sleep(rand() % 5 + 1);

            // Signal to the student that the TA is done
            sem_post(&student_sem);
        } else {
            pthread_mutex_unlock(&chair_mutex);
        }
    }
    return NULL;
}

// Function for each student's behavior
void *student_thread(void *student_id) {
    int id = *((int *)student_id);
    int help_requests = 0; 

    while (help_requests < MAX_HELP_REQUESTS) {
        // Simulate time programming before needing help with rand()
        sleep(rand() % 5 + 1);

        pthread_mutex_lock(&chair_mutex);
        if (waiting_students < NUM_CHAIRS) {
            waiting_students++;
            help_requests++;
            total_help_requests++; // Increment the amount of times the student asks for help
            printf("Student %d takes a seat, waiting = %d\n", id, waiting_students);
            
            // Notify TA if they're sleeping
            sem_post(&ta_sem);
            pthread_mutex_unlock(&chair_mutex);

            // Wait for TA to finish helping
            sem_wait(&student_sem);
        } else {
            printf("Student %d will try later\n", id);
            pthread_mutex_unlock(&chair_mutex);
        }
    }

    // Mark this student as done
    pthread_mutex_lock(&help_count_mutex);
    students_done++;
    pthread_mutex_unlock(&help_count_mutex);

    return NULL;
}

int main() {
    srand(time(NULL));

    pthread_t ta;
    pthread_t students[NUM_STUDENTS];
    int student_ids[NUM_STUDENTS];

    // Initialize semaphores and mutexes
    sem_init(&ta_sem, 0, 0);
    sem_init(&student_sem, 0, 0);
    pthread_mutex_init(&chair_mutex, NULL);
    pthread_mutex_init(&help_count_mutex, NULL); // Initialize help count mutex

    // Create the TA thread
    pthread_create(&ta, NULL, ta_thread, NULL);

    // Create student threads
    for (int i = 0; i < NUM_STUDENTS; i++) {
        student_ids[i] = i;
        pthread_create(&students[i], NULL, student_thread, &student_ids[i]);
    }

    // Join the student threads
    for (int i = 0; i < NUM_STUDENTS; i++) {
        pthread_join(students[i], NULL);
    }

    // After all students finish, wake the TA to check exit condition
    sem_post(&ta_sem); 
    pthread_join(ta, NULL); 

    // Cleanup
    sem_destroy(&ta_sem);
    sem_destroy(&student_sem);
    pthread_mutex_destroy(&chair_mutex);
    pthread_mutex_destroy(&help_count_mutex);

    return 0;
}