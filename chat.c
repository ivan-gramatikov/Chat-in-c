#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>
#include <ctype.h>

#define HOST "127.0.0.1"
#define LENGTH 512					/*  */
#define N_OF_USERS 50              /*  */

	void *thread_func(void *arg);	/*  */
	void sig_handler(int sign);    /*  */
	int s, ns, nport, nbytes;
	int sockets_array[N_OF_USERS]; 
	char c_time[16];
	static int array_ind = 0;
	
	struct UserStruct {				/*  */
		char name[32];
		int socket;
	} user[N_OF_USERS]; 
    
    
    int is_user(char* name) {			/*  */ 
		int i = 0;
		for( i = 0; i <= array_ind; i++ ) {
			 if( strcmp(name, user[i].name) == 0 ) {
				 return 1;
			 }
		}
		return 0;
    }
    
    int search_socket_by_name(char* name) {	/*  */
		int i = 0;
		for( i = 0; i <= array_ind; i++ ) {
			 if( strcmp(name, user[i].name) == 0 ) {
				 return user[i].socket;
			 }
		}
		return 1;
	}
	
	int search_socket_to_delete(char* name) {	/*   */
		int i = 0;
		for( i = 0; i <= array_ind; i++ ) {
			 if( strcmp(name, user[i].name) == 0 ) {
				 user[i].socket = 0;				/*  */
				 strcpy(user[i].name, "");
			 }
		}
		return 1;
	}
	
	   char* remove_n(char* str) {					/*  */
		char *p = strchr((const char*)str, '\r');
		if ( p != NULL )
			*p = '\0';
		p = strchr((const char*)str, '\n');
		if ( p != NULL )
			*p = '\0';
		return str;
	}
	
	int validate_name(char* name) {					/*  */
		char* p = remove_n(name);					/*  */
		while( *p != '\0' ) {
			if( isalpha(*p) || isdigit(*p) || ( *p == '_' ) ) {
				if( isdigit(*p) ) {
					p++;
					continue;
				}
				else if( isalpha(*p) ) {
					p++;
					continue;
				}
					else if( *p == '_' ) {
						p++;
						continue;
					}
						else return 0; 		
			} 
			else return 0; 	
			p++;
		}
		return 1;
	}
	
	char* time_str() {					/*  */
		char c_hour[3];
		char c_min[3];
		char c_sec[3];
		time_t raw_time; 
		struct tm *t;
		time(&raw_time);				/* */
		t = localtime(&raw_time); 		/*  */
		sprintf(c_hour,"%d",t->tm_hour);
		sprintf(c_min,"%d",t->tm_min);
		sprintf(c_sec,"%d",t->tm_sec);
		strcpy(c_time,"[");			
		strcat(c_time,c_hour);									/* */
		strcat(c_time,":");
		strcat(c_time,c_min);
		strcat(c_time,":");
		strcat(c_time,c_sec);
		strcat(c_time,"] ");
		return c_time;	
	}
    
    int main(int argc, char* argv[]) {
        int nport = atoi(argv[1]);								/*  */
        pthread_t thread;
        pthread_attr_t attr;                					/*   */
        unsigned int ids;                   					/*  */
        socklen_t addrlen;
        struct sockaddr_in6 serv_addr, clnt_addr;
        struct hostent;
        bzero(&serv_addr, sizeof(serv_addr));
        serv_addr.sin6_family = AF_INET6; 						/*  */
        //serv_addr.sin_addr.s_addr = INADDR_ANY; 				/*  */
        serv_addr.sin6_addr = in6addr_any; 
        serv_addr.sin6_port = htons(nport);
        serv_addr.sin6_scope_id = 5;
        if( (s = socket(AF_INET6, SOCK_STREAM, 0)) == -1 ) {	/*  */
            perror("error calling socket()"); 					/*  */
            exit(EXIT_FAILURE);
        }
        if( bind(s,(struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1  ) {
            perror("error calling bind()"); 
            close(s);
            exit(EXIT_FAILURE);
        }
        printf("Server is ready, waiting fo connection...\n");   /*  */
        if( listen(s, 5) == -1 ) {
            perror("error calling listen()"); 
            close(s);
            exit(EXIT_FAILURE);
        }
        pthread_attr_init(&attr);
        signal(SIGINT, sig_handler); 				/*  */
        
        while( 1 ) {								/*  */
            int res;
            bzero(&clnt_addr, sizeof(clnt_addr));
            addrlen = sizeof(clnt_addr);
            if( (ns = accept(s,(struct sockaddr*)&clnt_addr, &addrlen)) == -1 ) { /*  */
                perror("error calling accept()");
                close(s);
                exit(EXIT_FAILURE);
            }          
            write(ns, "OK. Conected to server, waiting for command...\n", sizeof("OK. Conected to server, waiting for command...\n"));
            ids = ns;
            res = pthread_create(&thread, &attr, &thread_func, &ids);    /*  */
            if( res != 0 ) {
				perror("Thread creation failed");
				close(s);
				exit(EXIT_FAILURE);
            }
        }
        close(s);
        //return 0;
    }
       
    void *thread_func(void *arg)          	/* */
    {	
		char name_command[] = "OK, command received, enter your name\n";
		char register_error[] = "Error! User's already registered!\n";
		char notfound_error[] = "Error, there's no such username!\n";
		char message_to_prompt[] = "Enter username to send the message to\n";
		char list[LENGTH];					/* */
		char message[LENGTH];
		char buffer[LENGTH];
		char command[16];					/*  */
		char recv_buf[LENGTH]; 				/*  */
		int is_registered = 0;				/*  */
		char current_user_name[64]; 
        unsigned int clients_s;         	/* */
        clients_s = *(unsigned int*)arg;     
       
        while(1)                            /*  */ {
			bzero(&buffer, sizeof(buffer)); /*  */
			bzero(&message, sizeof(message));
			bzero(&command, sizeof(command));
			bzero(&recv_buf, sizeof(recv_buf));
            bzero(&command, sizeof(command)); /**/
            nbytes = recv(clients_s, command, sizeof(command), 0); /*  */    
              
            if( strcmp(remove_n(command), ".name") == 0 ) {       					 /*  */
				if( is_registered ) {							   					/*  */
					write(clients_s, "Error! Already registered!\n", sizeof("Error! Already registered!\n"));	/*  */		
					bzero(&command, sizeof(command));
					continue;
				}	 
                write(clients_s, name_command, sizeof(name_command));
                nbytes = recv(clients_s, recv_buf, sizeof(recv_buf), 0);  /*  */
                if( !validate_name(recv_buf) ) {						  /*  */
					write(clients_s, "Error. Name isn't valid!\n", sizeof("Error. Name isn't valid!\n"));
					bzero(&recv_buf, sizeof(recv_buf));
					bzero(&command, sizeof(command));
					continue;
				}				
                if( is_user(remove_n(recv_buf)) ) {						 /*  */
					write(clients_s, register_error, sizeof(register_error));
					bzero(&recv_buf, sizeof(recv_buf));
					bzero(&command, sizeof(command));
					continue;
				}
                write(clients_s, "You're registered!\n", sizeof("You're registered!\n"));
                is_registered = 1;										/*  */
                strcpy(user[array_ind].name, recv_buf);
                strcpy(current_user_name, recv_buf);					/*  */
                user[array_ind].socket = clients_s;						/*  */
				array_ind++;											/*  */
            } else 
				if( strcmp(remove_n(command), ".msg") == 0 ) {			/* */
					if( !is_registered ) {								/*  */
						write(clients_s, "Error! Register first!\n", sizeof("Error! Register first!\n"));
						bzero(&recv_buf, sizeof(recv_buf));
					    bzero(&command, sizeof(command));
					    bzero(&buffer, sizeof(buffer));
						continue;
					}
					write(clients_s, message_to_prompt, sizeof(message_to_prompt));
					recv(clients_s, recv_buf, sizeof(recv_buf), 0); 	/*  */ 
					if( !is_user(remove_n(recv_buf)) ) {				/*  */
						write(clients_s, notfound_error, sizeof(notfound_error));
					    bzero(&recv_buf, sizeof(recv_buf));
					    bzero(&command, sizeof(command));
						continue;
					}
					write(clients_s, "Write your message:\n", sizeof("Write your message:\n"));
					recv(clients_s, message, sizeof(message), 0); 		/*  */				
					strcpy(buffer, time_str());
					strcat(buffer, "PRIVATE message from: ");
				    strcat(buffer, remove_n(current_user_name));
				    strcat(buffer, " to ");
				    strcat(buffer, recv_buf);
				    strcat(buffer, "> ");
					strcat(buffer, message);
					printf("%s",buffer);
					if( !search_socket_by_name(recv_buf) ) {			/*  */	
						perror(notfound_error);
					}
					else {
						write(search_socket_by_name(recv_buf), buffer, sizeof(buffer)); /*  */
					}
			} else 
				if( strcmp(remove_n(command), ".msg_all") == 0 ) {  	/*  */
					bzero(&recv_buf, sizeof(recv_buf));
					if( !is_registered ) {
						write(clients_s, "Error! Register first!\n", sizeof("Error! Register first!\n"));
					    bzero(&command, sizeof(command));
						continue;
					}
					write(clients_s, "Write your message:\n", sizeof("Write your message:\n"));
					recv(clients_s, recv_buf, sizeof(recv_buf), 0);     /*  */
					strcpy(buffer, time_str());
					strcat(buffer, "Public message from: ");
					strcat(buffer, current_user_name);
					strcat(buffer, "> ");
					strcat(buffer, recv_buf);
					printf("%s",buffer);
					int i = 0;
					for( i = 0; i <= array_ind; i++ ) {					/*  */
						write(user[i].socket, buffer, sizeof(buffer));	
					}
			} else
				if( strcmp(remove_n(command), ".list") == 0  ) {		/*  */
					int n = 0;
					bzero(&list, sizeof(list));
					while( n != array_ind ) { 
						if( strcmp(user[n].name, "") != 0 ) {
							strcat(list, user[n].name);	
							strcat(list, "\n");
							n++;
							continue;
						}
					n++;
					}
					write(clients_s, list, sizeof(list));			
			} else            
				if( strcmp(remove_n(command), ".quit") == 0  ) {
					search_socket_to_delete(current_user_name); /* */
					int i = 0;
					//bzero(&buffer, sizeof(buffer));
					strcpy(buffer, remove_n(current_user_name));
					strcat(buffer, " is out\n");
					printf("%s", buffer);
					for( i = 0; i <= array_ind; i++ ) {	
						if(	user[i].socket != 0 ) {	
							write(user[i].socket, buffer, sizeof(buffer));	/*  */
						}
					}	
					close(clients_s);	
					return 0;
			} else {
					write(clients_s, "Unknown command.\n", sizeof("Unknown command.\n"));	
			} 			
        }
    }

    void sig_handler(int sign)     /*  */
    {
		if ( sign == SIGINT ) {    /*  */
            close(s);
            printf("Good bye\n");
            exit(EXIT_FAILURE);
        }
     }
