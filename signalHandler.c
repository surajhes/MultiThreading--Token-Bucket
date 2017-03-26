//
// Created by surajhs04 on 9/22/16.
//
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include "definitions.h"

void performCleanUp();

long double getCTime();

void *handler(void *arg) {
    sigwait(&set,arg);
    pthread_mutex_lock(&mutex);
    performCleanUp();
    pthread_mutex_unlock(&mutex);
    return 0;
}

void emptyQueuesAndDisplay() {
    My402ListElem *my402ListElem = NULL;
    for (my402ListElem = My402ListFirst(&queue1); my402ListElem != NULL;
         my402ListElem = My402ListNext(&queue1, my402ListElem)) {
        fprintf(stdout, "\n%012.3Lfms: p%d removed from Q1", getCTime() - simulationStartTime,
                ((struct PacketData *) (my402ListElem->obj))->packetID);
        My402ListUnlink(&queue1, my402ListElem);
    }
    for (my402ListElem = My402ListFirst(&queue2); my402ListElem != NULL;
         my402ListElem = My402ListNext(&queue2, my402ListElem)) {
        fprintf(stdout, "\n%012.3Lfms: p%d removed from Q2", getCTime() - simulationStartTime,
                ((struct PacketData *) (my402ListElem->obj))->packetID);
        long double timeInQ1 = (((struct PacketData *) (my402ListElem->obj))->packetLeavesQ1 -
                                ((struct PacketData *) (my402ListElem->obj))->packetEntersQ1);
        statistics.totalTimeInQ1 = statistics.totalTimeInQ1 - timeInQ1;
        statistics.totalPacketsArrivedInQ1 =  statistics.totalPacketsArrivedInQ1 - 1;
        My402ListUnlink(&queue2, my402ListElem);
    }
    return;
}

void performCleanUp() {
    pthread_cancel(packetCreation);
    pthread_cancel(tokenGeneration);
    emptyQueuesAndDisplay();
    stopService = 1;
    pthread_cond_broadcast(&queueNotEmpty);
}

long double getCTime() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    long double milliSeconds = (tv).tv_sec * 1000.0 + (tv).tv_usec / 1000.0;
    return milliSeconds;
}

