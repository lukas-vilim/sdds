/* A simple server in the internet domain using TCP
	 The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

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

const char* TEXT_1 = "Text1";
const char* TEXT_2 = "Text2";
const char* TEXT_3 = "Text3";

unsigned int MakeStringMsg(char* buffer, unsigned id, const char* str)
{
	unsigned int msg, size;
	msg = 1;
	size = strlen(str);
	memcpy(buffer, &msg, 1);
	memcpy(buffer + 1, &id, 4);
	memcpy(buffer + 5, &size, 4);
	memcpy(buffer + 9, str, size);

	return 9 + size;
}

unsigned int MakeEntryMsg(char* buffer, Entry* entry)
{
	unsigned int id = 0, msg = 2;
	memcpy(buffer, &msg, 1);
	memcpy(buffer + 1, &id, 4);
	memcpy(buffer + 5, &entry->Id, 4);
	memcpy(buffer + 9, &entry->Float, 4);
	memcpy(buffer + 13, &entry->Int, 4);
	memcpy(buffer + 17, &entry->Bool, 1);
	memcpy(buffer + 18, &entry->Text, 4);

	return 22;
}

unsigned int MakeProtoHeader(char* buffer)
{
	unsigned int proto = 0xFEEDBEEFu;
	memcpy(buffer, &proto, 4);
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
		error("ERROR opening socket");
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	int portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
	{
		error("ERROR on binding");
	}

	listen(sockfd,5);
	socklen_t clilen = sizeof(cli_addr);
	int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	if (newsockfd < 0) 
	{
		error("ERROR on accept");
	}

	char buffer[256];
	bzero(buffer,256);

	// Strings.
	unsigned int size;
	size = MakeStringMsg(buffer, 0, TEXT_1);
	unsigned int n = write(newsockfd, buffer, size);
	if(n != 8 + size)
	{
		error("ERROR writing to socket str1");
	}
	
	size = MakeStringMsg(buffer, 1, TEXT_2);
	n = write(newsockfd, buffer, size);
	if(n != 8 + size)
	{
		error("ERROR writing to socket str2");
	}

	size = MakeStringMsg(buffer, 2, TEXT_3);
	n = write(newsockfd, buffer, size);
	if(n != 8 + size)
	{
		error("ERROR writing to socket str3");
	}

	size = MakeStringMsg(buffer, 3, "Id");
	write(newsockfd, buffer, size);
	size = MakeStringMsg(buffer, 4, "Int");
	write(newsockfd, buffer, size);
	size = MakeStringMsg(buffer, 5, "Float");
	write(newsockfd, buffer, size);
	size = MakeStringMsg(buffer, 6, "Bool");
	write(newsockfd, buffer, size);
	size = MakeStringMsg(buffer, 7, "Text");
	write(newsockfd, buffer, size);
	size = MakeStringMsg(buffer, 8, "Entry");
	write(newsockfd, buffer, size);

	// Entry descriptor
	unsigned int id = 0, entryName = 8;
	char numFields = 5;
	char msg = 3;
	memcpy(buffer, &msg, 1);
	memcpy(buffer + 1, &id, 4);
	memcpy(buffer + 5, &entryName, 4);
	memcpy(buffer + 9, &numFields, 1);

	unsigned int name = 3;
	buffer[10] = 1;
	memcpy(buffer + 11, &name, 4);

	name = 5;
	buffer[15] = 2;
	memcpy(buffer + 16, &name, 4);
	
	name = 4;
	buffer[20] = 1;
	memcpy(buffer + 21, &name, 4);

	name = 6;
	buffer[25] = 3;
	memcpy(buffer + 26, &name, 4);

	name = 7;
	buffer[30] = 4;
	memcpy(buffer + 31, &name, 4);

	write(newsockfd, buffer, 36);

	// Entries.
	for(int i = 0; i < 20; ++i)
	{
		MakeProtoHeader(buffer);
		Entry e;
		e.Id = i;
		e.Int = 7;
		e.Float = 7.0f;
		e.Bool = true;
		e.Text = i % 3;

		unsigned int size = MakeEntryMsg(buffer + 4, &e);
		n = write(newsockfd, buffer, size + 4);
		if(n != size + 4)
		{
			error("ERROR writing entries");
		}
	}

	close(newsockfd);
	close(sockfd);

	return 0; 
}
