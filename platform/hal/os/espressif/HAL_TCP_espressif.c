/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */





#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "iot_import.h"
#include "iotx_hal_internal.h"

#define HAL_TCP_CONNECT_TIMEOUT 10 * 1000000

static uint64_t _linux_get_time_ms(void)
{
    struct timeval tv = { 0 };
    uint64_t time_ms;

    gettimeofday(&tv, NULL);

    time_ms = tv.tv_sec * 1000 + tv.tv_usec / 1000;

    return time_ms;
}

static uint64_t _linux_time_left(uint64_t t_end, uint64_t t_now)
{
    uint64_t t_left;

    if (t_end > t_now) {
        t_left = t_end - t_now;
    } else {
        t_left = 0;
    }

    return t_left;
}

int HAL_TCP_Timeout(int fd, unsigned long usec)
{
    int ret = 0;
    struct timeval tm;
    fd_set set;
    int error = -1, len = sizeof(int);

    tm.tv_sec  = usec / 1000000;
    tm.tv_usec = usec % 1000000;
    FD_ZERO(&set);
    FD_SET(fd, &set);

    if (select(fd + 1, NULL, &set, NULL, &tm) <= 0) {
        ret = false;
    } else {
        getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, (socklen_t *) &len);
        ret = (error == 0) ? true : false;
    }

    return ret;
}

uintptr_t HAL_TCP_Establish(const char *host, uint16_t port)
{
    int on = 1;
    int32_t ret = 0;
    int32_t client_fd = -1;
    struct sockaddr_in sock_addr;
    struct hostent* entry = NULL;

    do {
        entry = gethostbyname(host);
        vTaskDelay(500 / portTICK_RATE_MS);
    } while (entry == NULL);

    hal_info("Creat socket ...");
    client_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (client_fd < 0) {
        hal_err("Creat socket failed...");
        return -1;
    }
    
    hal_info("OK");

    hal_info("bind socket ......");
    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = 0;
    sock_addr.sin_port = htons(port);

    ret = bind(client_fd, (struct sockaddr*)&sock_addr, sizeof(sock_addr));

    if (ret) {
        hal_err("failed");
        close(client_fd);
        return -1;
    }

    hal_info("OK");

    hal_info("setsockopt SO_REUSEADDR......");
    ret = setsockopt(client_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if (ret) {
        hal_warning("failed");
    } else {
        hal_info("OK");
    }

    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(port);
    sock_addr.sin_addr.s_addr = ((struct in_addr*)(entry->h_addr))->s_addr;

    hal_info("establish tcp connection with server(host=%s port=%u)", host, port);
    ioctl(client_fd, FIONBIO, &on);
    if (connect(client_fd, (struct sockaddr*) &sock_addr, sizeof(sock_addr)) == -1) {
        ret = HAL_TCP_Timeout(client_fd, HAL_TCP_CONNECT_TIMEOUT);
    } else {
        ret = true;
    }

    on = 0;
    ioctl(client_fd, FIONBIO, &on);

    if (!ret) {
        hal_info("failed");
        close(client_fd);
        client_fd = -1;
    } else {
        hal_info("OK");
    }

    return client_fd;
}


int HAL_TCP_Destroy(uintptr_t fd)
{
    int rc;

    /* Shutdown both send and receive operations. */
    rc = shutdown((int) fd, 2);
    if (0 != rc) {
        hal_err("shutdown error");
        return -1;
    }

    rc = close((int) fd);
    if (0 != rc) {
        hal_err("closesocket error");
        return -1;
    }

    return 0;
}


int32_t HAL_TCP_Write(uintptr_t fd, const char *buf, uint32_t len, uint32_t timeout_ms)
{
    int ret;
    uint32_t len_sent;
    uint64_t t_end, t_left;
    fd_set sets;

    t_end = _linux_get_time_ms() + timeout_ms;
    len_sent = 0;
    ret = 1; /* send one time if timeout_ms is value 0 */

    do {
        t_left = _linux_time_left(t_end, _linux_get_time_ms());

        if (0 != t_left) {
            struct timeval timeout;

            FD_ZERO(&sets);
            FD_SET(fd, &sets);

            timeout.tv_sec = t_left / 1000;
            timeout.tv_usec = (t_left % 1000) * 1000;

            ret = select(fd + 1, NULL, &sets, NULL, &timeout);
            if (ret > 0) {
                if (0 == FD_ISSET(fd, &sets)) {
                    hal_err("Should NOT arrive");
                    /* If timeout in next loop, it will not sent any data */
                    ret = 0;
                    continue;
                }
            } else if (0 == ret) {
                hal_err("select-write timeout %d", (int)fd);
                break;
            } else {
                if (EINTR == errno) {
                    hal_err("EINTR be caught");
                    continue;
                }

                hal_err("select-write fail");
                break;
            }
        }

        if (ret > 0) {
            ret = send(fd, buf + len_sent, len - len_sent, 0);
            if (ret > 0) {
                len_sent += ret;
            } else if (0 == ret) {
                hal_err("No data be sent");
            } else {
                if (EINTR == errno) {
                    hal_err("EINTR be caught");
                    continue;
                }

                hal_err("send fail");
                break;
            }
        }
    } while ((len_sent < len) && (_linux_time_left(t_end, _linux_get_time_ms()) > 0));

    return len_sent;
}


int32_t HAL_TCP_Read(uintptr_t fd, char *buf, uint32_t len, uint32_t timeout_ms)
{
    int ret, err_code;
    uint32_t len_recv;
    uint64_t t_end, t_left;
    fd_set sets;
    struct timeval timeout;

    t_end = _linux_get_time_ms() + timeout_ms;
    len_recv = 0;
    err_code = 0;

    do {
        t_left = _linux_time_left(t_end, _linux_get_time_ms());
        if (0 == t_left) {
            break;
        }
        FD_ZERO(&sets);
        FD_SET(fd, &sets);

        timeout.tv_sec = t_left / 1000;
        timeout.tv_usec = (t_left % 1000) * 1000;

        ret = select(fd + 1, &sets, NULL, NULL, &timeout);
        if (ret > 0) {
            ret = recv(fd, buf + len_recv, len - len_recv, 0);
            if (ret > 0) {
                len_recv += ret;
            } else if (0 == ret) {
                hal_err("connection is closed");
                err_code = -1;
                break;
            } else {
                if (EINTR == errno) {
                    hal_err("EINTR be caught");
                    continue;
                }
                hal_err("recv fail");
                err_code = -2;
                break;
            }
        } else if (0 == ret) {
            break;
        } else {
            hal_err("select-recv fail");
            err_code = -2;
            break;
        }
    } while ((len_recv < len));

    /* priority to return data bytes if any data be received from TCP connection. */
    /* It will get error code on next calling */
    return (0 != len_recv) ? len_recv : err_code;
}
