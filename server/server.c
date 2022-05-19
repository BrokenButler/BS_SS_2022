#include "server.h"

int readcommand(char *input, struct kvp *command) {
    memset(command, 0, sizeof(struct kvp));

    char *delimiter = " ,:";

    char *token = strtok(input, delimiter);

    int counter = 0;
    while (token != NULL) {
        if (counter == 1) {
            strcpy(command->key, token);
        } else if (counter == 2) {
            strcpy(command->value, token);
        }
        counter++;

        token = strtok(NULL, delimiter);
    }

    command->key[strcspn(command->key, "\r\n")] = 0;
    command->value[strcspn(command->value, "\r\n")] = 0;

    return 0;

}

void sendMessage(int clientSock, char *message, size_t messageLength) {
    long err = send(clientSock, message, messageLength, 0);
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

void startService() {

    char NUM_CLIENTS[10];

    int sem_id = init_semaphore();
    init_store();  //KeyValStore wird initialisiert

    int shm_id, shm_id_transblock, shm_id_blockingclient, shm_id_notify, *shm_count, shm_count_id;    //Shared Memory Segment wird angelegt // count neu für Aufgabe 5
    struct keyval *shar_mem;

    shm_id = shmget(IPC_PRIVATE, SEGMENT_SIZE, IPC_CREAT | 0644);
    shm_id_transblock = shmget(IPC_PRIVATE, SEGMENT_SIZE, IPC_CREAT | 0644);
    shm_id_blockingclient = shmget(IPC_PRIVATE, SEGMENT_SIZE, IPC_CREAT | 0644);

    shar_mem = (struct keyval *) shmat(shm_id, 0, 0);
    // Integer Variable für das Hochzählen der Clients wird erstellt und einem neu angelegten Shared Memory hinzugefügt
    shm_count_id = shmget(IPC_PRIVATE, SEGMENT_SIZE, IPC_CREAT | 0644);
    shm_count = (int *) shmat(shm_count_id, 0, 0);
    *shm_count = 0; //Auf 0 setzten, da am Anfang 0 Clients verbunden sind

    int *transactionblock = (int *) shmat(shm_id_transblock, 0, 0);
    int *blockingclient = (int *) shmat(shm_id_blockingclient, 0, 0);

    int msid = msgget(1, IPC_CREAT | 0644);     //Nachrichtenwarteschlange anlegen


    if (shm_id == -1) {
        perror("Shared Memory Segment konnte nicht angelegt werden!");
        exit(1);
    }


    int rfd; // Rendevouz-Descriptor
    int cfd; // Verbindungs-Descriptor

    struct sockaddr_in client; // Socketadresse eines Clients
    socklen_t client_len; // Länge der Client-Daten
    char input[BUFSIZE]; // Daten vom Client an den Server
    int bytes_read; // Anzahl der Bytes, die der Client geschickt hat


    // Socket erstellen
    rfd = socket(AF_INET, SOCK_STREAM, 0);
    if (rfd < 0) {
        fprintf(stderr, "socket konnte nicht erstellt werden\n");
        exit(-1);
    }

    // Socket Optionen setzen für schnelles wiederholtes Binden der Adresse
    int option = 1;
    setsockopt(rfd, SOL_SOCKET, SO_REUSEADDR, (const void *) &option, sizeof(int));

    printf("Socket created.\n");
    printf("Starting Server on port %i\n", PORT);

    // Socket binden
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);
    int brt = bind(rfd, (struct sockaddr *) &server, sizeof(server));
    if (brt < 0) {
        fprintf(stderr, "Unable to bind socket\n");
        exit(-1);
    }

    // Socket lauschen lassen
    int lrt = listen(rfd, 10);
    if (lrt < 0) {
        fprintf(stderr, "Socket could not listen\n");
        exit(-1);
    }

    // Verbindung eines Clients wird entgegengenommen
    Listen:
    printf("Waiting for clients...\n");
    printf("Client %i\n", *shm_count);
    int c = sizeof(struct sockaddr_in);
    //Zustand blockiert, da auf User input (verbinden des Clients) gewartet wird
    cfd = accept(rfd, (struct sockaddr *) &client, (socklen_t *) &c);
    *shm_count = *shm_count + 1;


    int pid = fork();
    if (pid < 0) {
        *shm_count = *shm_count - 1;
        printf("Failed to fork. Could not create new process.\n");
        close(cfd);
        close(rfd);
    } else if (pid == 0) {

        if (cfd < 0) {
            perror("Connection failed.\n");
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


        while (1) {

            char message[100];
            memset(message, 0, sizeof(message));

            // Lesen von Daten, die der Client schickt
            bzero(input, BUFSIZE);
            //Zustand blockiert, da auf USER Input gewartet wird
            bytes_read = read(cfd, input, BUFSIZE);

            struct sembuf enter, leave; // Structs für den Semaphor
            enter.sem_num = leave.sem_num = 0;  // Semaphor 0 input der Gruppe
            enter.sem_flg = leave.sem_flg = SEM_UNDO;
            enter.sem_op = -1; // blockieren, DOWN-Operation
            leave.sem_op = 1;   // freigeben, UP-Operation

            semop(sem_id, &enter, 1); //Kritischer Bereich 1 Anfang

            if (bytes_read > 0) {

                if (*transactionblock == 0 || cfd == *blockingclient) {

                    struct kvp *kvp;
                    memset(kvp, 0, sizeof(struct kvp));

                    if (strncmp("NUMCLIENTS", input, 10) == 0) {
                        sprintf(NUM_CLIENTS, "%i", *shm_count);
                        memset(&message[0], 0, sizeof(message));
                        strncat(message, NUM_CLIENTS, strlen(NUM_CLIENTS));
                        strncat(message, "\n", 1);
                        sendMessage(cfd, message, strlen(message));

                    } else if (strncmp("QUIT", input, 4) == 0) {
                        *shm_count = *shm_count -
                                     1;
                        printf("Socket wird geschlossen...\n");
                        sendMessage(cfd, "Socket wird geschlossen\n", 24);
                        shutdown(cfd, SHUT_RDWR);
                        close(cfd);
                        fflush(stdout);
                        exit(0);

                    } else if (strncmp("GET", input, 3) == 0) {
                        readcommand(input, kvp);
                        char *res[40];
                        memset(res[0], 0, sizeof(res)); //result wird gelöscht
                        //semop(sem_id, &enter, 1); //KB 04 Anfang - Verbesserungsvorschlag Aufgabe 4 Teil e)
                        printf("Return-Code: %i \n", get(kvp->key, *res));
                        //semop(sem_id, &leave, 1); //KB 04 Ende - Verbesserungsvorschlag Aufgabe 4 Teil e)
                        printf("GET:%s:%s \n", kvp->key, *res);

                        memset(&message[0], 0, sizeof(message));
                        strncat(message, "GET:",
                                6);                                                                //Message an den Client wird gebaut
                        strncat(message, kvp->key, 20);
                        strncat(message, ":", 1);
                        strncat(message, *res, 20);
                        strncat(message, "\n", 1);
                        sendMessage(cfd, message, strlen(message));


                    } else if (strncmp("PUT", input, 3) == 0) {
                        readcommand(input, kvp);
                        //semop(sem_id, &enter, 1); //KB 02 Anfang - Verbesserungsvorschlag Aufgabe 4 Teil d)
                        printf("%d\n", put(kvp->key, kvp->value));
                        //semop(sem_id, &leave, 1); //KB 02 Ende - Verbesserungsvorschlag Aufgabe 4 Teil d)
                        printf("PUT:%s:%s\n", kvp->key, kvp->value);

                        memset(&message[0], 0, sizeof(message));
                        strncat(message, "PUT:", 6);
                        strncat(message, kvp->key, 30);
                        strncat(message, ":", 1);
                        strncat(message, kvp->value, 30);
                        strncat(message, "\n", 1);
                        sendMessage(cfd, message, strlen(message));

                        msgsnd(msid, message, strlen(message),
                               0);                                    //Nachrichtenwarteschlange



                    } else if (strncmp("DEL", input, 3) == 0) {
                        readcommand(input, kvp);

                        memset(&message[0], 0, sizeof(message));
                        strncat(message, "DEL:", 6);
                        strncat(message, kvp->key, 20);
                        strncat(message, ":", 1);
                        //semop(sem_id, &enter, 1); //KB 03 Anfang - Verbesserungsvorschlag Aufgabe 4 Teil d)
                        if (del(kvp->key) == 0) {
                            strncat(message, "key_deleted", 11);
                            printf("key %s Deleted\n", kvp->key);
                        } else {
                            strncat(message, "key_nonexistent", 16);
                        }
                        //semop(sem_id, &leave, 1); //KB 03 Ende - Verbesserungsvorschlag Aufgabe 4 Teil d)

                        strncat(message, "\n", 1);
                        sendMessage(cfd, message, strlen(message));

                        msgsnd(msid, message, strlen(message), 0);

                    } else if (strncmp("SUB", input, 3) == 0) {
                        readcommand(input, kvp);

                        for (int i = 0; i < 20; i++) {
                            if ((strcmp(subbedkeys[i].key, "*") == 0)) {
                                strcpy(subbedkeys[i].key, kvp->key);
                                memset(&message[0], 0, sizeof(message));
                                strncat(message, "SUB:", 6);
                                strncat(message, kvp->key, 20);
                                strncat(message, "\n", 1);
                                sendMessage(cfd, message, strlen(message));
                                break;
                            }
                        }

                        char notification[50];
                        int subPid = fork();  //Neuer Prozess, welcher überwacht, ob an abonnierten Keys Änderungen vorgenommen werden
                        if (subPid == 0) {
                            while (1) {
                                int receive = msgrcv(msid, &notification, sizeof(notification), 0, IPC_NOWAIT);
                                char subcommand[50];
                                strcpy(subcommand, notification);
                                readcommand(notification, kvp);


                                if (receive >= 0) {
                                    for (int i = 0; i < 20; i++) {
                                        if (strcmp(subbedkeys[i].key, kvp->key) == 0) {
                                            sendMessage(cfd, subcommand, strlen(subcommand));
                                        }
                                    }

                                }
                            }
                        }

                    } else if (strncmp("BEG", input, 3) == 0) {
                        *transactionblock = 1;
                        *blockingclient = cfd;
                        sendMessage(cfd, "Begin Transaction\n", 18);
                    } else if (strncmp("END", input, 3) == 0) {
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
