#include "sub.h"

char *stoupper(char *str) {
    int i;
    char *upper;
    upper = malloc(strlen(str) + 1);
    for (i = 0; str[i] != '\0'; i++) {
        upper[i] = (char) toupper(str[i]);
    }
    return upper;
}

int readcommand(char *input, struct keyval *kvp) {
    memset(kvp->key, 0, sizeof(kvp->key));
    memset(kvp->val, 0, sizeof(kvp->val));

    char delimiter[] = " ,:";
    char *token;

    token = strtok(input, delimiter);

    int counter = 0;
    while (token != NULL) {
        if (counter == 1) {
            strcpy(kvp->key, token);
        } else if (counter == 2) {
            strcpy(kvp->val, token);
        } else if (counter > 2) {
            strcat(kvp->val, " ");
            strcat(kvp->val, token);
        }
        counter++;

        token = strtok(NULL, delimiter);
    }

    kvp->key[strcspn(kvp->key, "\r\n")] = 0;
    kvp->val[strcspn(kvp->val, "\r\n")] = 0;

    return 0;
}

void sendMessage(int clientSock, char *msg, size_t messageLength) {
    long err = send(clientSock, msg, messageLength, 0);
    if (err < 0) printf("Konnte nicht zu Client senden\n");
}

int init_semaphore() {
    int sem_id = semget(IPC_PRIVATE, 1, IPC_CREAT | IPC_EXCL | 0644);
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

    int sem_id = init_semaphore();
    initialize();

    int shm_id, shm_id_transblock, shm_id_blockingclient, shm_id_notify, *shm_count, shm_count_id;
    struct keyval *shar_mem;

    int *child_pids = malloc(sizeof(int) * MAX_CLIENTS);
    memset(child_pids, 0, sizeof(int) * MAX_CLIENTS);

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
    char input[BUFSIZE];
    int bytes_read;


    // Socket erstellen
    rfd = socket(AF_INET, SOCK_STREAM, 0);
    if (rfd < 0) {
        fprintf(stderr, "Could not create socket\n");
        exit(-1);
    }

    int option = 1;
    setsockopt(rfd, SOL_SOCKET, SO_REUSEADDR, (const void *) &option, sizeof(int));

    printf("Socket created.\n");
    printf("Starting server on port %i\n", PORT);

    // Socket binden
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);
    int brt = bind(rfd, (struct sockaddr *) &server, sizeof(server));
    if (brt < 0) {
        fprintf(stderr, "Could not bind socket\n");
        exit(-1);
    }

    // Socket lauschen lassen
    int lrt = listen(rfd, 5);
    if (lrt < 0) {
        fprintf(stderr, "Socket failed to listen\n");
        exit(-1);
    }

    // Verbindung eines Clients wird entgegengenommen
    while (TRUE) {
        printf("Waiting on connections...\n");
        printf("Client %i\n", *shm_count);
        int c = sizeof(struct sockaddr_in);
        cfd = accept(rfd, (struct sockaddr *) &client,
                     (socklen_t *) &c);
        *shm_count = *shm_count + 1;


        int pid = fork();
        if (pid < 0) {
            *shm_count = *shm_count - 1;
            printf("Fork failed. Could not create child process.\n");
            close(cfd);
            close(rfd);
        } else if (pid == 0) {

            if (cfd < 0) {
                perror("Could not accept connection.\n");
                exit(1);
            }

            /*------------Subscribed Keys-------------*/
            struct pubsubkey {
                char key[100];
            };

            struct pubsubkey subbedkeys[100];

            for (int i = 0; i < 20; i++) {
                strcpy(subbedkeys[i].key, "*");
            }


            while (TRUE) {

                bzero(input, BUFSIZE);
                bytes_read = read(cfd, input, BUFSIZE);

                struct sembuf enter, leave;
                enter.sem_num = leave.sem_num = 0;
                enter.sem_flg = leave.sem_flg = SEM_UNDO;
                enter.sem_op = -1;
                leave.sem_op = 1;

                if (bytes_read > 0) {

                    if (*transactionblock == 0 ||
                        cfd == *blockingclient) {

                        if (strncmp("NUMCLIENTS", stoupper(input), 10) == 0) {
                            char num_clients_str[5];
                            sprintf(num_clients_str, "%i", *shm_count);
                            char message[100];
                            memset(message, 0, sizeof(message));
                            strncat(message, num_clients_str, strlen(num_clients_str));
                            strncat(message, "\n", 1);
                            sendMessage(cfd, message, strlen(message));

                        } else if (strncmp("QUIT", stoupper(input), 4) == 0) {
                            *shm_count = *shm_count - 1;
                            printf("Closing socket...\n");
                            sendMessage(cfd, "Closing socket...\n", 24);
                            shutdown(cfd, SHUT_RDWR);
                            close(cfd);
                            fflush(stdout);
                            exit(0);

                            /*} else if (strncmp("KILL", stoupper(input), 4) == 0) {
                                put("kill", "kill");*/
                        } else if (strncmp("GET", stoupper(input), 3) == 0) {
                            struct keyval *kvp = malloc(sizeof(struct keyval));
                            readcommand(input, kvp);
                            char res[100];
                            semop(sem_id, &enter, 1);
                            printf("Return-Code: %i \n", get(kvp->key, res));
                            semop(sem_id, &leave, 1);
                            printf("GET:%s:%s \n", kvp->key, res);

                            char message[100];
                            memset(message, 0, sizeof(message));
                            strncat(message, "GET:", 6);
                            strncat(message, kvp->key, 100);
                            strncat(message, ":", 1);
                            strncat(message, res, 100);
                            strncat(message, "\n", 1);
                            sendMessage(cfd, message, strlen(message));


                        } else if (strncmp("PUT", stoupper(input), 3) == 0) {
                            struct keyval *kvp = malloc(sizeof(struct keyval));
                            readcommand(input, kvp);
                            semop(sem_id, &enter, 1);
                            printf("%d\n", put(kvp->key, kvp->val));
                            semop(sem_id, &leave, 1);
                            printf("PUT:%s:%s\n", kvp->key, kvp->val);

                            char message[100];
                            memset(message, 0, sizeof(message));
                            strncat(message, "PUT:", 6);
                            strncat(message, kvp->key, 100);
                            strncat(message, ":", 1);
                            strncat(message, kvp->val, 100);
                            strncat(message, "\n", 1);
                            sendMessage(cfd, message, strlen(message));

                            msgsnd(msid, message, strlen(message), 0);


                        } else if (strncmp("DEL", stoupper(input), 3) == 0) {
                            struct keyval *kvp = malloc(sizeof(struct keyval));
                            readcommand(input, kvp);

                            char message[100];
                            memset(message, 0, sizeof(message));
                            strncat(message, "DEL:", 6);
                            strncat(message, kvp->key, 100);
                            strncat(message, ":", 1);
                            semop(sem_id, &enter, 1);
                            if (del(kvp->key) == 0) {
                                strncat(message, "key_deleted", 11);
                                printf("Key %s Deleted\n", kvp->key);
                            } else {
                                strncat(message, "key_nonexistent", 16);
                            }
                            semop(sem_id, &leave, 1);

                            strncat(message, "\n", 1);
                            sendMessage(cfd, message, strlen(message));

                            msgsnd(msid, message, strlen(message), 0);

                        } else if (strncmp("SUB", stoupper(input), 3) == 0) {
                            struct keyval *kvp = malloc(sizeof(struct keyval));
                            readcommand(input, kvp);

                            for (int i = 0; i < 100; i++) {
                                if ((strcmp(subbedkeys[i].key, "*") == 0)) {
                                    strcpy(subbedkeys[i].key, kvp->key);

                                    char message[100];
                                    memset(message, 0, sizeof(message));
                                    strncat(message, "SUB:", 6);
                                    strncat(message, kvp->key, 100);
                                    strncat(message, "\n", 1);
                                    sendMessage(cfd, message, strlen(message));
                                    break;
                                }
                            }

                            char notification[100];
                            int subPid = fork();
                            if (subPid == 0) {
                                // clear the messages from before the subscription
                                int receive = 1;
                                while (receive > 0) {
                                    receive = msgrcv(msid, &notification, sizeof(notification), 0, IPC_NOWAIT);
                                }
                                memset(notification, 0, sizeof(notification));

                                while (TRUE) {
                                    int receive = msgrcv(msid, &notification, sizeof(notification), 0, IPC_NOWAIT);
                                    char subcommand[100];
                                    strcpy(subcommand, notification);
                                    struct keyval *subkvp = malloc(sizeof(struct keyval));
                                    readcommand(notification, subkvp);


                                    if (receive >= 0) {
                                        for (int i = 0; i < 100; i++) {
                                            if (strcmp(subbedkeys[i].key, subkvp->key) == 0) {
                                                sendMessage(cfd, subcommand, strlen(subcommand));
                                            }
                                        }
                                    }
                                }
                            }
                        } else if (strncmp("BEG", stoupper(input), 3) == 0) {
                            *transactionblock = 1;
                            *blockingclient = cfd;
                            sendMessage(cfd, "Begin Transaction.\n", 18);
                        } else if (strncmp("END", stoupper(input), 3) == 0) {
                            *transactionblock = 0;
                            *blockingclient = 0;
                            sendMessage(cfd, "Transaction ended.\n", 18);
                        } else {
                            sendMessage(cfd, "Unknown Command.\n", 16);
                        }
                    } else {
                        sendMessage(cfd, "You are blocked right now.\n", 26);
                    }
                }
            }
        } else {
            /*for (int i = 0; i < MAX_CLIENTS; i++) {
                if (child_pids[i] == 0) {
                    child_pids[i] = pid;
                    break;
                }
            }

            char *response = malloc(sizeof(char) * 100);
            if (get("kill", response) == 0) {
                for (int j = 0; j < MAX_CLIENTS; j++) {
                    if (child_pids[j] == 0){
                        continue;
                    }
                    kill(child_pids[j], SIGTERM);
                }
                goto Exit;
            }*/

        }
    }

    Exit:
    shmdt(shm_count);
    shmdt(shar_mem);
    shmctl(shm_id, IPC_RMID, 0);
    shmctl(shm_id_transblock, IPC_RMID, 0);
    shmctl(shm_id_blockingclient, IPC_RMID, 0);
    semctl(sem_id, 0, IPC_RMID, 0);
    close(cfd);
    close(rfd);
    exit(0);
}




