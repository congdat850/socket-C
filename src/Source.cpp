#include <stdio.h>
#include <WinSock2.h>
#include <windows.h>
#include <direct.h> // use _chdir function
#include <conio.h>

#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable : 4996)
// ReSharper disable CppCStyleCast

char buffer[2000]; //	buffer to send and receive data
char tp[500];	  //	store temporary character
int code;		   //	CODE NUMBER in FTP protocol
int bytes;		   //	indicate the number of bytes received from server
bool isActive = true;	//	active mode is on by default
char my_ip[50];

void errexit(const char *format, ...);
void recv_mess(SOCKET s, char *mess = buffer);
void send_mess(SOCKET s, char *mess = buffer);
void pause(void);
void sendLogIn(SOCKET s);
SOCKET sendPort(SOCKET s, int port_num = 0);
SOCKET sendPasv(SOCKET s);
void sendLs(SOCKET s);
void sendDir(SOCKET s);
void sendRmdir(SOCKET s);
void sendMkdir(SOCKET s);
void sendDelete(SOCKET s);
void sendCd(SOCKET s);
void sendPwd(SOCKET s);
void sendQuit(SOCKET s);
void sendGet(SOCKET s);
void sendPut(SOCKET s);

int main(int argc, char *argv[])
{
	//	Initialise Winsock
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa))
	{
		errexit("Can't initialise WinSock");
	}
	puts("WinSock initialised!!!");

	//	Create Socket
	SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
	if (s == INVALID_SOCKET)
	{
		errexit("Can't create socket");
	}
	puts("Socket created!!!");
	
	//	Connect to FTP server
	puts("Type IP of FTP server: ");
	char ser_ip[50];
	gets(ser_ip);
	puts("Type IP of Internet Interface you want to connect with server: ");
	gets(my_ip);

	sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(21);
	server.sin_addr.s_addr = inet_addr(ser_ip);
	if (connect(s, (sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
	{
		errexit("Can't connect to the server, please check again dude");
	}
	puts("Server connected!!!");

	//	Receive wellcome message
	puts("Welcome message from server:");
	recv_mess(s);
	printf("%s", buffer);
	puts("Press Enter to clean screen...");
	getchar();
	system("CLS");

	//	Log-in
	sendLogIn(s);
	puts("Press Enter to clean screen...");
	getchar();
	system("CLS");

	//	Enter working mode
	puts("You entered working mode, type in ur command (type ? to list all command)");
	while (1)
	{
		printf("\nftp> ");
		//	Get user commands
		char userInput[100], command[100];
		command[0] = '\0';
		tp[0] = '\0';
		gets(userInput);
		sscanf(userInput, "%s %[^\n]s", command, tp);

		//	Convert to lower character
		for (char &ch : command)
		{
			if (ch >= 'A' && ch <= 'Z')
				ch = ch - 'A' + 'a';
		}

		//	Handle all command here
		if (!strcmp(command, "?"))
		{
			printf(
				"List of all commands that being supported is:\nls, dir, rmdir, put, "
				"mput, get, mget, lcd, delete, mdelete, cd, pwd, passive, mkdir, quit, exit, ls_client\n");
		}
		else if (!strcmp(command, "ls"))
		{
			sendLs(s);
		}
		else if (!strcmp(command, "dir"))
		{
			sendDir(s);
		}
		else if (!strcmp(command, "rmdir"))
		{
			sendRmdir(s);
		}
		else if (!strcmp(command, "put"))
		{
			sendPut(s);
		}
		else if (!strcmp(command, "get"))
		{
			sendGet(s);
		}
		else if (!strcmp(command, "delete"))
		{
			sendDelete(s);
		}
		else if (!strcmp(command, "cd"))
		{
			sendCd(s);
		}
		else if (!strcmp(command, "pwd"))
		{
			sendPwd(s);
		}
		else if (!strcmp(command, "passive"))
		{
			isActive = !isActive;
			if (isActive)
			{
				puts("You are working on active mode");
			}
			else
			{
				puts("You are working on passive mode");
			}
		}
		else if (!strcmp(command, "mkdir"))
		{
			sendMkdir(s);
		}
		else if (!strcmp(command, "quit") || !strcmp(command, "exit"))
		{
			sendQuit(s);
		}
		else if (!strcmp(command, "lcd"))
		{
			if (tp[0] == '\0')
			{
				printf("Current working directory: ");
				system("cd");
			}
			else
			{
				_chdir(tp);
				printf("Current working directory changed: ");
				system("cd");
			}
		}
		else if (!strcmp(command, "ls_client"))
		{
			system("dir");
		}

		//	Split names of many files, then call function
		else if (!strcmp(command, "mget") || !strcmp(command, "mput") ||
				 !strcmp(command, "mdelete"))
		{
			char names[100]; //	Store name of many files
			int len = 0;
			sprintf(names, "%s ", tp);
			for (int i = 0; i < strlen(names); ++i)
			{
				char ch = names[i];
				if (ch == ' ')
				{
					tp[len] = 0;
					len = 0;
					printf("File %s:\n", tp);
					if (!strcmp(command, "mget"))
					{
						sendGet(s);
					}
					else if (!strcmp(command, "mput"))
					{
						sendPut(s);
					}
					else
						sendDelete(s);
					printf("\n");
				}
				else
					tp[len++] = ch;
			}
		}
		else
		{
			puts("Ur command haven't been supported just yet :3, type ? for help dude");
		}
	}
}

//	Print error
void errexit(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	WSACleanup();
	pause();
	exit(1);
}

void pause(void)
{
	char c;
	printf("Press Enter to continue\n");
	scanf("%c", &c);
}

//	Receive message from socket s
void recv_mess(SOCKET s, char *mess)
{
	bytes = recv(s, mess, 2000, 0);
	if (bytes == SOCKET_ERROR)
	{
		errexit("Sever closed connection\n");
	}
	mess[bytes] = '\0';
}

//	Send message to socket s
void send_mess(SOCKET s, char *mess)
{
	if (send(s, mess, strlen(mess), 0) == SOCKET_ERROR)
	{
		errexit("Can't send mess\n");
	}
}

//	Send log in info to socket s
void sendLogIn(SOCKET s)
{
	//	Send User
	printf("User name: ");
	gets(tp);
	sprintf(buffer, "USER %s\r\n\0", tp);
	send_mess(s);
	recv_mess(s);
	printf("%s", buffer);
	sscanf(buffer, "%d", &code);
	if (code != 331)
	{
		errexit("Error occur\n");
	}
	//	Send Password and check
	puts("Type password: ");
	while (1)
	{
		//	Read password
		int i = 0;
		char ch;
		const char ENTER = 13;
		while ((ch = _getch()) != ENTER)
		{
			tp[i++] = ch;
		}
		tp[i] = '\0';
		printf("\n");
		sprintf(buffer, "PASS %s\r\n\0", tp);
		send_mess(s);
		recv_mess(s);
		printf("%s", buffer);
		sscanf(buffer, "%d", &code);
		//	Right password
		if (code == 230)
		{
			break;
		}
		//	Wrong password
		printf("Type password again: ");
	}
}

void sendRmdir(SOCKET s)
{
	sprintf(buffer, "XRMD %s\r\n\0", tp);
	send_mess(s);
	recv_mess(s);
	printf("%s", buffer);
}

void sendMkdir(SOCKET s)
{
	sprintf(buffer, "XMKD %s\r\n\0", tp);
	send_mess(s);
	recv_mess(s);
	printf("%s", buffer);
}

void sendCd(SOCKET s)
{
	sprintf(buffer, "CWD %s\r\n\0", tp);
	send_mess(s);
	recv_mess(s);
	printf("%s", buffer);
}

void sendDelete(SOCKET s)
{
	sprintf(buffer, "DELE %s\r\n\0", tp);
	send_mess(s);
	recv_mess(s);
	printf("%s", buffer);
}

void sendPwd(SOCKET s)
{
	sprintf(buffer, "XPWD \r\n\0");
	send_mess(s);
	recv_mess(s);
	printf("%s", buffer);
}

void sendQuit(SOCKET s)
{
	sprintf(buffer, "QUIT \r\n\0", tp);
	send_mess(s);
	recv_mess(s);
	printf("%sProgram exit sucessfully ^^...!\n\n", buffer);
	getchar();
	exit(0);
}

//	Open port and send info to socket s
SOCKET sendPort(SOCKET s, int port_num)
{
	//	Create Socket to bind
	SOCKET s2 = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in me;
	me.sin_family = AF_INET;
	//char *my_ip = "127.0.0.1";
	me.sin_addr.s_addr = inet_addr(my_ip); //My IP
	me.sin_port = htons(port_num);				 //My port
	bind(s2, (sockaddr *)&me, sizeof(me));
	listen(s2, 3);

	//	If port num havn't been specified, let the system decide
	if (port_num == 0)
	{
		struct sockaddr_in srv;
		int len = sizeof(srv);
		getsockname(s2, (struct sockaddr *)&srv, &len);
		port_num = ntohs(srv.sin_port);
	}
	sprintf(buffer, "PORT %d,%d,%d,%d,%d,%d\r\n\0", me.sin_addr.s_net, me.sin_addr.s_host,
			me.sin_addr.s_lh, me.sin_addr.s_impno, (port_num & 65280) >> 8, port_num & 255);
	send_mess(s);
	recv_mess(s);
	printf("%s", buffer);
	return s2;
}

//	Send PASV command, receive port number then connect to server
SOCKET sendPasv(SOCKET s)
{
	char ip_addr[100];
	strcpy(buffer, "PASV\r\n\0");
	send_mess(s);
	recv_mess(s);
	printf("%s", buffer);
	int ip[4], port[2];
	sscanf(buffer, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)",
		   ip, ip + 1, ip + 2, ip + 3, port, port + 1);
	sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = port[0] * 1 + port[1] * 256;
	sprintf(ip_addr, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
	server.sin_addr.s_addr = inet_addr(ip_addr);
	SOCKET s2 = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(s2, (sockaddr *)&server, sizeof(server)))
	{
		errexit("Can't connect to the serever @@\n");
	}
	return s2;
}

void sendLs(SOCKET s)
{
	SOCKET s3;

	//	On active mode
	if (isActive)
	{
		SOCKET s2 = sendPort(s);
		sockaddr_in server;

		//	Send command and received 150 respond
		sprintf(buffer, "NLST\r\n\0");
		send_mess(s);
		recv_mess(s);
		printf("%s", buffer);
		int len = sizeof(server);
		s3 = accept(s2, (sockaddr *)&server, &len);
		closesocket(s2);
	}
	//	On passive mode
	else
	{
		s3 = sendPasv(s);
		sprintf(buffer, "NLST\r\n\0");
		send_mess(s);
		recv_mess(s);
		printf("%s", buffer);
	}
	recv_mess(s3);
	printf("%s", buffer);
	closesocket(s3);
	recv_mess(s);
	printf("%s", buffer);
}

void sendDir(SOCKET s)
{
	SOCKET s3;
	//	On active mode
	if (isActive)
	{
		SOCKET s2 = sendPort(s);
		sockaddr_in server;

		//	Send command and received 150 respond
		sprintf(buffer, "LIST\r\n\0");
		send_mess(s);
		recv_mess(s);
		printf("%s", buffer);
		int len = sizeof(server);
		s3 = accept(s2, (sockaddr *)&server, &len);
		closesocket(s2);
	}
	//	On passive mode
	else
	{
		//
		s3 = sendPasv(s);
		sprintf(buffer, "LIST\r\n\0");
		send_mess(s);
		recv_mess(s);
		printf("%s", buffer);
	}
	recv_mess(s3);
	printf("%s", buffer);

	closesocket(s3);
	recv_mess(s);
	printf("%s", buffer);
}

void sendGet(SOCKET s)
{
	SOCKET s3;
	//	On active mode
	if (isActive)
	{
		SOCKET s2 = sendPort(s);
		sockaddr_in server;

		//	Send command and received 150 respond
		sprintf(buffer, "RETR %s\r\n\0", tp);
		send_mess(s);
		recv_mess(s);
		printf("%s", buffer);

		//	File not exists in server
		sscanf(buffer, "%d", &code);
		if (code == 550)
		{
			return;
		}
		int len = sizeof(server);
		s3 = accept(s2, (sockaddr *)&server, &len);
		closesocket(s2);
	}
	//	On passive mode
	else
	{
		s3 = sendPasv(s);
		sprintf(buffer, "RETR %s\r\n\0", tp);
		send_mess(s);
		recv_mess(s);
		printf("%s", buffer);
		//	File not exists in server
		sscanf(buffer, "%d", &code);
		if (code == 550)
		{
			closesocket(s3);
			return;
		}
	}
	//	Open file to write
	FILE *fp = fopen(tp, "wb");
	if (!fp)
	{
		puts("Some err occur(maybe you don't have permission to write file)");
		fclose(fp);
		closesocket(s3);
		return;
	}

	//	Read to buffer 1000 bytes at once
	while ((bytes = recv(s3, buffer, 1000, 0)) > 0)
	{
		fwrite(buffer, 1, bytes, fp);
	}

	//	Finish transport file
	fclose(fp);
	closesocket(s3);
	recv_mess(s);
	printf("%s", buffer);
}

void sendPut(SOCKET s)
{
	//	Open file to read
	FILE *fp = fopen(tp, "rb");
	if (!fp)
	{
		puts("Some err occur(maybe you don't have permission to read file, or file not exists)");
		return;
	}

	SOCKET s3;

	//	Work in active mode
	if (isActive)
	{
		SOCKET s2 = sendPort(s);
		sockaddr_in server;

		//	Send PORT command and received 150 respond
		sprintf(buffer, "STOR %s\r\n\0", tp);
		send_mess(s);
		recv_mess(s);
		printf("%s", buffer);
		int len = sizeof(server);
		s3 = accept(s2, (sockaddr *)&server, &len);
		closesocket(s2);
	}

	//	Work in passive mode
	else
	{
		s3 = sendPasv(s);
		sprintf(buffer, "STOR %s\r\n\0", tp);
		send_mess(s);
		recv_mess(s);
		printf("%s", buffer);
	}

	//	Find out the size of file
	fseek(fp, 0, SEEK_END);
	int fileSize = ftell(fp);
	rewind(fp);
	//	Send the file 1000 bytes at once
	for (int i = 0; i < fileSize / 1000; ++i)
	{
		fread(buffer, 1, 1000, fp);
		send(s3, buffer, 1000, 0);
	}
	int left_over = fileSize - fileSize / 1000 * 1000;
	fread(buffer, 1, left_over, fp);
	send(s3, buffer, left_over, 0);

	//	Finish transport file
	fclose(fp);
	closesocket(s3);
	recv_mess(s);
	printf("%s", buffer);
}
