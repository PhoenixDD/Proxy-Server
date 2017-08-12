/****************************************************************************************************************************************************************
  Author: 
  Dhairya Dhondiyal
  
  Description:
  The program uses simple sockets programming to start a proxy server on the Host machine that handles GET requests and HTTP protocols only.
  
  Usage:
  Compile with port as argument.
  Eg: ./proxy.c 2222.
  
****************************************************************************************************************************************************************/
#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<strings.h>
#include<string.h>
#include<stdlib.h>
#include<netdb.h>
#include<stdbool.h>
#include<sys/select.h>

#define BACKLOG_QUEUE 1                                                       //Number of connections to listen.

int main(int arg_cnt,char *arg_val[])
{
  int proxy_sock,action_sock,clilen,n,web_sock;                               //Declaring all sockets.        
  char buffer[655350],temp_buffer[10];                                                          //Declaring read,write buffer.          
  struct sockaddr_in proxy_addr,accept_addr,web_addr;                         //Declaring internet addresses object of structures for proxy and webserver.
  struct hostent *webserver;
  
  if(arg_cnt<2)                                                               //Arguments provided while running less than 1
  {
    fprintf(stderr,"\nError, No port provided ");
    exit(1);                                                                  //Can't continue without port, Exit!
  }
  
  proxy_sock=socket(AF_INET,SOCK_STREAM,0);                                   //initializing socket for proxy.
  
  if(proxy_sock<0)                                                            //Error initializing proxy socket.
  {
    perror("\nError in opening proxy socket ");
    close(proxy_sock);
    exit(1);                                                                  //Can't continue initializing proxy socket, Exit!
  }
  
  bzero((char *)&proxy_addr,sizeof(proxy_addr));                              //}Initializing proxy address and accept address objects with 0
  bzero((char *)&accept_addr,sizeof(accept_addr));                            //}
  
  proxy_addr.sin_family=AF_INET;                                              //initializing for code for the address family.
  proxy_addr.sin_addr.s_addr=INADDR_ANY;                                      //Initializing for the IP address of the host/machine.
  proxy_addr.sin_port=htons(atoi(arg_val[1]));                                //Initializing for port number using arguments.
  
  if(bind(proxy_sock,(struct sockaddr *)&proxy_addr,sizeof(proxy_addr))<0)    //Bind the Socket and Addresses and check for error
  {
    perror("\nError in binding proxy socket and proxy addresses ");
    close(proxy_sock);
    exit(1);                                                                  //Can't continue without proper binding, Exit!
  }
  
  listen(proxy_sock,BACKLOG_QUEUE);                                           //Start listening from the proxy socket.                              
  clilen=sizeof(accept_addr);                                                 //Store the size and addresses of the proxy.
  
  while(1)                                                                    //Run proxy, until killed.
  {
    char host[1024],request[512],site[2048],*trim_host=NULL,*find_conn=NULL,sitecpy[2048],*trim_port=NULL;      //}Declaring required variables.
    bool portprovided=false,proceed=false;                                                                 //}
    int port=80,i;                                                                                         //}Default port is 80.
    action_sock=accept(proxy_sock,(struct sockaddr *)&accept_addr,&clilen);                                //open socket to accept traffic from proxy.
    
    if(action_sock<0)                                                                                      //Error opening accepting socket.
    {
      perror("\nError in accepting socket on proxy ");
      close(proxy_sock);
      close(action_sock);
      exit(1);                                                                                             //Can't continue without accepting socket, Exit!
    }
    bzero(buffer,sizeof(buffer));                                              //Initializing buffer with 0.
    do                                                                         //Read while all the data is read
    {
      bzero(temp_buffer,sizeof(temp_buffer));                                                //Initialize temp buffer with zero
      n=read(action_sock,temp_buffer,sizeof(temp_buffer)-1);                                 //Read contents from the proxy through accepting socket.
      if(n!=0)                                                                      //If some content is read.
        strcat(buffer,temp_buffer);                                                      //Concatenate everything to make the entire buffer.
    }while(buffer[strlen(buffer)-1]!='\n'||buffer[strlen(buffer)-2]!='\r'||buffer[strlen(buffer)-3]!='\n'||buffer[strlen(buffer)-4]!='\r'); //Exit loop when contents are completely read.
    sscanf(buffer,"%s %s ",request,site);                                      //Store the request type and website from the request stored in buffer.
    
    if((strncmp(request,"GET",3)!=0))                                             //Check if the request is a GET request.
    {
      printf("\nNot a GET Request!");
      proceed=false;                                    
    }
    else if((strncmp(site,"http://",7)!=0))                                    //Check if the protocol is HTTP.
    {  
      printf("\nNot a HTTP Request!");
      proceed=false;
    }
    else
      proceed=true;
      
    if(proceed==true)                                                         //If valid request(checked above).
    {
      for(i=7;i<strlen(site);i++)                                            //Check if port number is provided or not.
      {
        if(site[i]==':')
        {
          memcpy(sitecpy,&site[i],strlen(site)-i);                           //Copy everything from ':' if port provided to sitecpy.
          sitecpy[strlen(site)-i]='\0';                                      //Insert null character for end of string
          trim_port=strtok(sitecpy,":/");                                    //Extract only port from sitecpy. eg: :2222/hello.php will result in 2222
          
          for(i=0;i<strlen(trim_port);i++)                   //Check if trim_port stores number and not alphabets(exceptional cases request had unusual ':'.
          {
            if(trim_port[i]<'0'||trim_port[i]>'9')
            {
              portprovided=false;
              break;
            }
            else
              portprovided=true;
          }
          break;
        }
      }
      if(portprovided==true)
        port=atoi(trim_port);                                                //Store the extracted port number from request if provided.
        
      find_conn=strstr(buffer,"keep-alive");                                 //Find keep_alive in request(buffer).
      
      if(find_conn!=NULL)                                                   //If keep-alive found.
        strncpy(find_conn,"close\r\n\r\n\r",10);                             //Change it to close.
      
      trim_host=strtok(site,"//");                                           //}Extract host name from request thorugh webstite requested.
      trim_host=strtok(NULL,"/:");                                           //}
      
      sprintf(host,"%s",trim_host);                                          //}Store host in the host variable.
      
      web_sock=socket(AF_INET,SOCK_STREAM,0);                                //initialize the socket for webserver connection.
      
      if(web_sock<0)                                                         //Error opening web socket.
      {
        perror("\nError in opening web socket ");
        close(proxy_sock);
        close(action_sock);
        close(web_sock);
        exit(1);                                                             //Can't continue without web socket initialization, Exit!.
      }
      webserver=gethostbyname(host);                                         //Store host name in the webserver.
      
      if(webserver==NULL)                                                    //If bad host or error getting host.
      {
        printf("\nBad host or error in host");
        char bad_req[]="400 : BAD REQUEST\n\nNo Such host!";                 //Send bad request to browser thorugh proxy.
        write(action_sock,bad_req,strlen(bad_req));
        close(action_sock);
        continue;                                                            //Skip loop once since error in host name.
      }
        
      bzero((char *)&web_addr,sizeof(web_addr));                             //Initializing web address object with 0.
      web_addr.sin_port=htons(port);                                          //Initialize with port number of web server to connect to.
      web_addr.sin_family=AF_INET;                                           //initializing for code for the address family.
      bcopy((char *)webserver->h_addr,(char *)&web_addr.sin_addr.s_addr,webserver->h_length);    //Set all the feilds in Web address object.
      
      if(connect(web_sock,(struct sockaddr *)&web_addr,sizeof(web_addr))<0)                      //Connect to the web server.
      {
        perror("\nError connecting to webserver ");
        char bad_req[]="400 : BAD REQUEST\n\nConnection timed out";
        write(action_sock,bad_req,strlen(bad_req));    //Write to browser for not a valid request.
        close(action_sock);                            //close the proxy socket to reset for next iteration.
        continue;                                                              //Skip loop once since error connecting to webserver.
      }
        
      write(web_sock,buffer,strlen(buffer));                                   //Write the modified request to the webserver.    
      do                                            //Read the response from web server and write to browser till response is written completely.
      {
        bzero(buffer,sizeof(buffer));
        n=read(web_sock,buffer,sizeof(buffer)-1);
        
        if(n>0)
          write(action_sock,buffer,strlen(buffer));
          
      }while(n>0);
      close(action_sock);                          //close the proxy socket to reset for next iteration.
    }
    else                                           //If not GET request or not HTTP protocol in request.
    {
      char bad_req[]="400 : BAD REQUEST\n\nOnly HTTP Protocols and GET requests accepted";
      write(action_sock,bad_req,strlen(bad_req));    //Write to browser for not a valid request.
      close(action_sock);                            //close the proxy socket to reset for next iteration.
    }
  }
  close(proxy_sock);                                  //}Free Sockets.
  close(web_sock);                                    //}
  return 0;
}
