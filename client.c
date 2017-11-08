#include <stdio.h>
#include <string.h>
#include <time.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>

#define KB 1024
#define HTTP_PORT "80"
#define FLAGGED 255
#define HTTP_PAT "http://"
#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"


int check_input(char* input, int minus, int dot);
void input_err(int flag);
void toLower(char* str, int c);


int main ( int argc, char **argv )
{
	if(argc<2)
	{
		fprintf(stderr, "Error - not enough arguments, please insert at least the URL parameter\n");
		return -1;
	}
	int i;
	int minus_h = 0;
	int minus_d = 0;
	int d=0;
	int h=0;
	int m=0;
	char* host = 0;
	char* port = 0;
	char* path = 0;
	struct in_addr **addr_list = NULL;
	struct hostent *he = NULL;
	for(i=1; i<argc ;i++) // reads ALL the parameters
	{
		
		if(strlen(argv[i]) >= strlen(HTTP_PAT)) // first case is URL, which HAS to include at least the HTTP pattern
		{
			toLower(argv[i],(int) strlen(HTTP_PAT)); // converts the first part of the URL to lower case, if at all
			if(strstr(argv[i],HTTP_PAT)) // checks for http:// pattern existance
			{
				char* temp = strstr(argv[i],HTTP_PAT); // grabs the first http:// pattern
				if(temp != argv[i]) // verifies it's the start of the string
					input_err(0);
				temp+=strlen(HTTP_PAT); // jumps to the start of the host
				if(strchr(temp, (int) ':')) // case of finding char ':'
				{
					if(strchr(temp, (int) '/') != NULL && strchr(temp, (int) '/') < strchr(temp, (int) ':')) // case of ':' is not to signal the port but it's part of the path
					{
							char* temp2 = strchr(temp, (int) '/'); // grabs the path signal mark
							if(!(temp2-temp)/sizeof(char)) // obvious
								input_err(0);
							host = (char*) malloc (sizeof(char) * ((temp2-temp)/sizeof(char)+1)); // allocates host
							if(!host) // verify allocation
								input_err(2);
							strncpy(host,temp,(temp2-temp)/sizeof(char)); // copies the host 
							host[(temp2-temp)/sizeof(char)]='\0'; // finishes string
							if(!strlen(temp2)) // obvious
							{
								if(host)
									free(host);
								if(port)
									free(port);
								if(path)
									free(path);
								input_err(0);
							}
							path = (char*) malloc (sizeof(char) * (strlen(temp2)+1));
							if(!path)
							{	
								if(host)
									free(host);
								if(port)
									free(port);
								if(path)
									free(path);
								input_err(2);
							}
							strcpy(path,temp2);
							path[strlen(temp2)]='\0';
					}
					else // case normal: http://prefix.host.suffix:port+?
					{	
						char* temp2 = strchr(temp, (int) ':'); // grabs the long foretold ':' char
						if(temp2 == temp+1) // verifies input is not http://:
							input_err(0);
						if(!(temp2-temp)/sizeof(char))
							input_err(0);
						host = (char*) malloc (sizeof(char) * ((temp2-temp)/sizeof(char) + 1)); // allocates host
						if(!host) // verifies allocation
							input_err(2);
						strncpy(host,temp,(temp2-temp)/sizeof(char)); // copies the host
						host[(temp2-temp)/sizeof(char)] = '\0'; // ends the string
					//	temp++; // continue ,possibily pointless call;
						if(strchr(temp2, (int) '/')) // case http://prefix.host.suffix:port/path
						{
							char* temp3 = strchr(temp2, (int) '/'); // grabs the path
							if(temp3 == temp2 + 1) // verifies no input implentation of http://prefix.host.suffix:/path
							{	
								if(host)
									free(host);
								if(port)
									free(port);
								if(path)
									free(path);
								input_err(0);
							}
							temp2++; // continues to copy the numbers instead of the ':' char
							if(!(temp3-temp2)/sizeof(char)) // obvious
							{
								if(host)
									free(host);
								if(port)
									free(port);
								if(path)
									free(path);
								input_err(0);
							}
							port = (char*) malloc (sizeof(char) * (((temp3-temp2)/sizeof(char))+1)); // allocates
							if(!port) // verifies allocation
							{
								if(host)
									free(host);
								if(port)
									free(port);
								if(path)
									free(path);
								input_err(2);
							}
							strncpy(port, temp2, (temp3-temp2)/sizeof(char)); // copies port
							port[(temp3-temp2)/sizeof(char)] = '\0'; // terminates port
							if(!strlen(temp3)) // obvious
							{
								if(host)
									free(host);
								if(port)
									free(port);
								if(path)
									free(path);
								input_err(0);
							}
							path=(char*) malloc (sizeof(char) * (strlen(temp3)+1)); // allocates
							if(!path) // verifies input
							{	
								if(host)
									free(host);
								if(port)
									free(port);
								if(path)
									free(path);
								input_err(2);
							}
							strcpy(path, temp3); // copies path
							path[strlen(temp3)] = '\0'; // terminates path
						}
						else // case http://prefix.host.suffix:port
						{
							temp2++; // steps to copy numbers
							if(!strlen(temp2)) // obvious
							{
								if(host)
									free(host);
								if(port)
									free(port);
								if(path)
									free(path);
								input_err(0);
							}
							port = (char*) malloc (sizeof(char) * (strlen(temp2)+1)); // allocates
							if(!port) // verifies allocatoin
							{	
								if(host)
									free(host);
								if(port)
									free(port);
								if(path)
									free(path);
								input_err(2);
							}
							strcpy(port,temp2); // copies string
							port[strlen(temp2)] = '\0'; // terminates
						}
					}
				}
				else if(strchr(temp, (int) '/')) // case http://prefix.host.suffix/path
				{
					char* temp2 = strchr(temp, (int) '/'); // grabs aforementioned pointer
					if(!(temp2-temp)/sizeof(char)) // obvious
						input_err(0);
					host = (char*) malloc (sizeof(char) * ((temp2-temp)/sizeof(char)+1)); // allocates
					if(!host) // verifies allocation
						input_err(2);
					strncpy(host, temp, (temp2-temp)/sizeof(char)); // copies string
					host[(temp2-temp)/sizeof(char)] = '\0'; // terminates
					if(!strlen(temp2)) // obvious
					{
						if(host)
							free(host);
						if(port)
							free(port);
						if(path)
							free(path);
						input_err(0);
					}
					path = (char*) malloc (sizeof(char) * (strlen(temp2)+1)); // allocates
					if(!path) // verifies allocation
					{	
						if(host)
							free(host);
						if(port)
							free(port);
						if(path)
							free(path);
						input_err(2);
					}
					strcpy(path,temp2); // copies
					path[strlen(temp2)]= '\0'; // terminates
				}
				else // probably case http://prefix.host.suffix
				{
					if(!strlen(temp)) // obvious
						input_err(0);
					host = (char*) malloc (sizeof(char) * (strlen(temp) + 1)); // allocates
					if(!host) // verifies allocation
						input_err(2);
					strcpy(host,temp); // copies string
					host[strlen(temp)]='\0'; // terminates
				}
			}
			else // case of a bigger input than 2 but not an HTTP pattern, therefore illegal input
				input_err(1);
		}
		else if(strchr(argv[i],(int) '-')) // second case is the minus parameter (-h,-d)
		{
			char* minus = strchr(argv[i],(int) '-'); // grabs minus pointer
			if(minus != argv[i] || strlen(minus)!=2) // should be at the size of 2 (-'LETTER') and should be the start of the string
			{
				if(host)
					free(host);
				if(port)
					free(port);
				if(path)
					free(path);
				input_err(1);
			}
			if(minus[1] == 'd') // case -d, can check at [1] because of previous check
			{
				if(!(i+1 < argc)) // checks that -d is not the last parameter, which denies the option of time parameter
				{	
					if(host)
						free(host);
					if(port)
						free(port);
					if(path)
						free(path);
					input_err(0);
				}
				if(minus_d) // too many -d's?
				{	
					if(host)
						free(host);
					if(port)
						free(port);
					if(path)
						free(path);
					input_err(0);
				}
				minus_d = FLAGGED;
				char* day;
				char* hour;
				char* minute;
				i++; // I already checked I could go there... which HAS to hold the parameters of %d:%d:%d
				if(strstr(argv[i],HTTP_PAT)) // possibily?
				{
					if(host)
						free(host);
					if(port)
						free(port);
					if(path)
						free(path);
					input_err(0);
				}
				if(!strchr(argv[i],(int) ':')) // checks that the first ':' exists
				{	
					if(host)
						free(host);
					if(port)
						free(port);
					if(path)
						free(path);
					input_err(0);
				}
				char* temp = strchr(argv[i],(int) ':'); // grabs the first ':' pointer 
				if(temp == argv[i]) // verifies case :%d:%d or ::%d or :: doesn't exist 
				{	
					if(host)
						free(host);
					if(port)
						free(port);
					if(path)
						free(path);
					input_err(0);
				}
				if(!(temp-argv[i])/sizeof(char)) // obvious
				{
					if(host)
						free(host);
					if(port)
						free(port);
					if(path)
						free(path);
					input_err(0);
				}
				day = (char*) malloc (sizeof(char) * ((temp-argv[i])/sizeof(char) + 1)); // allocates
				if(!day) // verifies allocation
				{
					if(host)
						free(host);
					if(port)
						free(port);
					if(path)
						free(path);
					input_err(2);
				}
				strncpy(day,argv[i], (temp-argv[i])/sizeof(char)); // copies string
				day[(temp-argv[i])/sizeof(char)] = '\0'; // terminates
				temp++; // continue
				if(!strchr(temp,(int) ':')) // checks if ':' exists
				{	
					if(day)
						free(day);
					if(host)
						free(host);
					if(port)
						free(port);
					if(path)
						free(path);
					input_err(0);
				}
				char* temp2 = strchr(temp,(int) ':'); //grabs next ':'
				if(!(temp2-temp)/sizeof(char)) // obvious
				{
					if(day)
						free(day);
					if(host)
						free(host);
					if(port)
						free(port);
					if(path)
						free(path);					
					input_err(0);
				}
				hour = (char*) malloc (sizeof(char) * ((temp2-temp)/sizeof(char)+1)); // allocates
				if(!hour) // verifies allocation
				{
					if(day)
						free(day);
					if(host)
						free(host);
					if(port)
						free(port);
					if(path)
						free(path);
					input_err(2);
				}
				strncpy(hour,temp, (temp2-temp)/sizeof(char)); // copies
				hour[(temp2-temp)/sizeof(char)] = '\0'; // terminates
				temp2++; // continue
				if(!strlen(temp2)) // obvious
				{
					if(day)
						free(day);
					if(hour)
						free(hour);
					if(host)
						free(host);
					if(port)
						free(port);
					if(path)
						free(path);
					input_err(0);
				}
				minute = (char*) malloc (sizeof(char) * (strlen(temp2)+1)); // allocates
				if(!minute) // verifies allocation
				{
					if(day)
						free(day);
					if(hour)
						free(hour);
					if(host)
						free(host);
					if(port)
						free(port);
					if(path)
						free(path);
					input_err(2);
				}
				strcpy(minute,temp2); // copies
				minute[strlen(temp2)] = '\0'; // terminates
				if(strlen(day) == 0 || strlen(hour) == 0 || strlen(minute) == 0) // verifies there's input
				{
					if(day)
						free(day);
					if(hour)
						free(hour);
					if(minute)
						free(minute);
					if(host)
						free(host);
					if(port)
						free(port);
					if(path)
						free(path);
					input_err(0);
				}
				if(!check_input(day,1,0) ||  !check_input(hour,1,0) || !check_input(minute,1,0)) // checks for inputs to have only one minus but only numbers
				{
					if(day)
						free(day);
					if(hour)
						free(hour);
					if(minute)
						free(minute);
					if(host)
						free(host);
					if(port)
						free(port);
					if(path)
						free(path);
					input_err(1);
				}
				d = atoi(day); // converts after input check
				h = atoi(hour);
				m = atoi(minute);
				free(day); // frees
				free(hour);
				free(minute);
			}
			else if(minus[1] == 'h')
			{
				if(minus_h) // too many -h's?
				{
					if(host)
						free(host);
					if(port)
						free(port);
					if(path)
						free(path);
					input_err(0);
				}
				minus_h = FLAGGED;
			}
			else // case -'~anything else than h or d'
			{
			if(host)
				free(host);
			if(port)
				free(port);
			if(path)
				free(path);
			input_err(0);
			}
		}
		else // case of neither "-" nor "http://"
		{
			if(host)
				free(host);
			if(port)
				free(port);
			if(path)
				free(path);
			input_err(1);
		}
	}
	if(!host) // NO HOST?!
	{
		if(port)
			free(port);
		if(path)
			free(path);
		input_err(0);
	}
	int socket_fd;
	struct sockaddr_in serv_address;
	socklen_t addr_size;
	socket_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP); // creates the socket
	if(socket_fd == -1)
	{
		free(host);
		if(port)
			free(port);
		if(path)
			free(path);	
		perror("socket()\n");
		return -1;
	}
	serv_address.sin_family = AF_INET; // IPv4
	if(!port) 
	{
		char* temp = HTTP_PORT;
		serv_address.sin_port = htons(atoi(temp)); // on default port is 80
	}
	else
	{
		int c = check_input(port,0,0); // verifies input with port
		if(!c || !strlen(port) || atoi(port) > 65535 || atoi(port) < 0)
		{
			free(host);
			if(port)
				free(port);
			if(path)
				free(path);	
			input_err(1);
		}
		serv_address.sin_port = htons(atoi(port)); // input port
	}
	if(!strlen(host))
	{
		free(host);
		if(port)
			free(port);
		if(path)
			free(path);
		return -1;
	}
	int www_flag=check_input(host,0,1); // is IP address or host?
	if(!www_flag) // case of host having need for DNS
	{
		int k=5; // tries to get hostname
		he=NULL;
		while(he==NULL && k!=0) // few tries to grab the host
		{
			he = gethostbyname(host);
			k--;
			if(!he)
				break;
		}
		if(!he)
		{
			free(host);
			if(port)
				free(port);
			if(path)
				free(path);
			herror("Error grabbing hostname\n");
			return -1;
		}
		addr_list = (struct in_addr **) he->h_addr_list; // grabs list of IP addresses
	}
	else // case of host being IP address 
	{
		www_flag=-1;
		serv_address.sin_addr.s_addr = inet_addr(host);
	}
	memset(serv_address.sin_zero, '\0', sizeof(serv_address.sin_zero)); // some important flag, no idea what...
	addr_size = sizeof(serv_address);
	int k=0;
	int connection_check;
	if(!www_flag) // on case we have a few IP's, we will try to connect with all of them until we succeed
	{
		for(k=0; addr_list[k] != NULL; k++)
		{
			char ip[33]; // ip address is 32 bytes + 1 with null terminator... a dynamic malloc is idiotic
			strcpy(ip,""); 
			strcat(ip,inet_ntoa(*addr_list[k])); // copies IP address to string
			serv_address.sin_addr.s_addr = inet_addr(ip);  // inputs into our struct
			//printf("IP #%d is : %s\n",k,ip);
			connection_check=connect(socket_fd, (struct sockaddr *) &serv_address, addr_size); // tries to connect
			if(connection_check != -1) // on connection succeess
				break;
			
			if(serv_address.sin_port != ntohs(80))
			{
				free(host);
				if(port)
					free(port);
				if(path)
					free(path);
				perror("connect()\n");
				return -1;
			}

		}
	}
	else
		connection_check=connect(socket_fd, (struct sockaddr *) &serv_address, addr_size); // because we have only one IP anyways...
	if(connection_check == -1) // if we still couldn't connect...
	{
		free(host);
		if(port)
			free(port);
		if(path)
			free(path);
		perror("connect()\n");
		return -1;
	}		
	int pp_buff_size = 0; // pre computes buffer size, smarter thing to do on local variables than reallocate array constantly since sys-calls are logner to interept
	if(minus_h)
		pp_buff_size+=(int) strlen("HEAD ");
	else
		pp_buff_size+=(int) strlen("GET ");
	if(path)
		pp_buff_size+=(int) strlen(path);
	else
		pp_buff_size++;
	pp_buff_size+=(int) strlen(" HTTP/1.0\r\n");
	pp_buff_size+=(int) strlen("Host:");
	pp_buff_size+=(int) strlen(host);
	pp_buff_size+=(int) strlen("\r\n");
	if(minus_d)
	{
		pp_buff_size+=(int) strlen("If-Modified-Since: ");
		pp_buff_size+=(int) (128*sizeof(char));
		pp_buff_size+=(int) strlen("\r\n");
	}
	pp_buff_size+=(int) strlen("\r\n");
	char* buffer = (char*) malloc(sizeof(char) * (pp_buff_size+1));
	if(!buffer)
	{
		free(host);
		if(port)
			free(port);
		if(path)
			free(path);
		close(socket_fd);
		input_err(2);
	}
	if(minus_h)
	{
		strcpy(buffer, "HEAD ");	
	}
	else
	{
		strcpy(buffer, "GET ");
	}
	if(path)
	{
		strcat(buffer,path);
	}
	else
	{
		strcat(buffer,"/");
	}
	strcat(buffer," HTTP/1.0\r\n");
	strcat(buffer,"Host:");
	strcat(buffer,host);
	strcat(buffer,"\r\n");
	if(minus_d)
	{
		strcat(buffer,"If-Modified-Since: ");
		time_t now;
		char timebuf[128];
		now = time(NULL);
		now=now-(d*24*3600+h*3600+m*60);
		strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&now));
		strcat(buffer, timebuf);
		strcat(buffer,"\r\n");
	}
	strcat(buffer,"\r\n");
	free(host);
	if(port)
		free(port);
	if(path)
		free(path);	
	int sum=0; // sums the bytes so far read/written
	int drw=1; // for while its above 0, to get the amount of sent bytes
	printf("Request: \n%s\n LEN =%d\n",buffer,(int) strlen(buffer));
	while(drw!=0)
	{
		drw = write(socket_fd,buffer+sum, strlen(buffer)-sum); // writes bytes, if its second try I will write the bytes from the place last stopped
		if(drw==-1) // if I failed, let me stop
		{
			free(buffer);
			close(socket_fd);
			perror("write()\n");
			return -1;
		}
		sum+=drw;
	}
	free(buffer);
	int drcv = 1; // same concept as before, only with read
	sum=0;
	char* buff_kb = (char*) malloc (sizeof(char) * KB);
	if(!buff_kb)
	{
		close(socket_fd);
		input_err(2);
	}
	printf("Data recieved: \n");
	while(drcv!=0)
	{
		drcv = recv(socket_fd, buff_kb, KB-1, 0);
		if(drcv==-1) // on failure
		{
			close(socket_fd);
			perror("recv()\n");
			return -1;
		}
		else if(drcv)
		{
			buff_kb[drcv]='\0';
			char* response = (char*) malloc (sizeof(char) * (drcv+1));
			if(!response)
			{
				close(socket_fd);
				input_err(2);
			}
			strcpy(response,buff_kb);
			response[drcv]='\0';
			printf("%s",response);
			free(response);
			sum+=drcv;
		}
	}
	free(buff_kb);
	printf("\n Total received response bytes: %d\n",sum);
	close(socket_fd);
	return 0;
}

int check_input(char* input, int minus, int dot)
{
	char* temp = input;
	int flag=0;
	int counter=0;
	while(*temp != '\0')
	{
		if(!isdigit( (int) *temp))
		{
			if(minus)
			{
				if(!(temp[0] == '-'))
				{
					return 0;
				}
				else
				{
					if(temp != input)
						return 0;
					if(flag==1)
					{
						return 0;
					}						
					flag=1;
				}
			}
			else if(dot)
			{
				if(!(temp[0] == '.'))
				{
					return 0;
				}
				else
				{
					if(flag==3)
						return 0;
					flag++;
				}
			}
			else
			{
				return 0;
			}
		}
		else
		{
			counter++;
		}
		temp++;
	}
	if((minus || dot) && !counter)
		return 0;
	return 1;
}

void input_err(int flag)
{
	if(!flag)
	{
		fprintf(stderr,"Command line usage: client [-h] [-d <time-interval>] <URL>.\n");
	}
	else if(flag==1)
	{
		fprintf(stderr,"Wrong input\n");
	}
	else if(flag==2)
	{
		perror("malloc()");
		exit(-1);
		//fprintf(stderr,"malloc()");
		//fprintf(stderr,"Memory error, possibily due to wrong usage of command line\nCommand line usage: client [-h] [-d <time-interval>] <URL>.\n");
	}
	exit(-1);
}

void toLower(char* str, int c)
{
	int i=0;
	while(i<c)
	{
		if(str[i] >= 'A' && str[i] <= 'Z')
			str[i] +=32;
		i++;
	}
}
