#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <ctype.h>
#include <pthread.h>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include "definitions.h"


#define MAX_INT_VALUE 2147483647

#define NEGATIVE_NUMBER_ERROR 1
#define NOT_A_NUMBER_ERROR 2
#define OUT_OF_BOUND_POSITIVE_NUMBER_ERROR 3

const char *lambda = "-lambda";
const char *mu = "-mu";
const char *r = "-r";
const char *B = "-B";
const char *P = "-P";
const char *n = "-n";
const char *t = "-t";

void checkInputParameters(int argc, char *argv[], emulation *pInputCommand);

int checkIfInValidInt(char *data);

int checkIfInValidDouble(char *data);

void usage();

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t queueNotEmpty = PTHREAD_COND_INITIALIZER;

void calculateStatistics();

long double getCurrentTime();

sigset_t set;

My402List queue1;

My402List queue2;

emulation *parameters;

int checkDoubleValue(char *data);

long double simulationTime;

long double simulationStartTime;

int tFileUsed = 0;

int tFilePacketCount = 0;

pthread_t server1, server2, packetCreation, tokenGeneration, signalHandler;

void initializeDefault(emulation *parameters) {
    parameters->lambda = DEFAULT_LAMBDA;
    parameters->mu = DEFAULT_MU;
    parameters->r = DEFAULT_SERVER_RATE;
    parameters->B = DEFAULT_BUCKET_SIZE;
    parameters->P = DEFAULT_PACKETS_REQUIRED;
    parameters->n = DEFAULT_NUM;
    parameters->tFilePath = DEFAULT_FiLE_PATH;
    return;
}


int main(int argc, char *argv[]) {
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigprocmask(SIG_BLOCK, &set, 0);
    My402ListInit(&queue1);
    My402ListInit(&queue2);
    parameters = malloc(sizeof(emulation));
    initializeDefault(parameters);
    checkInputParameters(argc, argv, parameters);
    char *tFile = malloc(sizeof(strlen(parameters->tFilePath)));
    strcpy(tFile, parameters->tFilePath);
    if(*(parameters->tFilePath) == '\0') {
        parameters->numberOfEntries = 1;
        fprintf(stdout, "\nEmulation Parameters:");
        fprintf(stdout,
                "\n\tnumber to arrive = %d \n\tlambda = %f \n\tmu = %f \n\tr = %f \n\tB = %d \n\tP = %d",
                parameters->n, parameters->lambda, parameters->mu, parameters->r, parameters->B, parameters->P);
    }
    if(!tFileUsed){
        simulationStartTime = getCurrentTime();
        fprintf(stdout, "\n\n%012.3Lfms: Emulation begins", simulationStartTime - simulationStartTime);
    }
    pthread_create(&packetCreation, 0, createPacket, (void *) parameters);
    pthread_create(&tokenGeneration, 0, generateToken, (void *) parameters);
    pthread_create(&server1, 0, processPackets, (void *) 1);
    pthread_create(&server2, 0, processPackets, (void *) 2);
    pthread_create(&signalHandler, 0, handler, 0);
    pthread_join(packetCreation, NULL);
    pthread_join(tokenGeneration, NULL);
    pthread_join(server1, NULL);
    pthread_join(server2, NULL);
    simulationTime = getCurrentTime() - simulationStartTime;
    fprintf(stdout, "\n%012.3Lfms: Emulation ends", simulationTime);
    calculateStatistics();
    return 0;
}

long double getCurrentTime() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    long double milliSeconds = (tv).tv_sec * 1000.0 + (tv).tv_usec / 1000.0;
    return milliSeconds;
}

void calculateStatistics() {
    fprintf(stdout, "\n\n");
    fprintf(stdout, "\nStatistics:");
    if (statistics.totalPacketsGenerated == 0) {
        fprintf(stdout, "\n\taverage packet inter-arrival time couldnt be calculated as no packets were generated.");
    } else {
        fprintf(stdout, "\n\taverage packet inter-arrival time = %.6gs",
                (double)(statistics.totalInterArrivalTime / statistics.totalPacketsGenerated)/1000);
    }
    if (statistics.totalServicedPackets == 0) {
        fprintf(stdout, "\n\taverage packet service time couldnt be calculated as no packets were serviced.");
    } else {
        fprintf(stdout, "\n\taverage packet service time = %.6gs",
                (double)(statistics.totalServiceTime / statistics.totalServicedPackets)/1000);
    }
    fprintf(stdout, "\n");
    fprintf(stdout, "\n\taverage number of packets in Q1 = %.6g", (double)(statistics.totalTimeInQ1 / simulationTime));
    fprintf(stdout, "\n\taverage number of packets in Q2 = %.6g", (double)(statistics.totalTimeInQ2 / simulationTime));
    fprintf(stdout, "\n\taverage number of packets at S1 = %.6g", (double)(statistics.totalTimeInS1 / simulationTime));
    fprintf(stdout, "\n\taverage number of packets at S2 = %.6g", (double)(statistics.totalTimeInS2 / simulationTime));
    fprintf(stdout, "\n");
    if (statistics.totalServicedPackets == 0) {
        fprintf(stdout, "\n\taverage time a packet spent in system couldnt be calculated as no packets were serviced.");
    } else {
        fprintf(stdout, "\n\taverage time a packet spent in system = %.6g",
                (double)((statistics.totalTimeInSystem / statistics.totalServicedPackets)/1000));
    }
    if (statistics.totalServicedPackets == 0) {
        fprintf(stdout,
                "\n\tstandard deviation for time packet in system couldnt be calculated as no packets were serviced.");
    } else {
        long double timeInSystemAvg = statistics.totalTimeInSystem / statistics.totalServicedPackets;
        timeInSystemAvg = timeInSystemAvg * timeInSystemAvg;
        double variance = (double) ((statistics.totalSquareTimeSpentInSystem / statistics.totalServicedPackets) -
                                    timeInSystemAvg);
        double standardDeviation = pow(variance, 0.5);
        fprintf(stdout, "\n\tstandard deviation for time packet in system = %.6g", (standardDeviation/1000));
    }
    fprintf(stdout, "\n");
    if (statistics.totalTokensGenerated == 0) {
        fprintf(stdout, "\n\ttoken drop probability couldnt be calculated as no tokens were generated.");
    } else {
        fprintf(stdout, "\n\ttoken drop probability = %.6g",
                (double) statistics.totalDroppedTokens / (double) statistics.totalTokensGenerated);
    }
    if (statistics.totalPacketsGenerated == 0) {
        fprintf(stdout, "\n\tpacket drop probability couldnt be calculated as no packets generated.\n");
    } else {
        fprintf(stdout, "\n\tpacket drop probability = %.6g\n",
                (double) statistics.totalDroppedPackets / (double) statistics.totalPacketsGenerated);
    }
}


/*
 * Pareses the command line arguments and fills the commands in an array.
 */
void checkInputParameters(int argc, char *argv[], emulation *pInputCommand) {
    int i = 1;
    while (i < argc) {
        if ((strcmp(argv[i], lambda) == 0)) {
            if (i + 1 == argc) {
                fprintf(stderr, "\nERROR : No value encountered for -lambda option.");
                usage();
            }
            if (checkIfInValidDouble(argv[i + 1])) {
                fprintf(stderr, "\nERROR : Encountered an illegal double value in lambda.");
                usage();
            }
            double lambdaValue = (double) 0;
            if (sscanf(argv[i + 1], "%lf", &lambdaValue) != 1) {
                /* cannot parse argv[ i + 1] to get a double value */
                fprintf(stderr, "\nERROR : Encountered a non Real Value for lambda parameter.");
                usage();
            } else {
                /* success */
                if (lambdaValue < 0) {
                    fprintf(stderr, "\nERROR : Encountered a negative value for lambda.");
                    usage();
                }
                pInputCommand->lambda = lambdaValue;
                i = i + 2;
            }
        } else if ((strcmp(argv[i], mu) == 0)) {
            if (i + 1 == argc) {
                fprintf(stderr, "\nERROR : No value encountered for -mu option.");
                usage();
            }
            if (checkIfInValidDouble(argv[i + 1])) {
                fprintf(stderr, "\nERROR : Encountered an illegal double value in mu value.");
                usage();
            }
            double muValue = (double) 0;
            if (sscanf(argv[i + 1], "%lf", &muValue) != 1) {
                /* cannot parse argv[ i + 1] to get a double value */
                fprintf(stderr, "\nERROR : Encountered a non Real Value for mu parameter.");
                usage();
            } else {
                /* success */
                if (mu < 0) {
                    fprintf(stderr, "\nERROR : Encountered a negative value for mu");
                    usage();
                }
                pInputCommand->mu = muValue;
                i = i + 2;
            }
        } else if ((strcmp(argv[i], r) == 0)) {
            if (i + 1 == argc) {
                fprintf(stderr, "\nERROR : No value encountered for -r option.");
                usage();
            }
            if (checkIfInValidDouble(argv[i + 1])) {
                fprintf(stderr, "\nERROR : Encountered an illegal double value in r option.");
                usage();
            }
            double rateValue = (double) 0;
            if (sscanf(argv[i + 1], "%lf", &rateValue) != 1) {
                /* cannot parse argv[ i + 1] to get a double value */
                fprintf(stderr, "\nERROR : Encountered a non Real Value for r parameter.");
                usage();
            } else {
                /* success */
                if (rateValue < 0) {
                    fprintf(stderr, "\n ERROR : Encountered a negative value for mu");
                    usage();
                }
                pInputCommand->r = rateValue;
                i = i + 2;
            }
        } else if ((strcmp(argv[i], B) == 0)) {
            if (i + 1 == argc) {
                fprintf(stderr, "\n ERROR : No value encountered for -B option.");
                usage();
            }
            int error = checkIfInValidInt(argv[i + 1]);
            if (error) {
                switch (error) {
                    case NEGATIVE_NUMBER_ERROR :
                        fprintf(stderr, "\nERROR : Encountered a negative integer.");
                        fprintf(stderr, "Invalid parameter for [-B]. %s", argv[i + 1]);
                        break;
                    case NOT_A_NUMBER_ERROR :
                        fprintf(stderr,
                                "\n ERROR : Encountered a non integer at a place where an integer was expected for -B token.");
                        fprintf(stderr, "Invalid parameter : %s", argv[i + 1]);
                        break;
                    case OUT_OF_BOUND_POSITIVE_NUMBER_ERROR :
                        fprintf(stderr, "\n ERROR : Encountered an out of bound positive integer.");
                        fprintf(stderr, "Invalid parameter for [-B ]. %s", argv[i + 1]);
                    default:
                        usage();
                }
                usage();
            } else {
                pInputCommand->B = atoi(argv[i + 1]);
                if (pInputCommand->B >= MAX_INT_VALUE && strcmp(argv[i + 1], "2147483647") != 0) {
                    fprintf(stderr, "\n ERROR : Encountered an out of bound positive integer.");
                    fprintf(stderr, "Invalid parameter for [-B ]. %s", argv[i + 1]);
                    usage();
                }
                i = i + 2;
            }
        } else if ((strcmp(argv[i], P) == 0)) {
            if (i + 1 == argc) {
                fprintf(stderr, "\n ERROR : No value encountered for -P option.");
                usage();
            }
            int error = checkIfInValidInt(argv[i + 1]);
            if (error) {
                switch (error) {
                    case NEGATIVE_NUMBER_ERROR :
                        fprintf(stderr, "\n ERROR : Encountered a negative integer.");
                        fprintf(stderr, "Invalid parameter for [-P ]. %s", argv[i + 1]);
                        break;
                    case NOT_A_NUMBER_ERROR :
                        fprintf(stderr,
                                "\n ERROR : Encountered a non integer at a place where an integer was expected for -P token.");
                        fprintf(stderr, "Invalid parameter : %s", argv[i + 1]);
                        break;
                    case OUT_OF_BOUND_POSITIVE_NUMBER_ERROR :
                        fprintf(stderr, "\n ERROR : Encountered an out of bound positive integer.");
                        fprintf(stderr, "Invalid parameter for [-P ]. %s", argv[i + 1]);
                    default:
                        usage();
                }
                usage();
            } else {
                pInputCommand->P = atoi(argv[i + 1]);
                i = i + 2;
            }
        } else if ((strcmp(argv[i], n) == 0)) {
            if (i + 1 == argc) {
                fprintf(stderr, "\n ERROR : No value encountered for -n option.");
                usage();
            }
            int error = checkIfInValidInt(argv[i + 1]);
            if (error) {
                switch (error) {
                    case NEGATIVE_NUMBER_ERROR :
                        fprintf(stderr, "\n ERROR : Encountered a negative integer.");
                        fprintf(stderr, "Invalid parameter for [-n ]. %s", argv[i + 1]);
                        break;
                    case NOT_A_NUMBER_ERROR :
                        fprintf(stderr,
                                "\n ERROR : Encountered a non integer at a place where an integer was expected for -n token.");
                        fprintf(stderr, "Invalid parameter : %s", argv[i + 1]);
                        break;
                    case OUT_OF_BOUND_POSITIVE_NUMBER_ERROR :
                        fprintf(stderr, "\n ERROR : Encountered an out of bound positive integer.");
                        fprintf(stderr, "Invalid parameter for [-n ]. %s", argv[i + 1]);
                    default:
                        usage();
                }
                usage();
            } else {
                pInputCommand->n = atoi(argv[i + 1]);
                i = i + 2;
            }
        } else if ((strcmp(argv[i], t) == 0)) {
            if (i + 1 == argc) {
                fprintf(stderr, "\n ERROR : No value encountered for -t option.");
                usage();
            }
            pInputCommand->tFilePath = argv[i + 1];
            i = i + 2;
            tFileUsed = 1;
        } else {
            fprintf(stderr, "\n ERROR : Encountered Illegal command line parameter.");
            usage();
            return;
        }
    }
}

/*
 * Checks if whether the parameter following the option is a valid integer or not.
 */
int checkIfInValidInt(char *data) {
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
 * Prints the usage of the application.
 */
void usage() {
    fprintf(stderr, "\nUsage : warmup2 [-lambda lambda] [-mu mu] [-r r] [-B b] [-P p] [-n num] [-t tsfile]");
    exit(1);
}
