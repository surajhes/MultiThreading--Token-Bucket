//
// Created by surajhs04 on 9/16/16.
//
#include <sys/types.h>
#include <pthread.h>
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <memory.h>
#include <ctype.h>
#include <sys/stat.h>
#include "definitions.h"
#include <errno.h>
#include <string.h>

#define MAX_INT_VALUE 2147483647
#define NEGATIVE_NUMBER_ERROR 1
#define NOT_A_NUMBER_ERROR 2
#define OUT_OF_BOUND_POSITIVE_NUMBER_ERROR 3

int isValidFilePath(char *filePath);

int checkIfValidInt(char *data);

void initializeDefaultParams(emulation *parameters);

emulation *readEmulationParametersFromFile(struct emulationParameters *pParameters, FILE *pFILE);

void tokenizeInputLine(char *line, int lineNumber, emulation *traceParameters);

void splitInputLine(int lineNumber, emulation *traceParameters, int tokenCount, char *delim, char *startPtr);

extern pthread_mutex_t mutex;

extern pthread_cond_t queueNotEmpty;

long double getCurrentTimeInMilliSeconds();

char *getFileNameAfterCheckingForValidity(const struct emulationParameters *parameters);

void getNumberOfEntriesFromFile(FILE *filePointer);

struct PacketData *packet;

long double simulationStartTime;

My402List queue1;

int tokenBucketCount;

My402List queue2;

int packetCount;

int totalPacketToCreate = 0;

struct Statistics statistics;

extern int tFileUsed;

void getNumberOfEntriesFromFile(FILE *filePointer);

emulation *readEmulationParametersFromFile(struct emulationParameters *pParameters, FILE *pFILE) {
    char line[1024];
    memset(line, 0, sizeof(line));
    fgets(line, sizeof(line), pFILE);
    line[strlen(line) - 1] = '\0';
    emulation *traceParameters = malloc(sizeof(emulation));
    initializeDefaultParams(traceParameters);
    tokenizeInputLine(line, 0, traceParameters);
    traceParameters->B = pParameters->B;
    traceParameters->r = pParameters->r;
    traceParameters->tFilePath = pParameters->tFilePath;
    return traceParameters;
}

/*
 * Prints the usage of the application.
 */
void use() {
    fprintf(stderr, "\nUsage : warmup2 [-lambda lambda] [-mu mu] [-r r] [-B b] [-P p] [-n num] [-t tsfile]");
    exit(1);
}

void *createPacket(void *arg) {
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigprocmask(SIG_BLOCK, &set, 0);
    FILE *filePointer = NULL;;
    struct emulationParameters *parameters = (struct emulationParameters *) arg;
    if (tFileUsed/**(parameters->tFilePath) != '\0'*/) {
        //tFileUsed = 1;
        char *filePath = getFileNameAfterCheckingForValidity(parameters);
        filePointer = fopen(filePath, "r");
        if (!filePointer) {
            fprintf(stderr, "\n ERROR : Couldn't open the file successfully due to \n %s %s", parameters->tFilePath,
                    strerror(errno));
            use();
        }
        getNumberOfEntriesFromFile(filePointer);
        parameters->n = tFilePacketCount;
        simulationStartTime = getCurrentTimeInMilliSeconds();
        fprintf(stdout, "\nEmulation Parameters:");
        fprintf(stdout, "\nnumber to arrive %d\nr = %f\nB = %d", parameters[0].n,
                parameters[0].r,
                parameters[0].B);
        fprintf(stdout, "\ntsfile = %s", parameters->tFilePath);
        fprintf(stdout, "\n\n%012.3Lfms: Emulation begins",simulationStartTime - simulationStartTime);
    }
    long double threadWokeUpTime = 0.0, threadServiceTime = 0, currentTimeInMilliSeconds;
    long double actualRate = (1 / parameters->lambda) * 1000; //in ms
    long double sleepTime = actualRate - (threadServiceTime - threadWokeUpTime); //in ms
    long double delta = 0;
    long double interArrivalTime = getCurrentTimeInMilliSeconds();
    int i = 0;
    if (tFileUsed) {
        totalPacketToCreate = tFilePacketCount;
    } else {
        totalPacketToCreate = parameters->n;
    }
    packetCount = 0;

    for (i = 0; i < totalPacketToCreate; i++) {
        if (tFileUsed) {
            parameters = readEmulationParametersFromFile(parameters, filePointer);
            actualRate = parameters->lambda;
            sleepTime = actualRate - (threadServiceTime - threadWokeUpTime); //in ms
        } else {
            if ((actualRate / 1000) > 10) {
                actualRate = 10000.0;
                sleepTime = actualRate - (threadServiceTime - threadWokeUpTime); //in ms
            }
        }
        packet = malloc(sizeof(struct PacketData));
        if (sleepTime > 0)
            usleep((useconds_t) (sleepTime * 1000));
        packet->mu = parameters->mu;
        packet->numberOfTokens = parameters->P;
        threadWokeUpTime = getCurrentTimeInMilliSeconds();
        packet->packetCreationTime = threadWokeUpTime;
        interArrivalTime = threadWokeUpTime - interArrivalTime;
        pthread_mutex_lock(&mutex);
        packetCount++;
        packet->packetID = packetCount;
        statistics.totalPacketsGenerated++;
        statistics.totalInterArrivalTime = statistics.totalInterArrivalTime + interArrivalTime;
        if (packet->numberOfTokens > parameters->B) {
            statistics.totalDroppedPackets++;
            fprintf(stdout, "\n%012.3Lfms: p%d arrives,needs %d tokens,inter-arrival time = %.3Lfms, dropped",
                    packet->packetCreationTime - simulationStartTime,
                    packet->packetID,
                    packet->numberOfTokens, interArrivalTime);
            if (statistics.totalDroppedPackets == totalPacketToCreate) {
                stopToken = 1;
                pthread_cond_broadcast(&queueNotEmpty);
                pthread_mutex_unlock(&mutex);
                break;
            }
        } else {
            fprintf(stdout, "\n%012.3Lfms: p%d arrives,needs %d tokens,inter-arrival time = %.3Lfms",
                    packet->packetCreationTime - simulationStartTime,
                    packet->packetID,
                    packet->numberOfTokens, interArrivalTime);
            if (My402ListEmpty(&queue1)) {
                My402ListAppend(&queue1, (void *) packet);
                currentTimeInMilliSeconds = getCurrentTimeInMilliSeconds();
                statistics.totalPacketsArrivedInQ1++;
                packet->packetEntersQ1 = currentTimeInMilliSeconds;
                fprintf(stdout, "\n%012.3Lfms: p%d enters q1", packet->packetEntersQ1 - simulationStartTime,
                        packet->packetID);
                if (packet->numberOfTokens <= tokenBucketCount) {
                    My402ListElem *firstElemFromQ1 = My402ListFirst(&queue1);
                    struct PacketData *firstPacketForQ2;
                    firstPacketForQ2 = ((struct PacketData *) firstElemFromQ1->obj);
                    tokenBucketCount = tokenBucketCount - ((struct PacketData *) firstElemFromQ1->obj)->numberOfTokens;
                    My402ListUnlink(&queue1, firstElemFromQ1);
                    currentTimeInMilliSeconds = getCurrentTimeInMilliSeconds();
                    packet->packetLeavesQ1 = currentTimeInMilliSeconds;
                    long double timeInQ1 = packet->packetLeavesQ1 - packet->packetEntersQ1;
                    statistics.totalTimeInQ1 = statistics.totalTimeInQ1 + timeInQ1;
                    fprintf(stdout, "\n%012.3Lfms: p%d leaves q1,time in q1 = %.3Lfms",
                            packet->packetLeavesQ1 - simulationStartTime,
                            packet->packetID,
                            (packet->packetLeavesQ1 - packet->packetEntersQ1));
                    if (My402ListEmpty(&queue2)) {
                        My402ListAppend(&queue2, firstPacketForQ2);
                        statistics.totalPacketsArrivedInQ2++;
                        currentTimeInMilliSeconds = getCurrentTimeInMilliSeconds();
                        firstPacketForQ2->packetEntersQ2 = currentTimeInMilliSeconds;
                        fprintf(stdout, "\n%012.3Lfms: p%d enters q2",
                                firstPacketForQ2->packetEntersQ2 - simulationStartTime,
                                packet->packetID);
                        pthread_cond_broadcast(&queueNotEmpty);
                    } else {
                        My402ListAppend(&queue2, firstPacketForQ2);
                        statistics.totalPacketsArrivedInQ2++;
                        currentTimeInMilliSeconds = getCurrentTimeInMilliSeconds();
                        firstPacketForQ2->packetEntersQ2 = currentTimeInMilliSeconds;
                        fprintf(stdout, "\n%012.3Lfms: p%d enters q2",
                                firstPacketForQ2->packetEntersQ2 - simulationStartTime,
                                packet->packetID);
                    }
                }
            } else {
                My402ListAppend(&queue1, (void *) packet);
                currentTimeInMilliSeconds = getCurrentTimeInMilliSeconds();
                packet->packetEntersQ1 = currentTimeInMilliSeconds;
                fprintf(stdout, "\n%012.3Lfms: p%d enters q1", packet->packetEntersQ1 - simulationStartTime,
                        packet->packetID);
            }
        }
        threadServiceTime = getCurrentTimeInMilliSeconds();
        interArrivalTime = packet->packetCreationTime;
        delta = threadServiceTime - threadWokeUpTime;
        sleepTime = actualRate - delta;
        pthread_cond_broadcast(&queueNotEmpty);
        pthread_mutex_unlock(&mutex);
        if (tFileUsed) {
            //parameters++;
        }
    }
    if(tFileUsed){
        if(filePointer)
            fclose(filePointer);
    }
    stopToken = 1;
    return NULL;
}

void getNumberOfEntriesFromFile(FILE *filePointer) {
    char line[1024];
    memset(line, 0, sizeof(line));
    fgets(line, sizeof(line), filePointer);
    line[strlen(line) - 1] = '\0';
    if (checkIfValidInt(line)) {
            fprintf(stderr,
                    "\nERROR : Invalid format of trace file. Encountered a non integer value in the following line \n %s",
                    line);
            use();
        }
    tFilePacketCount = atoi(line);
}

char *getFileNameAfterCheckingForValidity(const struct emulationParameters *parameters) {
    char *filePath = malloc(sizeof(char) * (strlen(parameters->tFilePath) + 1));
    strcpy(filePath, parameters->tFilePath);
    if (access(filePath, R_OK) == -1) {
            // user doesnt have permissions to read
            fprintf(stderr, "\nERROR : User doesnt have the permission to read the specified file. (%s)", filePath);
            //usage();
            //exit(1);
        }
    if (access(filePath, F_OK) == -1) {
            // file doesn't exist
            fprintf(stderr, "\nERROR : The file specified  (%s)   doesnt exist.", filePath);
            //usage();
            //exit(1);
        }
    return filePath;
}

long double getCurrentTimeInMilliSeconds() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    long double milliSeconds = (tv).tv_sec * 1000.0 + (tv).tv_usec / 1000.0;
    return milliSeconds;
}


//added functions from main
void tokenizeInputLine(char *line, int lineNumber, emulation *traceParameters) {
    int tokenCount = 0;
    char delim[] = {' ', '\t'};
    char *startPtr = line;
    splitInputLine(lineNumber, traceParameters, tokenCount, delim, startPtr);
}

void splitInputLine(int lineNumber, emulation *traceParameters, int tokenCount, char *delim, char *startPtr) {
    while (startPtr != 0) {
        if (*(startPtr + strlen(startPtr) - 1) == '\n') {
            *(startPtr + strlen(startPtr) - 1) = '\0';
        }
        char *tabPtr = strchr(startPtr, *delim);
        if (tabPtr != NULL) {
            *tabPtr++ = '\0';
            while (isspace(*tabPtr)) {
                tabPtr++;
            }
        }
        switch (tokenCount) {
            case 0 :
                if (checkIfValidInt(startPtr)) {
                    fprintf(stderr, "\n ERROR : Found an invalid parameter for -lambda param @ Line : %d \n%s",
                            lineNumber + 1, startPtr);
                    exit(1);
                }
                traceParameters->lambda = atoi(startPtr);
                break;
            case 1 :
                if (checkIfValidInt(startPtr)) {
                    fprintf(stderr, "\n ERROR : Found an invalid parameter for -P param @ Line : %d \n%s",
                            lineNumber + 1, startPtr);
                    exit(1);
                }
                char *end = NULL;
                int value = strtol(startPtr, &end, 10);
                traceParameters->P = value;
                break;
            case 2 :
                if (*(startPtr + strlen(startPtr) - 1) == '\n') {
                    *(startPtr + strlen(startPtr) - 1) = '\0';
                }
                if (checkIfValidInt(startPtr)) {
                    fprintf(stderr, "\n ERROR : Found an invalid parameter -mu param @ Line : %d \n%s",
                            lineNumber + 1, startPtr);
                    exit(1);
                }
                traceParameters->mu = atoi(startPtr);
                break;
            default:
                fprintf(stderr, "\n ERROR : Invalid number of tokens in the trace file.");
                exit(1);
        }
        tokenCount++;
        startPtr = tabPtr;
    }
}


/*
 * Checks if its a directory or not and is a valid file path.
 */
int isValidFilePath(char *filePath) {
    struct stat stat1;
    stat(filePath, &stat1);
    if (S_ISDIR(stat1.st_mode)) {
        fprintf(stderr, "\n ERROR: %s is a directory", filePath);
        //usage();
        exit(1);
    }
    return TRUE;
}

int checkIfInValidDouble(char *data) {
    char *end = NULL;
    int inValid = 0;
    strtod(data, &end);
    if (data != end && *end != '\0') {
        inValid = 1;
        return inValid;
    }
    return inValid;
}

/*
 * Checks if whether the parameter following the option is a valid integer or not.
 */
int checkIfValidInt(char *data) {
    char *end = NULL;
    long value = strtol(data, &end, 10);
    if (*data != '\0' && *end == '\0') {
        if (value < 0) {
            return NEGATIVE_NUMBER_ERROR;
        }
        if (value == MAX_INT_VALUE && strcmp("2147483647", data) != 0) {
            return OUT_OF_BOUND_POSITIVE_NUMBER_ERROR;
        }
    } else {
        return NOT_A_NUMBER_ERROR;
    }
    return 0;
}

/*
void usage() {
    fprintf(stderr, "\nUsage : warmup2 [-lambda lambda] [-mu mu] [-r r] [-B b] [-P p] [-n num] [-t tsfile]");
    exit(1);
}
*/

void initializeDefaultParams(emulation *parameters) {
    parameters->lambda = DEFAULT_LAMBDA;
    parameters->mu = DEFAULT_MU;
    parameters->r = DEFAULT_SERVER_RATE;
    parameters->B = DEFAULT_BUCKET_SIZE;
    parameters->P = DEFAULT_PACKETS_REQUIRED;
    parameters->n = DEFAULT_NUM;
    parameters->tFilePath = DEFAULT_FiLE_PATH;
    return;
}
