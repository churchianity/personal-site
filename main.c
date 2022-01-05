#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(void) {
	FILE* fp = fopen("index.html", "rb");

	if (fp == NULL) {
	}

	// get the file's size in bytes so we know how much memory to allocate
	fseek(fp, 0, SEEK_END);
	int html_size = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	// allocate space for the file's data, fill the buffer with fread, null terminate it
	char* html = malloc(html_size + 1);
	fread(html, sizeof (char), html_size + 1, fp);
	html[html_size] = '\0';

	// close the file
	fclose(fp);

	// set up the web server
	struct sockaddr_in server_addr, client_addr;
	socklen_t sin_len = sizeof (client_addr);
	int fd_server, fd_client;
	char buffer[2048];
	int on = 1;

	// in linux, Berkeley Sockets are just integers, and are treated similarly to file descriptors
	fd_server = socket(AF_INET, SOCK_STREAM, 0);
	if (fd_server < 0) {
		exit(1);
	}

	setsockopt(fd_server, SOL_SOCKET, SO_REUSEADDR, &on, sizeof (int));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(80);

	if (bind(fd_server, (struct sockaddr*) &server_addr, sizeof (server_addr)) == -1) {
		close(fd_server);
		exit(1);
	}

	if (listen(fd_server, 10) == -1) {
		close(fd_server);
		exit(1);
	}

	while (1) {
		fd_client = accept(fd_server, (struct sockaddr*) &client_addr, &sin_len);

		if (fd_client == -1) {
			// connection failed ?? @TODO why?
			continue;
		}

		if (!fork()) {
			// child process
			close(fd_server);

			memset(buffer, 0, 2048);
			read(fd_client, buffer, 2047); // leaving room for NULL terminator
			// at this point, the buffer should contain a client request (or a chunk of it)
			
			write(fd_client, html, html_size - 1);
		}
		// parent process
		close(fd_client);
	}

	return 0;
}

