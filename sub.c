#include "sub.h"

char key[20];
char val[20];

char NUMCLIENTSSTR[5];

char message[50];
struct sembuf semaphore;
static int sem_id;


int readcommand(char *input) {
    memset(&key[0], 0, sizeof(key));  //Key und Value werden geleert
    memset(&val[0], 0, sizeof(val));

    char delimiter[] = " ,:";
    char *ptr;

    ptr = strtok(input, delimiter);

    int counter = 0;
    while (ptr != NULL) {
        if (counter == 1) {
            strcpy(key, ptr);
        } else if (counter == 2) {
            strcpy(val, ptr);
        }
        counter++;

        ptr = strtok(NULL, delimiter);
    }

    key[strcspn(key, "\r\n")] = 0;
    val[strcspn(val, "\r\n")] = 0;

    return 0;

}

void sendMessage(int clientSock, char *message, size_t messageLength) {
    long err = send(clientSock, message, messageLength, 0);
    if (err < 0) printf("Konnte nicht zu Client senden\n");
}

int init_semaphore() {
    sem_id = semget(IPC_PRIVATE, 1, IPC_CREAT | IPC_EXCL | 0644);
    if (sem_id < 0) {
        perror("Creating semaphore failed");
        return -1;
    }
    printf("Semaphor-ID: %d initialisiert.\n", sem_id);
    fflush(stdout);
    if (semctl(sem_id, 0, SETVAL, (int) 1) == -1) {
        return -1;
    }
    return sem_id;
}


void startsocket() {

    init_semaphore();
    initialize();

    int shm_id, shm_id_transblock, shm_id_blockingclient, shm_id_notify, *shm_count, shm_count_id;
    struct keyval *shar_mem;

    shm_id = shmget(IPC_PRIVATE, SEGMENT_SIZE, IPC_CREAT | 0644);
    shm_id_transblock = shmget(IPC_PRIVATE, SEGMENT_SIZE, IPC_CREAT | 0644);
    shm_id_blockingclient = shmget(IPC_PRIVATE, SEGMENT_SIZE, IPC_CREAT | 0644);

    shar_mem = (struct keyval *) shmat(shm_id, 0, 0);
    shm_count_id = shmget(IPC_PRIVATE, SEGMENT_SIZE, IPC_CREAT | 0644);
    shm_count = (int *) shmat(shm_count_id, 0, 0);
    *shm_count = 0;

    int *transactionblock = (int *) shmat(shm_id_transblock, 0, 0);
    int *blockingclient = (int *) shmat(shm_id_blockingclient, 0, 0);

    int msid = msgget(1, IPC_CREAT | 0644);


    if (shm_id == -1) {
        perror("Shared Memory Segment konnte nicht angelegt werden!");
        exit(1);
    }


    int rfd;
    int cfd;

    struct sockaddr_in client;
    socklen_t client_len;
    char in[BUFSIZE];
    int bytes_read;


    // Socket erstellen
    rfd = socket(AF_INET, SOCK_STREAM, 0);
    if (rfd < 0) {
        fprintf(stderr, "socket konnte nicht erstellt werden\n");
        exit(-1);
    }

    // Socket Optionen setzen für schnelles wiederholtes Binden der Adresse
    int option = 1;
    setsockopt(rfd, SOL_SOCKET, SO_REUSEADDR, (const void *) &option, sizeof(int));

    printf("Socket erstellt.\n");
    printf("Starte Server auf Port %i\n", PORT);

    // Socket binden
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);
    int brt = bind(rfd, (struct sockaddr *) &server, sizeof(server));
    if (brt < 0) {
        fprintf(stderr, "socket konnte nicht gebunden werden\n");
        exit(-1);
    }

    // Socket lauschen lassen
    int lrt = listen(rfd, 5);
    if (lrt < 0) {
        fprintf(stderr, "socket konnte nicht listen gesetzt werden\n");
        exit(-1);
    }

    // Verbindung eines Clients wird entgegengenommen
    Listen:
    printf("Warte auf eingehende Verbindungen...\n");
    printf("Client %i\n", *shm_count);
    int c = sizeof(struct sockaddr_in);
    cfd = accept(rfd, (struct sockaddr *) &client,
                 (socklen_t *) &c);
    *shm_count = *shm_count + 1;


    int pid = fork();
    if (pid < 0) {
        *shm_count = *shm_count - 1;
        printf("Fork fehlgeschlagen. Konnte keinen neuen Prozess für den Client erstellen.");
        close(cfd);
        close(rfd);
    } else if (pid == 0) {

        if (cfd < 0) {
            perror("Konnte Client-Verbindung nicht akzeptieren.");
            exit(1);
        }

        /*------------Subscribed Keys-------------*/
        struct pubsubkey {
            char key[20];
        };

        struct pubsubkey subbedkeys[20];                                                                                    //Array mit Keys, die der Client abonniert hat

        for (int i = 0; i <
                        20; i++) {                                                                                      //subbedkeys wird mit Elementen aufgefüllt
            strcpy(subbedkeys[i].key, "*");
        }


        while (ENDLOSSCHLEIFE) {

            bzero(in, BUFSIZE);
            bytes_read = read(cfd, in, BUFSIZE);

            struct sembuf enter, leave;
            enter.sem_num = leave.sem_num = 0;
            enter.sem_flg = leave.sem_flg = SEM_UNDO;
            enter.sem_op = -1;
            leave.sem_op = 1;

            semop(sem_id, &enter, 1);

            if (bytes_read > 0) {

                if (*transactionblock == 0 ||
                    cfd == *blockingclient) {

                    if (strncmp("NUMCLIENTS", in, 10) == 0) {
                        sprintf(NUMCLIENTSSTR, "%i",
                                *shm_count);
                        memset(&message[0], 0,
                               sizeof(message));
                        strncat(message, NUMCLIENTSSTR,
                                strlen(NUMCLIENTSSTR));
                        strncat(message, "\n", 1);
                        sendMessage(cfd, message, strlen(message));

                    } else if (strncmp("QUIT", in, 4) == 0) {
                        *shm_count = *shm_count - 1;
                        printf("Socket wird geschlossen...\n");
                        sendMessage(cfd, "Socket wird geschlossen\n", 24);
                        shutdown(cfd, SHUT_RDWR);
                        close(cfd);
                        fflush(stdout);
                        exit(0);

                    } else if (strncmp("GET", in, 3) == 0) {
                        readcommand(in);
                        char *res[40];
                        memset(res[0], 0, sizeof(res));
                        printf("Return-Code: %i \n", get(key, *res));
                        printf("GET:%s:%s \n", key, *res);

                        memset(&message[0], 0, sizeof(message));
                        strncat(message, "GET:", 6);
                        strncat(message, key, 20);
                        strncat(message, ":", 1);
                        strncat(message, *res, 20);
                        strncat(message, "\n", 1);
                        sendMessage(cfd, message, strlen(message));


                    } else if (strncmp("PUT", in, 3) == 0) {
                        readcommand(in);
                        printf("%d\n", put(key, val));
                        printf("PUT:%s:%s\n", key, val);

                        memset(&message[0], 0, sizeof(message));
                        strncat(message, "PUT:", 6);
                        strncat(message, key, 30);
                        strncat(message, ":", 1);
                        strncat(message, val, 30);
                        strncat(message, "\n", 1);
                        sendMessage(cfd, message, strlen(message));

                        msgsnd(msid, message, strlen(message), 0);


                    } else if (strncmp("DEL", in, 3) == 0) {
                        readcommand(in);

                        memset(&message[0], 0, sizeof(message));
                        strncat(message, "DEL:", 6);
                        strncat(message, key, 20);
                        strncat(message, ":", 1);
                        if (del(key) == 0) {
                            strncat(message, "key_deleted", 11);
                            printf("Key %s Deleted\n", key);
                        } else {
                            strncat(message, "key_nonexistent", 16);
                        }

                        strncat(message, "\n", 1);
                        sendMessage(cfd, message, strlen(message));

                        msgsnd(msid, message, strlen(message), 0);

                    } else if (strncmp("SUB", in, 3) == 0) {
                        readcommand(in);

                        for (int i = 0; i < 20; i++) {
                            if ((strcmp(subbedkeys[i].key, "*") == 0)) {
                                strcpy(subbedkeys[i].key, key);
                                memset(&message[0], 0, sizeof(message));
                                strncat(message, "SUB:", 6);
                                strncat(message, key, 20);
                                strncat(message, "\n", 1);
                                sendMessage(cfd, message, strlen(message));
                                break;
                            }
                        }

                        char notification[50];
                        int subPid = fork();
                        if (subPid == 0) {
                            while (ENDLOSSCHLEIFE) {
                                int receive = msgrcv(msid, &notification, sizeof(notification), 0, IPC_NOWAIT);
                                char subcommand[50];
                                strcpy(subcommand, notification);
                                readcommand(notification);


                                if (receive >= 0) {
                                    for (int i = 0; i < 20; i++) {
                                        if (strcmp(subbedkeys[i].key, key) == 0) {
                                            sendMessage(cfd, subcommand, strlen(subcommand));
                                        }
                                    }

                                }
                            }
                        }

                    } else if (strncmp("BEG", in, 3) == 0) {
                        *transactionblock = 1;
                        *blockingclient = cfd;
                        sendMessage(cfd, "Begin Transaction\n", 18);
                    } else if (strncmp("END", in, 3) == 0) {
                        *transactionblock = 0;
                        *blockingclient = 0;
                        sendMessage(cfd, "Transaction ended\n", 18);
                    } else {
                        sendMessage(cfd, "Unknown Command\n", 16);
                    }
                } else {
                    sendMessage(cfd, "You are blocked right now\n", 26);
                }
            }

            semop(sem_id, &leave, 1); //KritischerBereich 1 Ende

        }
    } else {
        goto Listen;
    }
    shmdt(shm_count);
    shmdt(shar_mem);                                                                                                    //Shared Memory Segmente werden entkoppelt
    shmctl(shm_id, IPC_RMID, 0);
    shmctl(shm_id_transblock, IPC_RMID, 0);
    shmctl(shm_id_blockingclient, IPC_RMID, 0);
    semctl(sem_id, 0, IPC_RMID, 0);
    close(cfd);
    close(rfd);
    exit(0);
}




