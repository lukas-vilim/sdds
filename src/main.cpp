/* A simple server in the internet domain using TCP
	 The port number is passed as an argument */
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h> 
#include <unistd.h>

void error(const char *msg)
{
	perror(msg);
	exit(1);
}

struct Entry
{
	int Id;
	float Float;
	int Int;
	bool Bool;
	unsigned int Text;
};

const char* STRING_TABLE[9] = 
{
	"Text1",	// = 0
	"Text2",	// = 1
	"Text3",	// = 2
	"Id",			// = 3
	"Int",		// = 4
	"Float",	// = 5
	"Bool",		// = 6
	"Text",		// = 7
	"Entry"		// = 8
};

unsigned int MakeProtoHeader(char* buffer)
{
	unsigned int proto = 0xFEEDBEEFu;
	memcpy(buffer, &proto, 4);
}

unsigned int MakeStringMsg(char* buffer, unsigned id, const char* str)
{
	MakeProtoHeader(buffer);
	buffer = buffer + 4;

	unsigned int size;
	unsigned char msg = 1;
	size = strlen(str);
	memcpy(buffer, &msg, 1);
	memcpy(buffer + 1, &id, 4);
	memcpy(buffer + 5, &size, 4);
	memcpy(buffer + 9, str, size);

	return 13 + size;
}

unsigned int MakeEntryMsg(char* buffer, Entry* entry)
{
	MakeProtoHeader(buffer);
	buffer = buffer + 4;

	unsigned int id = 0, msg = 2;
	memcpy(buffer, &msg, 1);
	memcpy(buffer + 1, &id, 4);
	memcpy(buffer + 5, &entry->Id, 4);
	memcpy(buffer + 9, &entry->Float, 4);
	memcpy(buffer + 13, &entry->Int, 4);
	memcpy(buffer + 17, &entry->Bool, 1);
	memcpy(buffer + 18, &entry->Text, 4);

	return 26;
}

int main(int argc, char *argv[])
{
	struct sockaddr_in serv_addr, cli_addr;
	if (argc < 2) 
	{
		fprintf(stderr,"ERROR, no port provided\n");
		exit(1);
	}

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
	{
		close(sockfd);
		error("ERROR opening socket");
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	int portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
	{
		close(sockfd);
		error("ERROR on binding");
	}

	listen(sockfd,5);
	socklen_t clilen = sizeof(cli_addr);
	int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	if (newsockfd < 0) 
	{
		close(sockfd);
		error("ERROR on accept");
	}

	char buffer[256];
	bzero(buffer,256);


	// Strings.
	unsigned int size;
	for(int i = 0; i < 9; ++i)
	{
		unsigned int size = MakeStringMsg(buffer, i, STRING_TABLE[i]);
		unsigned int n = write(newsockfd, buffer, size);
		if(n != size)
		{
			close(newsockfd);
			close(sockfd);
			error("ERROR writing string msg to socket");
		}
	}

	// Entry descriptor
	{
		unsigned int id = 0, entryName = 8;
		char numFields = 5;
		char msg = 3;
		char* buf_ptr = buffer;

		MakeProtoHeader(buf_ptr);
		buf_ptr += 4;

		memcpy(buf_ptr, &msg, 1);
		memcpy(buf_ptr + 1, &id, 4);
		memcpy(buf_ptr + 5, &entryName, 4);
		memcpy(buf_ptr + 9, &numFields, 1);

		buf_ptr += 10;

		unsigned int name = 3;

		buf_ptr[0] = 1;
		memcpy(buf_ptr + 1, &name, 4);
		buf_ptr += 5;

		name = 5;
		buf_ptr[0] = 2;
		memcpy(buf_ptr + 1, &name, 4);
		buf_ptr += 5;

		name = 4;
		buf_ptr[0] = 1;
		memcpy(buf_ptr + 1, &name, 4);
		buf_ptr += 5;

		name = 6;
		buf_ptr[0] = 3;
		memcpy(buf_ptr + 1, &name, 4);
		buf_ptr += 5;

		name = 7;
		buf_ptr[0] = 4;
		memcpy(buf_ptr + 1, &name, 4);
		buf_ptr += 5;

		write(newsockfd, buffer, uintptr_t(buf_ptr) - uintptr_t(buffer));
	}

	// Entries.
	for(int i = 0; i < 20; ++i)
	{
		Entry e;
		e.Id = i;
		e.Int = 7;
		e.Float = 7.0f;
		e.Bool = true;
		e.Text = i % 3;

		unsigned int size = MakeEntryMsg(buffer, &e);
		unsigned int n = write(newsockfd, buffer, size);
		if(n != size)
		{
			close(newsockfd);
			close(sockfd);
			error("ERROR writing entries");
		}
	}

	close(newsockfd);
	close(sockfd);

	return 0; 
}
