#ifndef WIFICLIENT_H
#define WIFICLIENT_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <cstring>
#include <stdint.h>
#include <sys/ioctl.h>
#include <iostream>

class WiFiClient {
    int _fd = -1;
public:
    bool connect(const char* host, uint16_t port) {
        struct hostent *server = gethostbyname(host);
        if (server == NULL) return false;

        _fd = socket(AF_INET, SOCK_STREAM, 0);
        if (_fd < 0) return false;

        struct sockaddr_in serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
        serv_addr.sin_port = htons(port);

        if (::connect(_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            close(_fd);
            _fd = -1;
            return false;
        }

        return true;
    }

    void stop() {
        if (_fd != -1) {
            close(_fd);
            _fd = -1;
        }
    }

    size_t write(const uint8_t *buf, size_t size) {
        if (_fd == -1) return 0;
        return ::write(_fd, buf, size);
    }

    int read(uint8_t *buf, size_t size) {
        if (_fd == -1) return -1;
        return ::read(_fd, buf, size);
    }

    int readBytes(char *buf, size_t size) {
        size_t total = 0;
        while (total < size) {
            int n = ::read(_fd, (uint8_t*)buf + total, size - total);
            if (n > 0) {
                total += n;
            } else if (n < 0) {
                if (errno == EINTR) continue;
                break;
            } else {
                break; // EOF
            }
        }
        return (int)total;
    }

    int available() {
        if (_fd == -1) return 0;
        int count;
        if (ioctl(_fd, FIONREAD, &count) < 0) return 0;
        return count;
    }

    int fd() { return _fd; }
};

#endif
