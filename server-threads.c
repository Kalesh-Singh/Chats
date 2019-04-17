//
// Created by kalesh on 4/16/19.
//

#include "server-threads.h"

char msg_buffer[MAX_SIZE];      // Message buffer
struct client_conn client_conns[MAX_CLIENT_CONNS];  // Client connections

void *send_msg(void *args) {
    struct thread_args *a = (struct thread_args *) args;
    while (a->conn->alive == 1) {   // TODO: Check if this is consistent if not change to sig_atomic (which is also int).
        // TODO: Since the write is just a decremetn to  an int, which is atomic on most systems. It might be sufficient
        // to mark this variable as volatile to prevent comiler optimizations.
        acquire_shared();
        send(a->conn->sockfd, (void*) a->msg_buffer, a->size, 0);
        release_shared();
    }
    return NULL;
}

void *recv_msg(void *args) {
    char tmp_buffer[MAX_SIZE];
    struct thread_args *a = (struct thread_args *) args;
    // Recv message into temp buffer
    // NOTE: Do not hold the lock while receiving as recv blocks.
    // Check if recv's return value is 0.
    // If this is the case: (In this exact order).
    // 1. Cancel the send thread. (pthread_cancel)
    // 2. Close the sock fd
    // 3. Set alive to 0.
    // 3. Exit this thread (pthread_exit).
    while (a->conn->alive == 1) {
        if (recv(a->conn->sockfd, (void *) &tmp_buffer, a->size, 0) == 0) {
            // TODO: Handle cleanup
            a->conn->alive--;
        } else {
            acquire_exclusive();
            printf("Recieved: %s\n", tmp_buffer);
            memcpy((void *) a->msg_buffer, (const void *) &tmp_buffer, MAX_SIZE);
            release_exclusive();
        }
    }
    return NULL;
}

void init_client_conns() {
    for (int i = 0; i < MAX_CLIENT_CONNS; i++) {
        client_conns[i].alive = 0;
    }
}

int next_client_conn() {
    for (int i = 0; i < MAX_CLIENT_CONNS; i++) {
        if (client_conns[i].alive == 0) {
            return i;
        }
    }
    return -1;
}

int add_client_conn(int sockfd) {
    int next = next_client_conn();
    if (next >= 0 && next < MAX_CLIENT_CONNS) {
        client_conns[next].sockfd = sockfd;
        client_conns[next].alive = 1;
        struct thread_args args = {msg_buffer, MAX_SIZE, &(client_conns[next])};
        pthread_create(&(client_conns[next].send_thread), NULL, send_msg, (void *) &args);
        pthread_create(&(client_conns[next].recv_thread), NULL, recv_msg, (void *) &args);
    }
    return next;
}
