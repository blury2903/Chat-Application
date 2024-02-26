#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#define clear() printf("\033[H\033[J") 
#define LISTEN_BACKLOG 5
#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)


pthread_t thr_server, thr_client, thr_msg;

struct data {
    int port;
    char* ip;
    int fd;
};

int client_fd[LISTEN_BACKLOG];
struct sockaddr_in client_addr[LISTEN_BACKLOG];
int client_port[LISTEN_BACKLOG];
char client_ip[LISTEN_BACKLOG][INET_ADDRSTRLEN];

int port = 0;
char cmd[100];
char* spl_cmd[100];
int id = 0;
int split_len = 0;
char server_ip[20];
int global_fd, global_port;


/*Function to remove new line in string*/
void rm_newLine(char *str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}

/*Function to split command*/
void *split_cmd(char *string, char *tokens[])
{
    int i = 0;
    char *token = strtok(string, " ");
    while (token != NULL) {
        tokens[i++] = token;
        token = strtok(NULL, " ");
    }
    split_len = i;
}

/*Function to process message */
char* get_msg(int len)
{
    char *msg = malloc(100);
    if (msg == NULL) {
        handle_error("malloc()");
    }
    msg[0] = '\0'; 
    for(int i = 2; i < len; i++){
        strcat(msg, spl_cmd[i]);
        strcat(msg, " ");
    }
    return msg;
}


/*Function to get port from socker file descriptor*/
int get_port(int fd)
{
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    getpeername(fd, (struct sockaddr*)&addr, &addr_len);
    int g_port = ntohs(addr.sin_port);
    return g_port;
}


/*Function to get ip from socker file descriptor*/
char* get_ip(int fd)
{
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    getpeername(fd, (struct sockaddr*)&addr, &addr_len);
    char *g_ip = malloc (15);
    inet_ntop(AF_INET, &(addr.sin_addr), g_ip, INET_ADDRSTRLEN);
    return g_ip;
}



/*Function to displau all available commands*/
void help_list()
{
    printf("\nAvailable Commands: \n");
    printf("help                                            - Display information about the available user interface options or command manual.\n");
    printf("myip                                            - Display the IP address of this process.\n");
    printf("myport                                          - Display the port on which this process is listening for incoming connections.\n");
    printf("connect <destination> <port>                    - Establish a new TCP connection to the specified destination at the specified port.\n");
    printf("list                                            - Display a numbered list of all the connections this process is part of.\n");
    printf("terminate <connection id>                       - Terminate the connection with connection id from connection list.\n");
    printf("send <connection id> <message>                  - Send a message to the connection with connection id from connection list.\n");
    printf("exit                                            - Close all connections and terminate this process.\n");
}


/*Function to get help list*/
void help_cmd()
{
    if(split_len == 1){
        help_list();
        printf("****Please enter your command****\n\n");
    } else{
        printf("Usage: help\n");
    }
}


/*Function to get server ip*/
void get_host_ip(char* ip)
{
    FILE *fd = popen("hostname -I", "r");
    if(fd == NULL){
        printf("get_host_ip() error\n");
        return;
    }
    fgets(ip, 20, fd);
    rm_newLine(ip);

}


/*Function to dislay all connections*/
void list_cmd()
{
    
    if(split_len == 1){
        printf("List of all connections:\n");
        for(int i = 0; i < id; i++){
            printf("ID: %d      IP: %s      PORT: %d\n", i, client_ip[i], client_port[i]);
        }
    } else{
        printf("Usage: list\n");
    }

}


/*Function to connect between client and server*/
void connect_cmd()
{
    if(split_len == 3)
    {
        
        client_port[id] = atoi(spl_cmd[2]);
        client_addr[id].sin_family = AF_INET;
        client_addr[id].sin_port   = htons(client_port[id]);

        if (inet_pton(AF_INET, spl_cmd[1], &client_addr[id].sin_addr) == -1) 
            handle_error("inet_pton()");

        for(int i = 0; i < id; i++){
            if(client_port[id] == client_port[i]){
                printf("Port already in use....\n");
                return;
            }
        }

        client_fd[id] = socket(AF_INET, SOCK_STREAM, 0);
        if (client_fd[id] == -1)
            handle_error("socket()");

        if (connect(client_fd[id], (struct sockaddr *)&client_addr[id], sizeof(client_addr[id])) == -1)
        {
            printf("Connect error()\n...\n");
            printf("Please connect again.\n");
            return;
        }
        char temp_ip[15];
        inet_ntop(AF_INET, &client_addr[id].sin_addr, temp_ip, INET_ADDRSTRLEN);
        strcpy(client_ip[id], temp_ip);

        client_port[id] = get_port(client_fd[id]);

        printf("Connected IP %s at port %d\n", client_ip[id], client_port[id]);
        id++;
    } else{
        printf("Usage: connect <destination> <port>\n");
    }
    
}

/*Function to display server ip*/
void myip_cmd()
{
    if(split_len == 1){
        printf("My IP is %s\n", server_ip);
    } else{
        printf("Usage: myip\n");
    }
}

/*Function to display server port*/
void myport_cmd()
{
    if(split_len == 1){
        printf("My port is %d\n", port);
    } else{
        printf("Usage: myport\n");
    }
}


/*Function to terminate client*/
void terminate_cmd()
{
    if(split_len == 2)
    {
        
        int id_temp = atoi(spl_cmd[1]);
        if(id_temp >= 0 && id_temp <= id){
            for(int i = id_temp; i <= id; i++){
                strcpy(client_ip[i], client_ip[i+1]);
                client_port[i] = client_port[i+1];
                client_fd[i] = client_fd[i+1];
            }
            printf("Connection with Client ID %d terminated....\n", id_temp);
            id--;
        }else{
            printf("Invalid ID....\n");
        }
        
    } else{
        printf("Usage: terminate <connection id>\n");
    }
}


/*Function to send message to server*/
void send_cmd()
{
    if(split_len >= 3)
    {
        int id_temp = atoi(spl_cmd[1]);
        char msg[100];
        int write_fd;

        strcpy(msg, get_msg(split_len));
        
        if(id_temp >= 0 && id_temp <= id){
            
            write_fd= write(client_fd[id_temp], msg, 100);
            if(write_fd == -1){
                handle_error("read()");
            }
            printf("Message sent to  %s\n", client_ip[id_temp]);
            memset(&msg, 0, 100);
        }else{
            printf("Invalid ID....\n");
        }
    } else{
        printf("Usage: send <connection id> <message>\n");
    }
}


/*Function to process command*/
void process_cmd() 
{
    if(strcmp(spl_cmd[0], "help") == 0){
        help_cmd();
    }
    else if(strcmp(spl_cmd[0], "myip") == 0){
        myip_cmd();
    }
    else if(strcmp(spl_cmd[0], "myport") == 0){
        myport_cmd();
    }
    else if(strcmp(spl_cmd[0], "connect") == 0){
        connect_cmd();
    }
    else if(strcmp(spl_cmd[0], "list") == 0){
        list_cmd();
    }
    else if(strcmp(spl_cmd[0], "terminate") == 0){
        terminate_cmd();
    }
    else if(strcmp(spl_cmd[0], "send") == 0){
        send_cmd();
    }
    else if(strcmp(spl_cmd[0], "exit") == 0){
        printf("Shutting down program....\n");
        // pthread_cancel(thr_msg);
        pthread_cancel(thr_server);
        pthread_exit(NULL);
    }
    else{
        printf("Invalid command. Type 'help' for list of available commands.\n");
    }

}


/*Function to read message from client and display it to screen*/
void *msg_handle(void *args)
{
    struct data *data = (struct data *) args;
    char msg[100];
    int read_fd;
    int msg_fd = data->fd;
    int msg_port = data->port;
    char *msg_ip = data->ip;
    
    while(1){
        memset(&msg, 0, 100);
        
        read_fd = read(msg_fd, msg, 100);
        if(read_fd == -1){
            handle_error("read()");
        }
        if(read_fd == 0){
            printf("%s at port %d disconnected\n......\n", msg_ip, msg_port);
            break;
        }
            
        printf("Message received from %s\n", msg_ip);
        printf("Sender's Port: %d\n", msg_port);
        printf("Message: %s\n", msg);
        printf("......................\n");

    }
    pthread_exit(NULL);
}


/*Function to get and handle command*/
void *cli_handle(void *args)
{
    printf("****Please enter your command****\n\n");
    while(1){
        fgets(cmd, sizeof(cmd), stdin);
        rm_newLine(cmd);
        split_cmd(cmd, spl_cmd);
        process_cmd();
    }
    pthread_exit(NULL);

}

/*Function to init server socket and accept new device*/		
void *serv_handle(void *args)
{
    struct sockaddr_in server_addr, client_addr;
    int server_fd, new_fd;
    
    memset(&server_addr, 0, sizeof(struct sockaddr_in));

    //init socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
        handle_error("Server socket() error"); 

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr =  INADDR_ANY;
    
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
        handle_error("Server bind() error");

    if (listen(server_fd, LISTEN_BACKLOG) == -1)
        handle_error("Server listen() error");
    
    get_host_ip(server_ip);
    printf("Server is listening from %s at port : %d\n....\n", server_ip, port);
    
    pthread_create(&thr_client, NULL, &cli_handle, NULL);

    int client_len = sizeof(client_addr);
    while(1){
        struct data thr_data;

        new_fd = accept(server_fd, (struct sockaddr*)&client_addr, (socklen_t *) &client_len);
        if(new_fd == -1){
            handle_error("Client accept() error");
        }else{
            thr_data.ip = get_ip(new_fd);
            getpeername(new_fd, (struct sockaddr*)&client_addr, &client_len);
            thr_data.port = ntohs(client_addr.sin_port);
            thr_data.fd = new_fd;
            printf("Accepted connection from IP %s\n....\n", thr_data.ip);
        }
        pthread_create(&thr_msg, NULL, &msg_handle, &thr_data);
    }

    pthread_exit(NULL);
}


/*Main function*/
int main(int argc, char *argv[])
{
    clear();
    if (argc < 2) {
        printf("No port provided\nCommand: ./chat <port number>\n");
        exit(EXIT_FAILURE);
    } else
        port = atoi(argv[1]);

    //create thread
    if (pthread_create(&thr_server, NULL, &serv_handle, NULL) != 0) {
        handle_error("pthread_create");
    }
    
    //used to block for the end of a thread and release
    pthread_join(thr_server,NULL);  
    pthread_join(thr_client,NULL);  
    pthread_join(thr_msg, NULL);  

    //close all client fds
    for (int i = 0; i < id; i++) {
        close(client_fd[i]);
    }

    printf("Server is terminated.....\n");

    return 0;
}

