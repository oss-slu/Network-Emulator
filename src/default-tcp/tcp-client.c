#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

#define MAXDATASIZE 1515

int main(int argc, char** argv) {
  int localSocket = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in remoteAddr;
  memset((char*)&remoteAddr, 0, sizeof(remoteAddr));
  remoteAddr.sin_family = AF_INET;
  remoteAddr.sin_port = htons(15000);
  remoteAddr.sin_addr.s_addr = inet_addr(argv[1]); 

  connect(localSocket, (struct sockaddr*)&remoteAddr, sizeof(remoteAddr));
  printf("Connected to server\n");

  char send_buf[MAXDATASIZE];
  char recv_buf[MAXDATASIZE];

  struct pollfd ufds[2];
  ufds[0].fd = STDIN_FILENO;
  ufds[0].events = POLLIN; 

  ufds[1].fd = localSocket;
  ufds[1].events = POLLIN; 

  int rv;
  while (1) {
    rv = poll(ufds, 2, 10000);
    if (rv == -1) {
      perror("poll");
    } else if (rv == 0) {
      printf("No data after 10 seconds timeout");
    } else {
      if (ufds[0].revents & POLLIN) {
        if (fgets(send_buf, MAXDATASIZE - 1, stdin)) {
          int sent_bytes = send(localSocket, send_buf, strlen(send_buf), 0);
          if (sent_bytes < 0)
            perror("send");
          else if (sent_bytes != strlen(send_buf))
            printf("Partially sent\n");
        } else {
          send(localSocket, send_buf, 0, 0);
          break;
        }
      }

      if (ufds[1].revents & POLLIN) {
        int recv_bytes = recv(localSocket, recv_buf, MAXDATASIZE - 1, 0);
        if (recv_bytes > 0) {
          recv_buf[recv_bytes] = '\0';
          printf("%s", recv_buf);
        } else if (recv_bytes == 0) {
          printf("The other side has closed connection\n");
          break;
        }
      }
    }
  }

  close(localSocket);
  return 0;
}
