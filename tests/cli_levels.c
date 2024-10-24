#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

int main() {
        int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (sockfd < 0) {
                fprintf(stderr, "Error in \"socket(AF_UNIX, SOCK_DGRAM, 0)\"");
                exit(EXIT_FAILURE);
        }

        struct sockaddr_un sockaddr_server;
        sockaddr_server.sun_family = AF_UNIX;
        strcpy(sockaddr_server.sun_path, "sound_meter_server_socket");
        size_t len = sizeof(sockaddr_server.sun_family) + strlen(sockaddr_server.sun_path);
        int result = connect(sockfd, (struct sockaddr *)&sockaddr_server, len);
        if (result < 0) {
                fprintf(stderr, "Error in \"connect(sock, sockaddr_server, len)\"");
                exit(EXIT_FAILURE);
        }
        while (1) {
                char buffer[200];
                
                ssize_t nbytes = read(sockfd, buffer, sizeof buffer);
                buffer[nbytes] = '\0';
                puts(buffer);
        }
}
