//
// Created by surajhs04 on 9/16/16.
//

#include "my402list.h"

#ifndef WARMUP2_DEFINITIONS_H
#define WARMUP2_DEFINITIONS_H

#define DEFAULT_LAMBDA 1
#define DEFAULT_MU 0.35
#define DEFAULT_SERVER_RATE 1.5
#define DEFAULT_BUCKET_SIZE 10
#define DEFAULT_PACKETS_REQUIRED 3
#define DEFAULT_NUM 20
#define DEFAULT_FiLE_PATH ""
#define TRUE 1
#define FALSE 0

struct PacketData {
    int packetID;
    long double packetCreationTime;
    double mu;
    int numberOfTokens;
    long double packetEntersQ1;
    long double packetEntersQ2;
    long double packetLeavesQ1;
    long double packetLeavesQ2;
    long double packetEntersS;
    long double packetLeavesS;
};

struct PacketData *packet;

typedef struct emulationParameters {
    double lambda;
    double mu;
    double r;
    int B;
    int P;
    int n;
    char *tFilePath;
    int numberOfEntries;
} emulation;

struct Statistics {
    long double totalInterArrivalTime;
    int totalPacketsArrivedInQ1;
    int totalPacketsArrivedInQ2;
    long double totalServiceTime;
    int totalServicedPackets;
    int totalDroppedTokens;
    int totalTokensGenerated;
    int totalDroppedPackets;
    int totalPacketsGenerated;
    long double totalTimeInSystem;
    long double totalSquareTimeSpentInSystem;
    long double totalTimeInQ1;
    long double totalTimeInQ2;
    long double totalTimeInS1;
    long double totalTimeInS2;
};

struct Statistics statistics;

extern pthread_mutex_t mutex;

extern pthread_cond_t queueNotEmpty;

void *processPackets(void *arg);

void *createPacket(void *arg);

void *generateToken(void *arg);

void *handler(void *arg);

extern int tokenBucketCount;

extern int packetCount;

extern int tFilePacketCount;

extern int servicedPacketCount;

extern int stopService;

extern long double simulationStartTime;

extern int stopToken;

extern My402List queue1;

extern My402List queue2;

extern int totalPacketToCreate;

extern pthread_t server1, server2, packetCreation, tokenGeneration, signalHandler;

extern sigset_t set;

#endif //WARMUP2_DEFINITIONS_H
