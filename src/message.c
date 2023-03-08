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
#include <fcntl.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/un.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "message.h"
#include "print.h"

#define MAX_CLIENTS 1
#define MSG_SOCKET_NAME "input-emulator.socket"

static int srv_sockfd;
static int new_srv_sockfd;
static int cli_sockfd;
static int *sockfd = &new_srv_sockfd;

bool message_server_running(void)
{
    int r;
    struct sockaddr_un serv_addr;
    int sockfd;
    bool in_use_status = false;

    /* Create socket */
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        error_printf("Opening socket\n");
        exit(EXIT_FAILURE);
    }

    /* Initialize socket structure */
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path + 1, MSG_SOCKET_NAME); // Abstract socket

    /* Bind the host address using bind() call.*/
    r = bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    if (r < 0)
    {
        if (errno == EADDRINUSE)
        {
            in_use_status = true;
        }
        else
        {
            error_printf("On binding (%s)\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        close(sockfd);
    }

    return in_use_status;
}

void message_client_mode_enable(void)
{
    sockfd = &cli_sockfd;
}

void message_server_open(void)
{
    debug_printf("Server opening message socket\n");

    struct sockaddr_un srv_addr;

    debug_printf("Starting message server..\n");

    /* Create UNIX file socket */
    srv_sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (srv_sockfd < 0)
    {
        error_printf("Opening socket (%s)\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* Initialize socket structure */
    bzero((char *) &srv_addr, sizeof(srv_addr));
    srv_addr.sun_family = AF_UNIX;
    strcpy(srv_addr.sun_path + 1, MSG_SOCKET_NAME); // Use abstract socket

    /* Bind the host address using bind() call (creates socket file) */
    if (bind(srv_sockfd, (struct sockaddr *) &srv_addr, sizeof(srv_addr)) < 0)
    {
        error_printf("On binding (%s)\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void message_server_listen(void (*callback)(void))
{
    struct sockaddr_un cli_addr;
    socklen_t cli_len;

    /* Listen and sleep until incoming connection */
    listen(srv_sockfd, MAX_CLIENTS);
    cli_len = sizeof(cli_addr);

    while (1)
    {
        /* Block until a new connection is available */
        new_srv_sockfd = accept(srv_sockfd, (struct sockaddr *) &cli_addr, &cli_len);
        if (new_srv_sockfd < 0)
        {
            error_printf("On accept (%s)\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        /* Do callback which will handle incoming request by reading/writing
         * messages */
        callback();

        /* Close connection when done handling incoming request */
        close(new_srv_sockfd);
    }
}

void message_server_close(void)
{
    close(srv_sockfd);
}

void message_client_open(void)
{
    struct sockaddr_un srv_addr;

    debug_printf("Starting socket client\n");

    /* Create a UNIX file socket */
    cli_sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (cli_sockfd < 0)
    {
        error_printf("Opening socket (%s)\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* Initialize socket structure */
    bzero((char *) &srv_addr, sizeof(srv_addr));
    srv_addr.sun_family = AF_UNIX;
    strcpy(srv_addr.sun_path + 1, MSG_SOCKET_NAME); // Use abstract socket

    /* Connect to the server */
    if (connect(cli_sockfd, (struct sockaddr*)&srv_addr, sizeof(srv_addr)) < 0)
    {
        error_printf("Connect failure (%s)\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    message_client_mode_enable();
}

void message_client_close(void)
{
    debug_printf("Closing socket client\n");
    close(cli_sockfd);
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
    ssize_t bytes_sent;
    ssize_t bytes_remaining;
    char *message_p = message;

    message_header_t *header = message;
    bytes_remaining = sizeof(message_header_t) + header->payload_length;

    while (bytes_remaining)
    {
        bytes_sent = write(*sockfd, message_p, bytes_remaining);
        if (bytes_sent < 0)
        {
            warning_printf("Writing to socket (%s)\n", strerror(errno));
            return -errno;
        }

        bytes_remaining -= bytes_sent;
        message_p += bytes_sent;
    }

    return 0;
}

int msg_receive(void **message)
{
    message_header_t header;
    ssize_t bytes_read;
    ssize_t bytes_remaining;
    char *message_p;

    /* Read message header */
    bytes_read = read(*sockfd, &header, sizeof(header));
    if (bytes_read < 0)
    {
        error_printf("Reading from (%s)\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* Allocate message (header + payload) receive buffer */
    *message = malloc(sizeof(message_header_t) + header.payload_length);
    if (*message == NULL)
    {
        error_printf("malloc() failed (%s)\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* Install header */
    memcpy(*message, &header, sizeof(message_header_t));

    /* Read message payload */
    bytes_remaining = header.payload_length;
    message_p = *message;
    message_p += sizeof(message_header_t);

    while (bytes_remaining)
    {
        bytes_read = read(*sockfd, message_p, bytes_remaining);
        if (bytes_read < 0)
        {
            warning_printf("Writing to socket (%s)\n", strerror(errno));
            return -errno;
        }

        bytes_remaining -= bytes_read;
        message_p += bytes_read;
    }

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
