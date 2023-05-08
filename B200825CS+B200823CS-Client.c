//reciever code or client code ( recieves the packets sent by the sender and processes them)
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
#include <semaphore.h>
#define pn 9999
#define MAXLINE 1000
#define N 1024
sem_t s1;
sem_t s2;
sem_t s3;

struct packet
{
    int type;
    int seqno;
    char arr[N];
    int checksum;
};

int my_socket, len;
struct packet p1;
struct sockaddr_in servaddr, cliaddr;



int received_count_1=0;
int received_count_2=0;
int t1prev=-1;
int t2prev=-1;

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

void print_packet(struct packet* p)
{
    printf("type: %d\n",p->type);
    printf("seqno: %d\n",p->seqno);
    printf("payload: %s\n",p->arr);
    printf("checksum: %d\n",p->checksum);
}


void check_for_errors(struct packet* p)
{
    printf("checking for errors\n");
    if(p->checksum==get_checksum(p->arr,N))
    {
        printf("No error in the packet\n");
    }
    else
    {
        printf("error in recieved packet\n"); 
    }
}

void *checker_thread(void* arg) //t1
{
    while(1)
    {
        sem_wait(&s1);
        int n = recvfrom(my_socket, &p1, sizeof(p1),0, (struct sockaddr*)&cliaddr,&len);
        check_for_errors(&p1);
        sem_post(&s1);
        usleep(10000);
    }
    
}

void *type1_process(void* arg) //t2
{
    while(1)
    {
        sem_wait(&s1);
        if(p1.type==1 && p1.seqno>t1prev)
        {
            print_packet(&p1);
            t1prev=p1.seqno;
            sem_post(&s1);

            sem_wait(&s2);
            received_count_1++;
            sem_post(&s2);
        }
        else
        sem_post(&s1);

    }
}

void *type2_process(void* arg) //t3
{
    while(1)
    {
        sem_wait(&s1);
        if(p1.type==2 && p1.seqno>t2prev)
        {
            print_packet(&p1);
            t2prev=p1.seqno;
            sem_post(&s1);

            sem_wait(&s2);
            received_count_2++;
            sem_post(&s2);
        }
        else
        sem_post(&s1);

    }
}

void *counter_thread(void* arg) //t4
{
    while(1)
    {
        usleep(300000);
        sem_wait(&s2);
        printf("packets recieved of type 1: %d\n",received_count_1);
        printf("packets recieved of type 2: %d\n",received_count_2);
        printf("\n");
        sem_post(&s2);
    }
}

void* func_receive(void* arg)
{
    while(1)
    {
        int n = recvfrom(my_socket, &p1, sizeof(p1),0, (struct sockaddr*)&cliaddr,&len);
        check_for_errors(&p1);
        if(p1.type==1)
        {
            print_packet(&p1);
            sem_wait(&s2);
            received_count_1++;
            sem_post(&s2);
        }
        else if(p1.type==2)
        {
            print_packet(&p1);
            sem_wait(&s2);
            received_count_2++;
            sem_post(&s2);
        }
        printf("\n");
    }
}
int main()
{
    sem_init(&s1, 0, 1);
    sem_init(&s2, 0, 1);
    sem_init(&s3, 0, 1);
	bzero(&servaddr, sizeof(servaddr));
	my_socket = socket(AF_INET, SOCK_DGRAM, 0);		
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(pn);
	servaddr.sin_family = AF_INET;
	bind(my_socket, (struct sockaddr*)&servaddr, sizeof(servaddr));
	len = sizeof(cliaddr);

    int choose=1; 
    if(choose==1) //method 1 4 thread one
    {
        pthread_t t1r;
        pthread_t t2r;
        pthread_t t3r;
        pthread_t t4r;
        pthread_create(&t1r, NULL, checker_thread, NULL);
        pthread_create(&t2r, NULL, type1_process, NULL);
        pthread_create(&t3r, NULL, type2_process, NULL);
        pthread_create(&t4r, NULL, counter_thread, NULL);
        pthread_join(t1r, NULL);
        pthread_join(t2r, NULL);
        pthread_join(t3r, NULL);
        pthread_join(t4r, NULL);
    }
    else //method 2
    {
        pthread_t t1r;
        pthread_t t2r;
        pthread_create(&t1r, NULL, func_receive, NULL);
        pthread_create(&t2r, NULL, counter_thread, NULL);
        pthread_join(t1r, NULL);
        pthread_join(t2r, NULL);
    }
    close(my_socket);
}
