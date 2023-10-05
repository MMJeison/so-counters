#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sched.h>
#include <time.h>

int nroIteraciones;

typedef struct __counter_t {
  int value;
  pthread_mutex_t lock;
} counter_t;

void init(counter_t *c) {
  c->value = 0;
  pthread_mutex_init(&c->lock, NULL);
}

void increment(counter_t *c) {
  pthread_mutex_lock(&c->lock);
  c->value++;
  pthread_mutex_unlock(&c->lock);
}

void decrement(counter_t *c) {
  pthread_mutex_lock(&c->lock);
  c->value--;
  pthread_mutex_unlock(&c->lock);
}

int get(counter_t *c) {
  pthread_mutex_lock(&c->lock);
  int rc = c->value;
  pthread_mutex_unlock(&c->lock);
  return rc;
}

counter_t counter;
void *thread_function(void *arg) {
  int thread_id = *((int *)arg);
  int cpu = sched_getcpu();
  printf("Hilo %d iniciado en el núcleo %d\n", thread_id, cpu);
  int i;
  for (i = 0; i < nroIteraciones; i++) {
    increment(&counter);
  }
  printf("Hilo %d terminado\n", thread_id);
  return NULL;
}

int main(int argc, char *argv[]) {
  init(&counter);
  int numThreads = 32;
  nroIteraciones = 2000000;
  if(argc > 3) {
    printf("Invalid arguments\n");
    return 1;
  }
  if (argc > 1) {
    nroIteraciones = atoi(argv[1]);
  }
  if (argc > 2) {
    numThreads = atoi(argv[2]);
  }
  pthread_t threads[numThreads];
  int trhead_ids[numThreads];

  clock_t inicio, fin;
  double tiempo_transcurrido;
  inicio = clock();
  int i;
  for (i = 0; i < numThreads; i++) {
    trhead_ids[i] = i;
    pthread_create(&threads[i], NULL, thread_function, &trhead_ids[i]);
  }
  for (i = 0; i < numThreads; i++) {
    pthread_join(threads[trhead_ids[i]], NULL);
  }
  fin = clock();
  tiempo_transcurrido = (double)(fin - inicio) / (CLOCKS_PER_SEC * 10.0);
  printf("Counter value: %d\n", get(&counter));
  printf("Tiempo transcurrido: %f segundos\n", tiempo_transcurrido);

  return 0;
}