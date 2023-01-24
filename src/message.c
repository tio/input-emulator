/*
 * Copyright (C) 2023  DEIF A/S
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <mqueue.h>
#include <errno.h>
#include "message.h"
#include "print.h"

#define QUEUE_NAME_SRV_OUT  "/input-emulator-srv-out"
#define QUEUE_NAME_SRV_IN   "/input-emulator-srv-in"
#define MAX_SIZE    1024

static mqd_t mq_out;
static mqd_t mq_in;

void message_queue_server_open(void)
{
    struct mq_attr attr;

    debug_printf("Server opening message queues\n");

    /* Initialize queue attributes */
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_SIZE;
    attr.mq_curmsgs = 0;

    /* Create outgoing message queue */
    if ((mq_out = mq_open(QUEUE_NAME_SRV_OUT, O_CREAT | O_WRONLY | O_EXCL, 0644, &attr) ) == -1)
    {
        perror("mq_open() failure");
        exit (EXIT_FAILURE);
    }

    /* Create incoming message queue */
    if ((mq_in = mq_open(QUEUE_NAME_SRV_IN, O_CREAT | O_RDONLY | O_EXCL, 0644, &attr) ) == -1)
    {
        perror("mq_open() failure");
        exit (EXIT_FAILURE);
    }
}

void message_queue_client_open(void)
{
    int status;

    debug_printf("Client opening queues\n");

    /* Open incoming message queue */
    if ((mq_in = mq_open(QUEUE_NAME_SRV_OUT, O_RDONLY) ) == -1)
    {
        perror("mq_open() failure");
        exit (EXIT_FAILURE);
    }

    /* Open outgoing message queue */
    if ((mq_out = mq_open(QUEUE_NAME_SRV_IN, O_WRONLY) ) == -1)
    {
        perror("mq_open() failure");
        exit (EXIT_FAILURE);
    }

    /* Put advisory lock on message queue file so we don't open it if already open (busy) */
    status = flock(mq_in, LOCK_EX | LOCK_NB);
    if ((status == -1) && (errno == EWOULDBLOCK))
    {
        perror("Device file is locked by another process");
        exit(EXIT_FAILURE);
    }
}

void message_queue_server_close(void)
{
    debug_printf("Server closing mqueue\n");

    /* Cleanup */
    mq_close(mq_out);
    mq_unlink(QUEUE_NAME_SRV_OUT);
    mq_close(mq_in);
    mq_unlink(QUEUE_NAME_SRV_IN);
}

void message_queue_client_close(void)
{
    debug_printf("Client closing mqueue\n");

    /* Cleanup */
    mq_close(mq_out);
    flock(mq_in, LOCK_UN);
    mq_close(mq_in);
}

int msg_create(
        void **message,
        message_type_t type,
        void *payload,
        uint32_t payload_length)
{
    message_header_t *header;
    char *payload_p;

    // Allocate memory for message buffer
    *message = malloc(sizeof(message_header_t) + payload_length);
    if (*message == NULL)
    {
        perror("Failed to allocate memory for message\n");
        return -1;
    }

    // Create message header
    header = *message;
    header->type = type;
    header->payload_length = payload_length;

    // Copy payload if any
    if (payload_length > 0)
    {
        payload_p = *message;
        memcpy(payload_p + sizeof(message_header_t), payload, payload_length);
    }

    return 0;
}

void msg_destroy(void *message)
{
    free(message);
}

int msg_send(void *message)
{
    int status;

    // Send message via mq queue
    message_header_t *header = message;

    status = mq_send(mq_out, message, sizeof(message_header_t) + header->payload_length, 0);
    if (status == -1)
    {
        perror("msg_send() failed");
    }

    return status;
}

int msg_receive(void **message)
{
    char buffer[MAX_SIZE + 1];
    ssize_t bytes_read;
    message_header_t *header = (message_header_t *)buffer;

    /* Receive message */
    bytes_read = mq_receive(mq_in, buffer, MAX_SIZE, NULL);
    if (bytes_read < 0)
    {
        perror("mq_receive failure");
        exit(EXIT_FAILURE);
    }

    // Allocate message (header + payload) receive buffer
    *message = malloc(sizeof(message_header_t) + header->payload_length);
    if (*message == NULL)
    {
        perror("malloc() failed\n");
        return -1;
    }

    // Install header
    memcpy(*message, header, sizeof(message_header_t));

    // Install payload
    char *payload_p = *message + sizeof(message_header_t);
    char *payload_buffer_p = &buffer[sizeof(message_header_t)];
    memcpy(payload_p, payload_buffer_p, header->payload_length);

    return 0;
}

void msg_send_rsp_ok(void)
{
    void *message = NULL;

    // Send response
    msg_create(&message, RSP_OK, NULL, 0);
    msg_send(message);
    msg_destroy(message);
}

void msg_receive_rsp_ok(void)
{
    void *message = NULL;
    message_header_t *header;

    // Receive response
    msg_receive(&message);
    header = message;
    if (header->type == RSP_ERROR)
    {
        // char *rsp_text = message + sizeof(message_header_t);
        // error_printf("%s", rsp_text);
    }
    else if (header->type != RSP_OK)
    {
        warning_printf("Invalid message type received\n");
    }

    msg_destroy(message);
}
