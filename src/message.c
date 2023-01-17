/*
 * input-emulator - a simple input emulator
 *
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
#include <mqueue.h>
#include "message.h"
#include "print.h"

#define QUEUE_NAME  "/input-emulator"
#define MAX_SIZE    1024

static mqd_t mq;

void message_queue_server_open(void)
{
    struct mq_attr attr;

    debug_printf("Server opening queue\n");

    /* Initialize queue attributes */
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_SIZE;
    attr.mq_curmsgs = 0;

    /* Create message queue */
    if ((mq = mq_open(QUEUE_NAME, O_CREAT | O_RDWR | O_EXCL, 0644, &attr) ) == -1)
    {
        perror("mq_open() failure");
        exit (EXIT_FAILURE);
    }
}

void message_queue_client_open(void)
{
    debug_printf("Client opening queue\n");

    /* Open message queue */
    if ((mq = mq_open(QUEUE_NAME, O_RDWR) ) == -1)
    {
        perror("mq_open() failure");
        exit (EXIT_FAILURE);
    }
}

void message_queue_server_close(void)
{
    debug_printf("Server closing mqueue\n");

    /* Cleanup */
    mq_close(mq);
    mq_unlink(QUEUE_NAME);
}

void message_queue_client_close(void)
{
    debug_printf("Client closing mqueue\n");

    /* Cleanup */
    mq_close(mq);
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

    status = mq_send(mq, message, sizeof(message_header_t) + header->payload_length, 0);
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
    bytes_read = mq_receive(mq, buffer, MAX_SIZE, NULL);
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
