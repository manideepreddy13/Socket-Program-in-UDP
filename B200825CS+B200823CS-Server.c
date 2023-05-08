//sender code or server code (sends the packets every 100ms and 150ms)
#include<stdlib.h>
#include <stdio.h>
#include<unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define N 1024
#define pn 9999
#define serverip "127.0.0.1"

struct packet
{
    int type;
    int seqno;
    char arr[N];
    int checksum;
};
int my_socket, len, flen;
struct sockaddr_in server;
struct sockaddr_in from;
int get_checksum(char* arr,int n)
{
    int j=0;
    int curxor=0;
    while(j<n && arr[j]!='\0')
    {
        curxor=curxor^arr[j];
        j++;
    }
    return curxor;
}
void initialize_packet(struct packet* pac,int t,int sq,char* str)
{
    pac->type=t;
    pac->seqno=sq;
    int j=0;
    char* arr=pac->arr;
    while(str[j]!='\0')
    {
        arr[j]=str[j];
        j++;
    }
    arr[j]=str[j];
    pac->checksum=get_checksum(arr,N);
}
void string_gen(char* str,int n)
{
    int j=0;
    while(j<n-1)
    {
        str[j]=rand()%26+'a';
        j++;
    }
    str[j]='\0';
}
void print_packet(struct packet* p)
{
    printf("type: %d\n",p->type);
    printf("seqno: %d\n",p->seqno);
    printf("payload: %s\n",p->arr);
    printf("checksum: %d\n",p->checksum);
}
void send_packet(int type,int seq)
{
    struct packet p1;
    char str[N];
    string_gen(str,N);
    initialize_packet(&p1,type,seq,str);
    int retval;
    retval=sendto(my_socket,&p1,sizeof(p1),0,(struct sockaddr *)&server,len);
    print_packet(&p1);
}
void func_sender()
{
    int counter=0;
    int seqp1=1;
    int seqp2=1;
    while(1)
    {
        usleep(500000);
        counter++;
        if(counter%2==0)
        {
            send_packet(1,seqp1);
            seqp1++;
        }
        if(counter%3==0)
        {
            send_packet(2,seqp2);
            seqp2++;
        }
    }
}
void *type_sender(void* arg)
{
    int* adr=arg;
    int type=*adr;
    int counter=0;
    int seqno=1;
    while(1)
    {
        usleep(500000);
        counter++;
        if(counter%(type+1)==0)
        {
            send_packet(type,seqno);
            seqno++;
        }
    }
    printf("%d : hello\n",type);
    return NULL;
}

void retval_checker(int x,int id)
{
    if(x<0)
    printf("error  id: %d\n",id);
    
}
    
int main(int argc, char* argv[])
{
    

    int chosen=0;

    my_socket=socket(AF_INET,SOCK_DGRAM,0);
    retval_checker(my_socket,1);

    //initializing server
    len=sizeof(server);
    bzero(&server,len);
    server.sin_family=AF_INET;
    server.sin_addr.s_addr=INADDR_ANY;
    server.sin_port=htons(pn);
    int retval;
    retval=bind(my_socket,(struct sockaddr *)&server,len);
    if(chosen==1) //method 1
    {
        pthread_t t1s;
        pthread_t t2s;
        int t1=1;
        int t2=2;
        pthread_create(&t1s, NULL, type_sender, &t1);
        pthread_create(&t2s, NULL, type_sender, &t2);
        pthread_join(t1s, NULL);
        pthread_join(t2s, NULL);
        printf("done\n");
    }
    else //method 2
    func_sender();
    close(my_socket);
}
