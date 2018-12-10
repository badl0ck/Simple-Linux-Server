/*
  Простой сервер, создаваемый сугубо в научно-исследовательских целях.

  При подключении запрашиваются логин (login) и пароль (password). Вводимые клиентом и присылаемые на сервер
  данные ищутся в файле паролей Linux, после подтверждения осуществляется вход.

  Во время сеанса возможен ввод команд:
  ls - просмотр каталогов и файлов в текущем каталоге
  cd <path> - переход к каталогу
  rm - удаление файла или каталога

  Сервер работает непрерывно (это демон).
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

struct addrinfo min_info_strc; // addrinfo with minimum of information
struct addrinfo *full_info_strc; // addrinfo which will be filled by getaddrinfo()

int sck, new_sck; // my and client socket's address

struct sockaddr_storage their_addr; // info about incoming connection will go here

socklen_t addr_size; // address size

// extended getaddrinfo() with memset and error checking
void e_gai()
{
  int status; // code which will be returned by getaddrinfo()

  memset(&min_info_strc, 0, sizeof min_info_strc); // fill min addrinfo by zeros

  min_info_strc.ai_family = AF_INET; // ipv4 and ipv6
  min_info_strc.ai_socktype = SOCK_STREAM; // tcp
  min_info_strc.ai_flags = AI_PASSIVE; // localhost

  status = getaddrinfo(NULL, "9999", &min_info_strc, &full_info_strc);
  if (status != 0) // 0 is correct
  {
    fprintf(stderr, "e_gai() error: %s\n", gai_strerror(status)); // print error in console
    exit(1);
  }
}

// extended socket() with sck value assignment and error checking
void e_socket(int domain, int type, int protocol)
{
  sck = socket(domain, type, protocol);

  if (sck == -1) // -1 is error
  {
    fprintf(stderr, "e_socket() error: %s\n", gai_strerror(sck));
    exit(1);
  }
}

// extended bind() with error checking
void e_bind(int socket_address, struct sockaddr *my_addr, int addrlen)
{
  if (bind(socket_address, my_addr, addrlen) == -1) // -1 is error
  {
    fprintf(stderr, "e_bind() error: %s\n", gai_strerror(-1));
    exit(1);
  }
}

// extended listen() with error checking
// socket_address is sock file descr
// count_of_connections - maximum allowed connections
void e_listen(int socket_address, int count_of_connections)
{
  if (listen(socket_address, count_of_connections) == -1) // -1 is error
  {
    fprintf(stderr, "e_listen() error: %s\n", gai_strerror(-1));
    exit(1);
  }
}

void authorisation()
{
  char *send_msg_greet = "proftpd clone 0.1beta\n";
  send(new_sck, send_msg_greet, strlen(send_msg_greet), 0);

  char *send_msg_login_request = "Login: ";
  send(new_sck, send_msg_login_request, strlen(send_msg_login_request), 0);

  char recieve_login_buffer[20];
  recv(new_sck, &recieve_login_buffer, sizeof recieve_login_buffer, 0);
  printf("Login entered: %s\n", recieve_login_buffer);
  memset(&recieve_login_buffer, 0, sizeof recieve_login_buffer);

  char *send_msg_password_request = "Password: ";
  send(new_sck, send_msg_password_request, strlen(send_msg_password_request), 0);

  char recieve_password_buffer[20];
  recv(new_sck, &recieve_password_buffer, sizeof recieve_password_buffer, 0);
  printf("Password entered: %s\n", recieve_password_buffer);
  memset(&recieve_password_buffer, 0, sizeof recieve_password_buffer);
}

// extended accept() with addr_size initialization (size of client's address), client socket's creation and error checking
void e_accept(int socket_address)
{
  // interaction with client is here

  while(1)
  {
    addr_size = sizeof their_addr; // initializing their address's size

    new_sck = accept(socket_address, (struct sockaddr *)&their_addr, &addr_size); // my socket's adress, their sockaddr, their address's size

    if (new_sck == -1) // error checking
    {
      fprintf(stderr, "e_accept() error: %s\n", gai_strerror(-1));
      exit(1);
    }

    authorisation();
  }
}

// entry point of programm
int main(int argc, char *argv[])
{
  e_gai();
  e_socket(full_info_strc->ai_family, full_info_strc->ai_socktype, full_info_strc->ai_protocol);
  e_bind(sck, full_info_strc->ai_addr, full_info_strc->ai_addrlen);
  e_listen(sck, 20);
  e_accept(sck);

  freeaddrinfo(full_info_strc);
}
