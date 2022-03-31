


/** The libraries I used **/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/ioctl.h>
#include "chatServer.h"

/** Macro Block **/
#define BUFFER_SIZE 4096

/** Global Variables Block **/
static int end_server = 0;

/** Block for declaring the auxiliary functions **/
void input_test(int, char *[]);

int create_welcome_socket(int);

int create_client_socket(int);

/** Functions implementation block  **/
void intHandler() {
    end_server = 1;
}

int init_pool(conn_pool_t *pool) {
    if (pool == NULL) {
        puts("Allocation failure: Failed to allocate the desired amount of memory for your pool\n");
        return -1;
    }
    pool->maxfd = pool->nready = 0;
    pool->nr_conns = 0;
    pool->conn_head = NULL;
    FD_ZERO(&(pool->read_set));
    FD_ZERO(&(pool->ready_read_set));
    FD_ZERO(&(pool->write_set));
    FD_ZERO(&(pool->ready_write_set));
    return 0;
}

int add_conn(int sd, conn_pool_t *pool) {
    //Create a new connection
    conn_t *new_connection = (conn_t *) (malloc(sizeof(conn_t)));
    if (new_connection == NULL) {
        puts("Allocation failure: Failed to allocate the desired amount of memory for your pool");
        return -1;
    }
    //Update her info
    new_connection->fd = sd;
    new_connection->write_msg_head = new_connection->write_msg_tail = NULL;
    new_connection->next = new_connection->prev = NULL;
    //Add the new connection to the pool
    conn_t *p = pool->conn_head, *prev = NULL;
    for (; p != NULL; prev = p, p = p->next);
    if (prev == NULL)
        pool->conn_head = new_connection;
    else {
        prev->next = new_connection;
        new_connection->prev = prev;
    }
    //Update the info in the pool
    FD_SET(sd, &(pool->read_set));
    if (pool->maxfd < sd)
        pool->maxfd = sd;
    pool->nr_conns++;
    return 0;
}

int remove_conn(int sd, conn_pool_t *pool) {
    conn_t *p = pool->conn_head;
    while (p != NULL) {
        if (p->fd == sd) {
            //Update the info of the pool
            pool->nr_conns--;
            FD_CLR(sd, &(pool->read_set)), FD_CLR(sd, &(pool->write_set));
            //if we have some message to clean in the list
            for (msg_t *q = p->write_msg_head, *prev = NULL; q != NULL;) {
                prev = q, q = q->next;
                free(prev->message), prev->message = NULL;
                free(prev), prev = NULL;
            }
            //Update the max fd if this sd = maxfd
            if (sd == pool->maxfd) {
                int new_max = 3;
                for (conn_t *t = pool->conn_head; t != NULL; t = t->next)
                    if (t->fd != sd && t->fd > new_max)
                        new_max = t->fd;
                pool->maxfd = new_max;
            }
            //remove from pool
            if (p == pool->conn_head)
                pool->conn_head = pool->conn_head->next;
            else if (p->next == NULL)
                p->prev->next = NULL;
            else
                p->prev->next = p->next;
            //Deallocate the connection
            free(p), p = NULL;
            close(sd);
            printf("removing connection with sd %d \n", sd);
            return 0;
        }
        p = p->next;
    }
    return -1;
}

int add_msg(int sd, char *buffer, int len, conn_pool_t *pool) {
    //add this message to connection with the fd = sd
    conn_t *p = pool->conn_head;
    msg_t *q, *prev;
    while (p != NULL) {
        if (p->fd != sd) {
            //Allocate memory for the message struct
            msg_t *message = (msg_t *) malloc(sizeof(msg_t));
            if (message == NULL)
                return -1;
            //Allocate memory for the message content
            if ((message->message = (char *) malloc(sizeof(char) * (len + 1))) == NULL)
                return -1;
            //initialize the message
            memset(message->message, '\0', len + 1), strcpy(message->message, buffer);
            message->size = len;
            //Add this connection to the write_set
            FD_SET(p->fd, &(pool->write_set));
            //Add the message to this message list
            q = p->write_msg_head;
            prev = NULL;
            for (; q != NULL; prev = q, q = q->next);
            if (prev == NULL)
                p->write_msg_head = p->write_msg_tail = message;
            else {
                p->write_msg_tail->next = message;
                p->write_msg_tail = p->write_msg_tail->next;
            }
            p->write_msg_tail->next = NULL;
        }
        p = p->next;
    }
    return 0;
}

int write_to_client(int sd, conn_pool_t *pool) {
    conn_t *p = pool->conn_head;
    while (p != NULL) {
        //If this is the file descriptor to
        if (p->fd == sd) {
            msg_t *t = p->write_msg_head, *prev = NULL;
            //Each message of this client message list we will write to her file descriptor
            while (t != NULL) {
                ssize_t num_of_bytes = write(p->fd, t->message, t->size);
                if (num_of_bytes < 0 || num_of_bytes != t->size)
                    continue;
                prev = t, t = t->next;
                p->write_msg_head = t;
                free(prev->message), prev->message = NULL;
                free(prev), prev = NULL;
            }
            FD_CLR(sd, &(pool->write_set));
            return 0;
        }
        p = p->next;
    }
    return -1;
}

/** Auxiliary functions implementation block  **/
void input_test(int argc, char *argv[]) {
    //this is what tamar-bash the lecturer said
    if (argc != 2 || strtol(argv[1], NULL, 10) < 1024 || strtol(argv[1], NULL, 10) > 9999) {
        printf("Usage : chatServer <port>\n");
        exit(EXIT_SUCCESS);
    }
}

int create_welcome_socket(int port) {
    //Create a socket descriptor
    int welcome_sd = socket(PF_INET, SOCK_STREAM, 0);
    if (welcome_sd < 0) {
        perror("error: socket \n");
        exit(EXIT_FAILURE);
    }
    //Create sockaddr_in struct
    struct sockaddr_in srv;
    srv.sin_family = AF_INET, srv.sin_port = htons(port), srv.sin_addr.s_addr = htonl(INADDR_ANY);
    //Set that all sockets that are created from this welcome socket will be non-blocking
    int on = 1, rc = ioctl(welcome_sd, (int) (FIONBIO), (char *) &on);
    if (rc < 0) {
        perror("error: ioctl \n");
        return -1;
    }
    //Binding
    if (bind(welcome_sd, (struct sockaddr *) &srv, sizeof(srv)) < 0) {
        perror("error: bind \n");
        return -1;
    }
    //listening
    if (listen(welcome_sd, 5) < 0) {
        perror("error: listen \n");
        return -1;
    }
    return welcome_sd;
}

int create_client_socket(int welcome_sd) {
    struct sockaddr_in client;
    int client_sd, client_len = sizeof(client);
    if ((client_sd = accept(welcome_sd, (struct sockaddr *) &client, (socklen_t *) (&client_len))) < 0) {
        return -1;
    }
    printf("New incoming connection on sd %d\n", client_sd);
    return client_sd;
}

/**Block for the main function **/
int main(int argc, char *argv[]) {
    //check that the input is correct
    input_test(argc, argv);
    //A statement that we are handling the SIGINT signal
    signal(SIGINT, intHandler);
    //Initialize a pool
    conn_pool_t *pool = malloc(sizeof(conn_pool_t));
    if (init_pool(pool) == -1)
        return EXIT_FAILURE;
    //Welcome socket creation
    int welcome_sd;
    if ((welcome_sd = create_welcome_socket((int) strtol(argv[1], NULL, 10))) == -1)
        return EXIT_FAILURE;
    //Initialize the read_set
    FD_SET(welcome_sd, &(pool->read_set));
    pool->maxfd = welcome_sd;
    //Start the chatServer
    do {
        //first step: copy the master fd sets to the working sets
        pool->ready_read_set = pool->read_set;
        pool->ready_write_set = pool->write_set;
        //Second step: call to select
        printf("Waiting on select()...\nMaxFd %d\n", pool->maxfd);
        if ((pool->nready = select(pool->maxfd + 1, &pool->ready_read_set, &pool->ready_write_set, NULL, NULL)) < 0)
        {
            if(end_server)
                break;
            perror("error: select \n");
        }
        //We can assume at this point that after sure there are file descriptions which are ready to read or write
        for (int i = 3; i <= pool->maxfd; i++) {
            //Check if an event occurred within the read_set
            if (FD_ISSET(i, &(pool->ready_read_set))) {
                //if the fd is the welcome socket case
                if (i == welcome_sd) {
                    int client_sd = create_client_socket(welcome_sd);
                    if (client_sd == -1)
                        continue;
                    //Adding a new connection to connections list
                    add_conn(client_sd, pool);
                }
                    //if the fd is not the welcome socket case
                else {
                    char buffer[BUFFER_SIZE + 1];
                    ssize_t num_of_bytes;
                    memset(buffer, '\0', BUFFER_SIZE + 1);
                    printf("Descriptor %d is readable\n", i);
                    num_of_bytes = read(i, buffer, BUFFER_SIZE);
                    if (num_of_bytes < 0)
                        continue;
                    else if (num_of_bytes == 0) {
                        printf("Connection closed for sd %d\n", i);
                        remove_conn(i, pool);
                    } else {
                        printf("%d bytes received from sd %d\n", (int) num_of_bytes, i);
                        add_msg(i, buffer, (int) num_of_bytes, pool);
                    }

                }
                //If I went through all the file descriptions select reported to me, there is no reason to stay
                if (--pool->nready == 0)
                    break;
            }
            //Check if an event occurred within the write_set
            if (FD_ISSET(i, &(pool->ready_write_set))) {
                write_to_client(i, pool);
                if (--pool->nready == 0)
                    break;
            }
        }
    } while (end_server == 0);
    //Reset all the file descriptors
    for (int i = 3; i <= pool->maxfd; i++)
        if (FD_ISSET(i, &pool->read_set))
            close(i);
    // Deallocate memory Block
    conn_t *p1 = pool->conn_head, *p1_prev;
    msg_t *q1, *q1_prev;
    while (p1 != NULL) {
        printf("removing connection with sd %d \n", p1->fd);
        p1_prev = p1;
        p1 = p1->next;
        q1 = p1_prev->write_msg_head;
        while (q1 != NULL) {
            q1_prev = q1;
            q1 = q1->next;
            free(q1_prev->message), q1_prev->message = NULL;
            free(q1_prev), q1_prev = NULL;
        }
        free(p1_prev), p1_prev = NULL;
    }
    free(pool), pool = NULL;
    return EXIT_SUCCESS;
}