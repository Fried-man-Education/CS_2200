#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>

#include "queue.h"
#include "network.h"
#include "rtp.h"

typedef struct message {
    char *buffer;
    int length;
} message_t;

/* ================================================================ */
/*                  H E L P E R    F U N C T I O N S                */
/* ================================================================ */

/**
 * --------------------------------- PROBLEM 1 --------------------------------------
 * 
 * Convert the given buffer into an array of PACKETs and returns the array.  The 
 * value of (*count) should be updated so that it contains the number of packets in 
 * the array
 * 
 * @param buffer pointer to message buffer to be broken up packets
 * @param length length of the message buffer.
 * @param count number of packets in the returning array
 * 
 * @returns array of packets
 */
packet_t *packetize(char *buffer, int length, int *count) {
    packet_t * packets;
    int packetsTotal = length / MAX_PAYLOAD_LENGTH, temp = MAX_PAYLOAD_LENGTH;

    if (length % MAX_PAYLOAD_LENGTH) {
      temp = length % MAX_PAYLOAD_LENGTH;
      packetsTotal++;
    }

    packets = malloc(sizeof(packet_t) * (size_t) packetsTotal);
    for (int i = 0; i < packetsTotal - 1; i++) {
      packets[i].type = DATA;
      packets[i].payload_length = MAX_PAYLOAD_LENGTH;
      for (int j = 0; j < MAX_PAYLOAD_LENGTH; j++) packets[i].payload[j] = buffer[i * MAX_PAYLOAD_LENGTH + j];
      packets[i].checksum = checksum(packets[i].payload, MAX_PAYLOAD_LENGTH);
    }

    packets[packetsTotal - 1].type = LAST_DATA;
    packets[packetsTotal - 1].payload_length = temp;
    for (int i = 0; i < temp; i++) packets[packetsTotal - 1].payload[i] = buffer[(packetsTotal - 1) * MAX_PAYLOAD_LENGTH + i];
    packets[packetsTotal - 1].checksum = checksum(packets[packetsTotal - 1].payload, temp);
    * count = packetsTotal;

    return packets;
}

/**
 * --------------------------------- PROBLEM 2 --------------------------------------
 * 
 * Compute a checksum based on the data in the buffer.
 * 
 * Checksum calcuation: Sum the ascii values of all charecters in the buffer, if the 
 * index is even, multiply the ascii value by the index, and return the total. 
 * 
 * Example: "abcd" checksum = (0 * a) + (b) + (2 * c) + (d)
 * 
 * @param buffer pointer to the char buffer that the checksum is calculated from
 * @param length length of the buffer
 * 
 * @returns calcuated checksum
 */
int checksum(char *buffer, int length) {
    int output = 0;
    for (int i = 0; i < length; i++) output += buffer[i] * (!(i % 2) ? i : 1);
    return output;
}


/* ================================================================ */
/*                      R T P       T H R E A D S                   */
/* ================================================================ */

static void *rtp_recv_thread(void *void_ptr) {

    rtp_connection_t *connection = (rtp_connection_t *) void_ptr;

    do {
        message_t *message;
        int buffer_length = 0;
        char *buffer = NULL;
        packet_t packet;

        /* Put messages in buffer until the last packet is received  */
        do {
            if (net_recv_packet(connection->net_connection_handle, &packet) <= 0 || packet.type == TERM) {
                /* remote side has disconnected */
                connection->alive = 0;
                pthread_cond_signal(&connection->recv_cond);
                pthread_cond_signal(&connection->send_cond);
                break;
            }

            if (packet.type == LAST_DATA || packet.type == DATA) {
              packet_t * temp = malloc(sizeof(packet_t));

              if (checksum(packet.payload, packet.payload_length) == packet.checksum) {
                ( * temp).type = ACK;

                buffer = buffer ? realloc(buffer, (unsigned long) (buffer_length + packet.payload_length)) : malloc((unsigned long) packet.payload_length);
                for (int i = 0; i < packet.payload_length; i++) buffer[buffer_length + i] = packet.payload[i];
                buffer_length += packet.payload_length;
              } else {
                ( * temp).type = NACK;

                if (packet.type == LAST_DATA) packet.type = DATA;
              }

              net_send_packet(( * connection).net_connection_handle, temp);
              free(temp);
            } else if (packet.type == ACK || packet.type == NACK) {
              pthread_mutex_lock( & ( * connection).ack_mutex);

              ( * connection).ack = packet.type == ACK ? 1 : 2;

              pthread_cond_signal( & ( * connection).ack_cond);
              pthread_mutex_unlock( & ( * connection).ack_mutex);
            }
        } while (packet.type != LAST_DATA);

        if (connection->alive == 1) {
            message = malloc(sizeof(message_t));
            ( * message).length = buffer_length;
            ( * message).buffer = buffer;
            pthread_mutex_lock( & ( * connection).recv_mutex);

            queue_add( & ( * connection).recv_queue, message);

            pthread_mutex_unlock( & ( * connection).recv_mutex);
            pthread_cond_signal( & ( * connection).recv_cond);

        } else free(buffer);

    } while (connection->alive == 1);

    return NULL;

}

static void *rtp_send_thread(void *void_ptr) {

    rtp_connection_t *connection = (rtp_connection_t *) void_ptr;
    message_t *message;
    int array_length = 0;
    int i;
    packet_t *packet_array;

    do {
        /* Extract the next message from the send queue */
        pthread_mutex_lock(&connection->send_mutex);
        while (queue_size(&connection->send_queue) == 0 && connection->alive == 1) {
            pthread_cond_wait(&connection->send_cond, &connection->send_mutex);
        }

        if (connection->alive == 0) break;

        message = queue_extract(&connection->send_queue);

        pthread_mutex_unlock(&connection->send_mutex);

        /* Packetize the message and send it */
        packet_array = packetize(message->buffer, message->length, &array_length);

        for (i = 0; i < array_length; i++) {

            /* Start sending the packetized messages */
            if (net_send_packet(connection->net_connection_handle, &packet_array[i]) <= 0) {
                /* remote side has disconnected */
                connection->alive = 0;
                break;
            }

            pthread_mutex_lock( & ( * connection).ack_mutex);

            while (!( * connection).ack) pthread_cond_wait( & ( * connection).ack_cond, & ( * connection).ack_mutex);
            if (( * connection).ack == 2) i--;
            ( * connection).ack = 0;

            pthread_mutex_unlock( & ( * connection).ack_mutex);
        }

        free(packet_array);
        free(message->buffer);
        free(message);
    } while (connection->alive == 1);
    return NULL;


}

static rtp_connection_t *rtp_init_connection(int net_connection_handle) {
    rtp_connection_t *rtp_connection = malloc(sizeof(rtp_connection_t));

    if (rtp_connection == NULL) {
        fprintf(stderr, "Out of memory!\n");
        exit(EXIT_FAILURE);
    }

    rtp_connection->net_connection_handle = net_connection_handle;

    queue_init(&rtp_connection->recv_queue);
    queue_init(&rtp_connection->send_queue);

    pthread_mutex_init(&rtp_connection->ack_mutex, NULL);
    pthread_mutex_init(&rtp_connection->recv_mutex, NULL);
    pthread_mutex_init(&rtp_connection->send_mutex, NULL);
    pthread_cond_init(&rtp_connection->ack_cond, NULL);
    pthread_cond_init(&rtp_connection->recv_cond, NULL);
    pthread_cond_init(&rtp_connection->send_cond, NULL);

    rtp_connection->alive = 1;

    pthread_create(&rtp_connection->recv_thread, NULL, rtp_recv_thread,
                   (void *) rtp_connection);
    pthread_create(&rtp_connection->send_thread, NULL, rtp_send_thread,
                   (void *) rtp_connection);

    return rtp_connection;
}

/* ================================================================ */
/*                           R T P    A P I                         */
/* ================================================================ */

rtp_connection_t *rtp_connect(char *host, int port) {

    int net_connection_handle;

    if ((net_connection_handle = net_connect(host, port)) < 1)
        return NULL;

    return (rtp_init_connection(net_connection_handle));
}

int rtp_disconnect(rtp_connection_t *connection) {

    message_t *message;
    packet_t term;

    term.type = TERM;
    term.payload_length = term.checksum = 0;
    net_send_packet(connection->net_connection_handle, &term);
    connection->alive = 0;

    net_disconnect(connection->net_connection_handle);
    pthread_cond_signal(&connection->send_cond);
    pthread_cond_signal(&connection->recv_cond);
    pthread_join(connection->send_thread, NULL);
    pthread_join(connection->recv_thread, NULL);
    net_release(connection->net_connection_handle);

    /* emtpy recv queue and free allocated memory */
    while ((message = queue_extract(&connection->recv_queue)) != NULL) {
        free(message->buffer);
        free(message);
    }
    queue_release(&connection->recv_queue);

    /* emtpy send queue and free allocated memory */
    while ((message = queue_extract(&connection->send_queue)) != NULL) {
        free(message);
    }
    queue_release(&connection->send_queue);

    free(connection);

    return 1;

}

int rtp_recv_message(rtp_connection_t *connection, char **buffer, int *length) {

    message_t *message;

    if (connection->alive == 0)
        return -1;
    /* lock */
    pthread_mutex_lock(&connection->recv_mutex);
    while (queue_size(&connection->recv_queue) == 0 && connection->alive == 1) {
        pthread_cond_wait(&connection->recv_cond, &connection->recv_mutex);
    }

    if (connection->alive == 0) {
        pthread_mutex_unlock(&connection->recv_mutex);
        return -1;
    }

    /* extract */
    message = queue_extract(&connection->recv_queue);
    *buffer = message->buffer;
    *length = message->length;
    free(message);

    /* unlock */
    pthread_mutex_unlock(&connection->recv_mutex);

    return *length;
}

int rtp_send_message(rtp_connection_t *connection, char *buffer, int length) {

    message_t *message;

    if (connection->alive == 0)
        return -1;

    message = malloc(sizeof(message_t));
    if (message == NULL) {
        return -1;
    }
    message->buffer = malloc((size_t) length);
    message->length = length;

    if (message->buffer == NULL) {
        free(message);
        return -1;
    }

    memcpy(message->buffer, buffer, (size_t) length);

    /* lock */
    pthread_mutex_lock(&connection->send_mutex);

    /* add */
    queue_add(&(connection->send_queue), message);

    /* unlock */
    pthread_mutex_unlock(&connection->send_mutex);
    pthread_cond_signal(&connection->send_cond);
    return 1;

}
