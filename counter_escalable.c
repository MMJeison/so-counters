#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sched.h>
#include <time.h>

// numero de CPUs
#define NUMCPUS 8

int nroIteraciones;

typedef struct __counter_t {
  int global; // global count
  pthread_mutex_t glock; // global lock
  int local[NUMCPUS]; // local count (per cpu)
  pthread_mutex_t llock[NUMCPUS]; // ... and locks
  int threshold; // update frequency
} counter_t;

// init: record threshold, init locks, init values
//      of all local counts and global count
void init(counter_t *c, int threshold) {
  c->threshold = threshold;
  c->global = 0;
  pthread_mutex_init(&c->glock, NULL);
  int i;
  for (i = 0; i < NUMCPUS; i++) {
    c->local[i] = 0;
    pthread_mutex_init(&c->llock[i], NULL);
  }
}


void update(counter_t *c, int cpu) {
  // int cpu = threadID % NUMCPUS;
  pthread_mutex_lock(&c->llock[cpu]);
  c->local[cpu] += 1;
  if (c->local[cpu] >= c->threshold) { // transfer to global (synchronization point)
    pthread_mutex_lock(&c->glock);
    c->global += c->local[cpu];
    pthread_mutex_unlock(&c->glock);
    c->local[cpu] = 0;
  }
  pthread_mutex_unlock(&c->llock[cpu]);
}

// get: just return global amount (which may not be perfect)
int get(counter_t *c) {
  pthread_mutex_lock(&c->glock);
  int val = c->global;
  pthread_mutex_unlock(&c->glock);
  return val; // only approximate!
}

counter_t *c;

void *thread_function(void *arg) {
    int thread_id = *((int *)arg);
    int cpu = sched_getcpu(); // get CPU where thread is running
    printf("Hilo %d iniciado en el núcleo %d\n", thread_id, cpu);
    int i;
    for (i = 0; i < nroIteraciones; i++) {
        update(c, cpu);
    }
    printf("Hilo %d terminado\n", thread_id);
    return NULL;
}

int main(int argc, char *argv[]) {
  c = malloc(sizeof(counter_t));
  int threshold = 10000;
  nroIteraciones = 2000000;
  int numThreads = 32;
  if (argc > 4) {
    printf("Invalid arguments\n");
    return 1;
  }
  if (argc > 1) {
    nroIteraciones = atoi(argv[1]);
  }
  if (argc > 2) {
    numThreads = atoi(argv[2]);
  }
  if (argc > 3) {
    threshold = atoi(argv[3]);
  }


  init(c, threshold);

  pthread_t threads[32];
  int thread_ids[32];

  clock_t inicio, fin;
  double tiempo_transcurrido;
  inicio = clock();
  int i;
  for (i = 0; i < numThreads; i++) {
    thread_ids[i] = i;
    pthread_create(&threads[i], NULL, thread_function, &thread_ids[i]);
  }
  for (i = 0; i < numThreads; i++) {
    pthread_join(threads[thread_ids[i]], NULL);
  }
  fin = clock();
  tiempo_transcurrido = (double)(fin - inicio) / (CLOCKS_PER_SEC * 10.0);
  printf("Counter value: %d\n", get(c));
  printf("Tiempo transcurrido: %f segundos\n", tiempo_transcurrido);
  return 0;
}