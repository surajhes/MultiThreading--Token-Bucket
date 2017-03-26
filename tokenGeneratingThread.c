//
// Created by surajhs04 on 9/18/16.
//
#include <time.h>
#include <sys/types.h>
#include <pthread.h>
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include "definitions.h"

int tokenBucketCount = 0;

int stopService;

int stopToken = 0;

long double simulationStartTime;

My402List queue1;

My402List queue2;

struct Statistics statistics;

long double getCurrentTimeMS();

void *generateToken(void *arg) {
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigprocmask(SIG_BLOCK, &set, 0);
    struct emulationParameters *parameters = (struct emulationParameters *) arg;
    int bucketSize = parameters->B;
    long double tokenThreadWakeUpTime = 0.0, tokenThreadServiceTime = 0, currentTimeInMilliSeconds;
    long double actualRate = (1 / parameters->r) * 1000; //in ms
    if ((actualRate / 1000) > 10) {
        actualRate = 10000.0;
    }
    long double sleepTime = actualRate - (tokenThreadServiceTime - tokenThreadWakeUpTime); //in ms
    long double delta = 0;
    long double interTokenArrivalRate = getCurrentTimeMS();
    int tokenCount = 0;
    while (1) {
        if (sleepTime > 0)
            usleep((useconds_t) (sleepTime * 1000));
        tokenThreadWakeUpTime = getCurrentTimeMS();
        interTokenArrivalRate = tokenThreadWakeUpTime - interTokenArrivalRate;
        pthread_mutex_lock(&mutex);
        if (My402ListEmpty(&queue1) && stopToken) {
            stopService = 1;
            pthread_cond_broadcast(&queueNotEmpty);
            pthread_mutex_unlock(&mutex);
            break;
        }
        tokenCount++;
        tokenBucketCount++;
        statistics.totalTokensGenerated++;
        if (tokenBucketCount < 2) {
            if (tokenBucketCount > bucketSize) {
                statistics.totalDroppedTokens++;
                fprintf(stdout, "\n%012.3Lfms: token t%d arrives,dropped",
                        getCurrentTimeMS() - simulationStartTime,
                        tokenCount);
                tokenBucketCount--;
            }
            fprintf(stdout, "\n%012.3Lfms: token t%d arrives,token bucket now has %d token",
                    getCurrentTimeMS() - simulationStartTime,
                    tokenCount, tokenBucketCount);
        } else {
            if (tokenBucketCount > bucketSize) {
                statistics.totalDroppedTokens++;
                fprintf(stdout, "\n%012.3Lfms: token t%d arrives,dropped",
                        getCurrentTimeMS() - simulationStartTime,
                        tokenCount);
                tokenBucketCount--;
            } else {
                fprintf(stdout, "\n%012.3Lfms: token t%d arrives,token bucket now has %d tokens",
                        getCurrentTimeMS() - simulationStartTime,
                        tokenCount, tokenBucketCount);
            }
        }
        if (!My402ListEmpty(&queue1)) {
            My402ListElem *firstPacket = My402ListFirst(&queue1);
            if (((struct PacketData *) firstPacket->obj)->numberOfTokens <= tokenBucketCount) {
                tokenBucketCount = tokenBucketCount - ((struct PacketData *) firstPacket->obj)->numberOfTokens;
                //if (My402ListEmpty(&queue2)) {
                struct PacketData *firstPacketForQ2;
                firstPacketForQ2 = ((struct PacketData *) firstPacket->obj);
                My402ListUnlink(&queue1, firstPacket);
                firstPacketForQ2->packetLeavesQ1 = getCurrentTimeMS();
                long double timeInQ1 = firstPacketForQ2->packetLeavesQ1 - firstPacketForQ2->packetEntersQ1;
                statistics.totalTimeInQ1 = statistics.totalTimeInQ1 + timeInQ1;
                fprintf(stdout, "\n%012.3Lfms: p%d leaves q1,time in q1 = %.3Lfms",
                        firstPacketForQ2->packetLeavesQ1 - simulationStartTime,
                        firstPacketForQ2->packetID, timeInQ1);
                if (My402ListEmpty(&queue2)) {
                    My402ListAppend(&queue2, firstPacketForQ2);
                    currentTimeInMilliSeconds = getCurrentTimeMS();
                    My402ListElem *firstPacketInQ2 = My402ListFirst(&queue2);
                    ((struct PacketData *) firstPacketInQ2->obj)->packetEntersQ2 = currentTimeInMilliSeconds;
                    fprintf(stdout, "\n%012.3Lfms: p%d enters q2",
                            ((struct PacketData *) firstPacketInQ2->obj)->packetEntersQ2 - simulationStartTime,
                            ((struct PacketData *) firstPacketInQ2->obj)->packetID);
                    pthread_cond_broadcast(&queueNotEmpty);
                } else {
                    My402ListAppend(&queue2, firstPacketForQ2);
                    currentTimeInMilliSeconds = getCurrentTimeMS();
                    firstPacketForQ2->packetEntersQ2 = currentTimeInMilliSeconds;
                    fprintf(stdout, "\n%012.3Lfms: p%d enters q2",
                            firstPacketForQ2->packetEntersQ2 - simulationStartTime,
                            firstPacketForQ2->packetID);
                }

            }
        }
        tokenThreadServiceTime = getCurrentTimeMS();
        delta = tokenThreadServiceTime - tokenThreadWakeUpTime;
        sleepTime = actualRate - delta;
        pthread_cond_broadcast(&queueNotEmpty);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

long double getCurrentTimeMS() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    long double milliSeconds = (tv).tv_sec * 1000.0 + (tv).tv_usec / 1000.0;
    return milliSeconds;
}
