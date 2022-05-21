
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

char* file_paths[] = {
	"index.html",

	"hexyz/index.html",
	"hexyz/amulet.js",
	"hexyz/amulet.wasm",
	"hexyz/data.pak",
	"hexyz/amulet_license.txt",
};
#define FILEPATH_LENGTH(array) (sizeof(array)/sizeof(array[0]))
#define FPL FILEPATH_LENGTH(file_paths)
char* file_contents[FPL];
int file_lengths[FPL];

char* load_file_and_cache_size(int i) {
	FILE* fp = fopen(file_paths[i], "rb");

	if (fp == NULL) {
		printf("failed to load %s\n", file_paths[i]);
		exit(1);
	}

	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	char* buffer = malloc(size + 1);
	fread(buffer, sizeof (char), size + 1, fp);
	buffer[size] = '\0';

	file_lengths[i] = size;

	fclose(fp);

	return buffer;
}

void load_servable_file_contents() {
	for (int i = 0; i < FPL; i++) {
		file_contents[i] = load_file_and_cache_size(i);
	}
}

// we only care about the path and maybe method
char* parse_http(char* buffer, int size) {
	// METHOD PATH HTTP-VERSION
	int i = 0;

	char* cursor = buffer;
	char* method = cursor;
	int method_length = 0;
	for (; i < size; i++) {
		if (buffer[i] == ' ') {
			while (buffer[++i] == ' ') {}
			break;
		}

		method_length++;
	}

	cursor = buffer + i;
	char* path = cursor;
	int path_length = 0;
	for (; i < size; i++) {
		if (buffer[i] == ' ') {
			while (buffer[++i] == ' ') {}
			break;
		}

		path_length++;
	}

	if (method_length == 3 && (strncmp(method, "GET", 3) == 0)) {
		char* string = malloc(sizeof(char) * (path_length - 1));

		if (path[0] == '/') {
			if (path_length == 1) {
				char* index = malloc(sizeof("index.html\0"));
				memcpy(index, "index.html\0", sizeof("index.html\0"));
				return index;

			} else {
				memcpy(string, path + 1, path_length - 1);
				string[path_length - 1] = '\0';
			}
		} else {
			return NULL;
		}

		return string;
	}

	return NULL;
}

int main(void) {
	load_servable_file_contents();

	struct sockaddr_in server_addr, client_addr;
	socklen_t sin_len = sizeof (client_addr);
	int fd_server, fd_client;
	char buffer[2048];
	int on = 1;

	fd_server = socket(AF_INET, SOCK_STREAM, 0);
	if (fd_server < 0) {
		printf("failed to create socket\n");
		exit(1);
	}

	setsockopt(fd_server, SOL_SOCKET, SO_REUSEADDR, &on, sizeof (int));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(80);

	if (bind(fd_server, (struct sockaddr*) &server_addr, sizeof (server_addr)) == -1) {
		printf("failed to bind to server to server_addr\n");
		close(fd_server);
		exit(1);
	}

	if (listen(fd_server, 10) == -1) {
		printf("failed to listen to server\n");
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

			char* path = parse_http(buffer, 2048);


			if (path != NULL) {
				for (int i = 0; i < FPL; i++) {
					if (strcmp(path, file_paths[i]) == 0) {
						printf("|%s|\n", file_paths[i]);
						write(fd_client, file_contents[i], file_lengths[i]);
						break;
					}
				}
				free(path);
			}
		}
		// parent process
		close(fd_client);
	}

	return 0;
}

