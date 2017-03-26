//
// Created by surajhs04 on 9/13/16.
//
#include <sys/types.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include "definitions.h"

extern pthread_mutex_t mutex;

extern pthread_cond_t queueNotEmpty;

extern My402List queue2;

int packetCount;

int servicedPacketCount = 0;


int stopService;

struct Statistics statistics;

long double simulationStartTime;

long double getCurrentTimeInMSInServer();

extern int tFileUsed;

extern int tFilePacketCount;

void *processPackets(void *arg) {
    int id = (int) arg;
    stopService = 0;
    while (1) {
        pthread_mutex_lock(&mutex);
        while (My402ListEmpty(&queue2) && !stopService) {
            pthread_cond_wait(&queueNotEmpty, &mutex);
        }
        if (stopService && My402ListEmpty(&queue2)) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        if (!My402ListEmpty(&queue2)) {
            My402ListElem *my402ListElem = My402ListFirst(&queue2);
            struct PacketData *packetData = ((struct PacketData *) (my402ListElem->obj));
            My402ListUnlink(&queue2, my402ListElem);
            long double currentTimeInMilliSeconds = getCurrentTimeInMSInServer();
            packetData->packetLeavesQ2 = currentTimeInMilliSeconds;
            long double timeInQ2 = packetData->packetLeavesQ2 - packetData->packetEntersQ2;
            statistics.totalTimeInQ2 = statistics.totalTimeInQ2 + timeInQ2;
            fprintf(stdout, "\n%012.3Lfms: p%d leaves Q2. time in Q2 = %.3Lfms",
                    packetData->packetEntersQ2 - simulationStartTime,
                    packetData->packetID, (packetData->packetLeavesQ2 - packetData->packetEntersQ2));

            pthread_cond_signal(&queueNotEmpty);
            pthread_mutex_unlock(&mutex);
            double serviceTime;
            if (tFileUsed) {
                serviceTime = (packetData->mu);
            } else {
                serviceTime = (1 / packetData->mu);
                if (serviceTime > 10) {
                    serviceTime = 10000.0;
                }else{
                    serviceTime = serviceTime * 1000;
                }
            }
            packetData->packetEntersS = getCurrentTimeInMSInServer();
            fprintf(stdout, "\n%012.3Lfms: p%d begins service at S%d,requesting %.3fms of service",
                    packetData->packetEntersS - simulationStartTime, packetData->packetID, id,
                    serviceTime);
            usleep((useconds_t) (serviceTime) * 1000);
            packetData->packetLeavesS = getCurrentTimeInMSInServer();
            statistics.totalServiceTime =
                    statistics.totalServiceTime + (packetData->packetLeavesS - packetData->packetEntersS);
            statistics.totalServicedPackets++;
            long double timeInSystem = (packetData->packetLeavesS - packetData->packetCreationTime);
            if (id == 1) {
                statistics.totalTimeInS1 =
                        statistics.totalTimeInS1 + (packetData->packetLeavesS - packetData->packetEntersS);
            } else {
                statistics.totalTimeInS2 =
                        statistics.totalTimeInS2 + (packetData->packetLeavesS - packetData->packetEntersS);
            }
            statistics.totalTimeInSystem = statistics.totalTimeInSystem + timeInSystem;
            statistics.totalSquareTimeSpentInSystem =
                    statistics.totalSquareTimeSpentInSystem + timeInSystem * timeInSystem;
            fprintf(stdout, "\n%012.3Lfms: p%d departs from S%d, service time = %.3Lfms,time in system = %.3Lfms",
                    packetData->packetLeavesS - simulationStartTime,
                    packetData->packetID,
                    id,
                    (packetData->packetLeavesS - packetData->packetEntersS),
                    (packetData->packetLeavesS - packetData->packetCreationTime));
            servicedPacketCount++;
            if (tFileUsed) {
                if (servicedPacketCount == (tFilePacketCount - statistics.totalDroppedPackets)) {
                    if (My402ListEmpty(&queue1) && My402ListEmpty(&queue2)) {
                        stopService = 1;
                        break;
                    }
                }
            }
        }
    }
    return NULL;
}

long double getCurrentTimeInMSInServer() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    long double milliSeconds = (tv).tv_sec * 1000.0 + (tv).tv_usec / 1000.0;
    return milliSeconds;
}

