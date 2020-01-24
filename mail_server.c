#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include<stdio.h>

char buffer[100];

int file_exists(const char * filename)
{
    /* try to open file to read */
    FILE *file;
    if (file = fopen(filename, "r"))
    {
        fclose(file);
        return 1;
    }
    return 0;
}

char* append(char *c,char *d)
{
    int cl=strlen(c);
    int dl=strlen(d);
    char *ret=(char *)malloc((cl+dl+1)*sizeof(char));
    int i;
    for(i=0; i<cl; i++)
    {
        ret[i]=c[i];
    }
    for(i=0; i<=dl; i++)
    {
        ret[i+cl]=d[i];

    }
    return ret;
}

void clearr()
{
    int i=0;
    for(i=0; i<100; i++)
    {
        buffer[i]=0;
    }
}

char *getEmail(char *c)
{
    char *ret=(char *)malloc(strlen(c)*sizeof(c));
    int i=0,j=0;
    while(c[i]!='<')i++;
    i++;
    while(c[i]!='>')
    {
        ret[j]=c[i];
        i++;
        j++;
    }
    ret[i]=0;
    return ret;
}


int main()
{

    char hostname[1024];    //getting hostname
    gethostname(hostname, 1024);

    struct sockaddr_in servaddr,cli;                 //Socket Creation.credit:geeksforgeeks
    int sockfd,len,socket_connect,valread;
    sockfd=socket(AF_INET,SOCK_STREAM,0);
    if(sockfd<0)
    {
        printf("Socket Creation Failed ! ");

    }
    else
    {
        printf("Connection Established\n");

    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = 0;  //port 0 means os will assign a free port

    if ((bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0)
    {
        printf("socket bind failed...\n");
        exit(0);
    }
    else
    {
        socklen_t len = sizeof(servaddr);
        if (getsockname(sockfd, (struct sockaddr *)&servaddr, &len) != -1)
            printf("port number %d\n", ntohs(servaddr.sin_port));
        printf("Socket successfully binded..\n");
    }

    listen(sockfd, 5); //Server is listening
    len = sizeof(cli);

    while((socket_connect = accept(sockfd, (struct sockaddr*)&cli, &len))>0)  //press ctrl+c to break and terminate the server
    {
        send(socket_connect, "220 WELCOME\n", 100, 0);

        clearr();
        valread = read(socket_connect,buffer,1024);
        printf("C: %s\n",buffer);
        if(strncmp(buffer,"HELO",3)!=0)
        {
            send(socket_connect, "500 Invalid Command\n", 100, 0);
            printf("500 Invalid Command\n");
            continue;
        }
        char *peerIP;
        peerIP=inet_ntoa(servaddr.sin_addr);
        char *hello = "250 hello";
        send(socket_connect, hello, 100, 0);


        //from

        clearr();
        valread = read( socket_connect, buffer, 100);
        if(strncmp(buffer,"MAIL FROM",9)!=0)
        {
            send(socket_connect, "500 Invalid Command\n", 100, 0);
            printf("S: 500 Invalid Command\n" );
            close(socket_connect);
            continue;
        }
        printf("C: %s\r\n",buffer );

        char *SENDER="250 ";
        char *SENDER2="... Sender ok";
        SENDER=append(SENDER,getEmail(buffer));
        SENDER=append(SENDER,SENDER2);
        send(socket_connect,SENDER, strlen(SENDER), 0 );


        char *RECIPIENT="250 ok ";                                                          //this appending style collected from tanvir fahim bhai's git
        char *DATA_START="354 Enter mail, end with \".\" on a line by itself";
        char *ACCEPTED="250 Message accepted for delivery";
        char *CLOSE_CONNECTION="221 ";
        char *CLOSE_CONNECTION2=" closing connection";
        CLOSE_CONNECTION=append(CLOSE_CONNECTION,hostname);
        CLOSE_CONNECTION=append(CLOSE_CONNECTION,CLOSE_CONNECTION2);
        char* IC="500 INVALID COMMAND";

        clearr();
        valread = read( socket_connect, buffer, 1024);
        if(strncmp(buffer,"RCPT TO",7)!=0)
        {

            send(socket_connect,IC, strlen(IC), 0 );
            printf("S: %s\r\n%s\r\n\n\n",buffer,IC );
            close(socket_connect);
            continue;
        }
        printf("C: %s \r\n",buffer );

        char *remail = getEmail(buffer);
        RECIPIENT=append(RECIPIENT,remail);



        int ind=0;
        for(ind=0; ind<strlen(remail); ind++)
        {
            if(remail[ind]=='@')
            {
                remail[ind]=0;
                break;
            }
        }

        FILE *ppFile;

        ppFile=fopen(remail, "r");
        if(ppFile==NULL)
        {
            char* er="421 User not exists";

            send(socket_connect,er, strlen(er), 0 );
            printf("S: %s\r\n\n\n\n",er );
            close(socket_connect);
            continue;
        }
        fclose(ppFile);


        send(socket_connect,RECIPIENT, strlen(RECIPIENT), 0 );
        printf("S: %s\r\n",RECIPIENT );


        clearr();
        valread = read( socket_connect, buffer, 1024);
        if(strncmp(buffer,"DATA",4)!=0)
        {

            send(socket_connect,IC, strlen(IC), 0 );
            printf("S: %s\r\n%s\r\n\n\n",buffer,IC );
            close(socket_connect);
            continue;
        }
        printf("C: %s\r\n",buffer );



        send(socket_connect,DATA_START, strlen(DATA_START), 0 );

        FILE *pFile;
        pFile=fopen(remail, "a");


        ///
        while(1)
        {
            clearr();
            valread = read( socket_connect, buffer, 1024);
            printf("C: %s\r\n",buffer);
            if(buffer[0]=='.'&&buffer[1]==0)
                break;
            else
            {

                send(socket_connect,".", strlen("."), 0 );
                fprintf(pFile, "%s\r\n", buffer);
            }
        }
        fprintf(pFile, "\r\n\n\n\n" );
        fclose(pFile);

        ///

        send(socket_connect,ACCEPTED, strlen(ACCEPTED), 0 );
        printf("S: %s\r\n",ACCEPTED);

        clearr();
        valread = read( socket_connect, buffer, 1024);
        if(strncmp(buffer,"QUIT",4))
        {

            send(socket_connect,IC, strlen(IC), 0 );
            printf("S: %s\r\n%s\r\n\n\n",buffer,IC );
            close(socket_connect);
            continue;
        }
        printf("C: %s\r\n",buffer );

        send(socket_connect,CLOSE_CONNECTION, strlen(CLOSE_CONNECTION), 0 );

        printf("S: %s\r\n\n\n\n",CLOSE_CONNECTION);
        close(socket_connect);
    }

    return 0;
}

