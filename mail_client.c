#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include<time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>


char buffer[100];

int check(char *mail)
{
    char *a = strchr(mail,'@');
    char *b = strchr(mail,':');
    int la = a-mail+1;
    int lb = b-mail+1;
    if(a==NULL || b == NULL ) return 0;
    if(lb-la>0) return 1;
    return 0;
}

char *getter(char *mail,int type)
{
    char * pch;
    char *ret[3];
    int i=0;
    pch = strtok (mail,"@:");
    while (pch != NULL && i<3)
    {
        ret[i] = pch;
        pch = strtok (NULL, "@:");
        i++;
    }
    return ret[type-1];

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


void clear()
{
    int i=0;
    for(i=0; i<100; i++)
    {
        buffer[i]=0;
    }
}

char *getFile(char *fname)  //This filereading is collcted from tutorialspoint.
{
    char *ch;
    ch= (char *)malloc(250*sizeof(char));
    int c;
    int i=0;
    FILE *file;
    file = fopen(fname, "r");
    if (file)
    {
        while ((c = getc(file)) != EOF)
        {
            ch[i]=c;
            i++;
        }
        fclose(file);
        ch[i]=0;
    }
    else
    {
        printf("FILE NOT FOUND\n");
        exit(0);
    }
    return ch;
}




int main(int argc, char const *argv[])
{
    if(argc<4)
    {
        printf("Incorrect number of arguments : %d\n",argc);
        return 0;
    }

    char *email = (char*) argv[1];
    if(!check(email))
    {
        printf("Incorrect Mail Format\n");
        return 0;
    }
    char *user = getter(email,1);
    char *serverr = getter(email,2);
    int port = atoi(getter(email,3));
    printf("%d\n",port);
    char hostname[50];
    gethostname(hostname, 50);
    char *uname=getenv("USER");
    char *myemail= (char *) strcat(uname,"@");
    myemail= (char *)strcat(myemail,hostname);
    char subject[100];
    for(int i =2; i<argc-1; i++)
    {
        strcat(subject,argv[i]);
        strcat(subject," ");
    }

    char *fname=(char *) argv[argc-1];
    char *mailBody=getFile(fname);
    char date[30];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(date, sizeof(date)-1, "DATE %d-%m-%Y %H:%M", t);

    struct sockaddr_in serv_addr;
    struct hostent *server;
    int sock,valread;
    ///socket creation & verification
    sock=socket(AF_INET,SOCK_STREAM,0);
    if(sock<0)
    {
        printf("Socket creation failed\n");

    }
    else
        printf("Socket successfully created\n");
    bzero((char *) &serv_addr, sizeof(serv_addr));


    bzero(&serv_addr, sizeof(serv_addr));


    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(port);

    // connect the client socket to server socket
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) != 0)
    {
        printf("connection with the server failed...\n");
        exit(0);
    }
    else
        printf("connected to the server..\n");

    clear();
    valread = read( sock, buffer, 1024);
    if(buffer[0]!='2')
    {
        printf("ERROR %s\n",buffer);
        return 0;
    }
    printf("Server: %s\r\n",buffer );
    char *hello = "HELO";
    hello = append(hello,serverr);
    send(sock,hello,100,0 );

    clear();
    valread = read( sock, buffer, 1024);
    if(buffer[0]!='2')
    {
        printf("ERROR %s\r\n",buffer);
        return 0;
    }
    printf("Server: %s\r\n",buffer );


    //mail from
    char *MAIL_FROM="MAIL FROM: <";
    char *MAIL_FROM2=">";
    MAIL_FROM=append(MAIL_FROM,myemail);
    MAIL_FROM=append(MAIL_FROM,MAIL_FROM2);
    send(sock, MAIL_FROM, 100, 0 );


    clear();
    valread = read( sock, buffer, 1024);
    if(buffer[0]!='2')
    {
        printf("ERROR %s\r\n",buffer);
        return 0;
    }
    printf("Server: %s\r\n",buffer );

    char *RCPT="RCPT TO: ";
    char *DATA="DATA";
    char *QUIT="QUIT";
    char *st="<";
    char *en=">";
    RCPT=append(RCPT,st);
    RCPT=append(RCPT,email);
    RCPT=append(RCPT,en);
    send(sock, RCPT, strlen(RCPT), 0 );
    printf("C: %s\r\n",RCPT);

    clear();
    valread = read( sock, buffer, 1024);
    if(buffer[0]!='2')
    {
        printf("ERROR %s\r\n",buffer);
        return 0;
    }
    printf("Server: %s\r\n",buffer );


    send(sock, DATA, strlen(DATA), 0 );
    printf("C: %s\r\n",DATA);

    clear();
    valread = read( sock, buffer, 1024);
    if(buffer[0]!='2'&&buffer[0]!='3')
    {
        printf("ERROR %s\r\n",buffer);
        return 0;
    }
    printf("Server: %s\r\n",buffer );


    char *from="FROM: <";
    char *from2=">";
    from=append(from,myemail);
    from=append(from,from2);
    send(sock, from, strlen(from), 0 );
    printf("C: %s\r\n",from);

    clear();
    valread = read( sock, buffer, 1024);


    char *to="TO: <";
    char *to2=">";
    to=append(to,email);
    to=append(to,to2);
    send(sock, to, strlen(to), 0 );
    printf("C: %s\r\n",to);

    clear();
    valread = read( sock, buffer, 1024);

    send(sock, date, strlen(date), 0 );
    printf("C: %s\r\n",date);

    clear();
    valread = read( sock, buffer, 1024);

    char *sub="Subject: ";
    sub=append(sub,subject);


    send(sock, sub, strlen(sub), 0 );
    printf("C: %s\r\n",sub);

    clear();
    valread = read( sock, buffer, 1024);




///

    char *spbuff=" ";
    send(sock, spbuff, strlen(spbuff), 0 );
    printf("C: %s\r\n",spbuff);
    clear();
    valread = read( sock, buffer, 1024);

    int maillen=strlen(mailBody);
    char *filebuff=(char *)malloc(maillen*sizeof(char));
    int i,j;
    for(i=0; i<strlen(mailBody); i++)
    {
        filebuff[j]=mailBody[i];
        if(mailBody[i]=='\n'||mailBody[i]==0)
        {
            filebuff[j]=0;
            j=0;
            send(sock, filebuff, strlen(filebuff), 0 );
            printf("C: %s\r\n",filebuff);
            clear();
            valread = read( sock, buffer, 1024);
        }
        else
            j++;
    }
    char *dot=".";
    send(sock, dot, strlen(dot), 0 );
    printf("C: %s\r\n",dot);
///



    clear();
    valread = read( sock, buffer, 1024);
    if(buffer[0]!='2')
    {
        printf("ERROR\r\n");
        return 0;
    }
    printf("Server: %s\r\n",buffer );



    send(sock, QUIT, strlen(QUIT), 0 );
    printf("C: %s\r\n",QUIT);

    clear();
    valread = read( sock, buffer, 1024);
    if(buffer[0]!='2')
    {
        printf("Server: %s\r\n",buffer );
        printf("ERROR\r\n");
        return 0;
    }
    printf("Server: %s\r\n",buffer );

    return 0;
}

