/****************************************************************************************************************************************************************
  Author: 
  Dhairya Dhondiyal.
  
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
#include<sys/ioctl.h>

#define BACKLOG_QUEUE 1                                                      //Number of connections to listen.

int main(int arg_cnt,char *arg_val[])
{
  fd_set fd;
  struct timeval tv;
  int proxy_sock,action_sock,clilen,n,web_sock,ftp_sock,sel_ret;                               //Declaring all sockets.        
  char buffer[655350],temp_buffer[10];                                                          //Declaring read,write buffer.          
  struct sockaddr_in proxy_addr,accept_addr,web_addr,ftp_addr;                         //Declaring internet addresses object of structures for proxy and webserver.
  struct hostent *webserver,*webserverftp;
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
    char code[1000],host[1024],request[512],site[2048],*trim_host=NULL,*find_conn=NULL,sitecpy[2048],*trim_port=NULL,path[2048],RETR[2048];//}Declaring required variables.
    bool isfile=false,isftp=false,portprovided=false,proceed=false;                                        //}
    int port=80,i;                                                                                         //}Default port is 80.
    action_sock=accept(proxy_sock,(struct sockaddr *)&accept_addr,&clilen);                                //open socket to accept traffic from proxy.
    
    if(action_sock<0)                                                                                      //Error opening accepting socket.
    {
      perror("\nError in accepting socket on proxy ");
      close(proxy_sock);
      close(action_sock);
      exit(1);                                                                   //Can't continue without accepting socket, Exit!
    }
    bzero(buffer,sizeof(buffer));                                              //Initializing buffer with 0.
    FD_ZERO(&fd);                                                              //}
    FD_SET(action_sock,&fd);                                                   //}Initialise fd_set values
    do                                                                         //Read until all the data is read
    {
      n=0;                                                                       
      tv.tv_sec=1;                                                               //} Timeout values for select
      tv.tv_usec=0;                                                              //}
      bzero(temp_buffer,sizeof(temp_buffer));                                                //Initialize temp buffer with zero
      sel_ret=select(action_sock+1,&fd,NULL,NULL,&tv);                            //Wait for data
      if(sel_ret)
      {
        n=read(action_sock,temp_buffer,sizeof(temp_buffer)-1);                                 //Read contents from the proxy through accepting socket.
        strcat(buffer,temp_buffer);                                                      //Concatenate everything to make the entire buffer.
      }
    }while(n>0);                                                                 //Exit loop when contents are completely read.*/
    sscanf(buffer,"%s %s ",request,site);                                      //Store the request type and website from the request stored in buffer.
    if((strncmp(request,"GET",3)!=0))                                             //Check if the request is a GET request.
      printf("\nNot a GET Request!");                                    
    else if((strncmp(site,"http://",7)!=0)&&(strncmp(site,"ftp://",6)!=0))                                    //Check if the protocol is HTTP.
      printf("\nNot a HTTP or FTP Request!");
    else
      proceed=true;  
    if(proceed==true)                                                         //If valid request(checked above).
    {
      if(strncmp(site,"ftp://",6)==0)
      {
        isftp=true;
        strcpy(path,site);
      }
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
      if(isftp)                                                              //If ftp link
      {
          if(portprovided==true)
            trim_host=strtok(NULL,"/");
          trim_host=strtok(NULL,"\0");
          if(trim_host!=NULL)                                                //Change variables to point to directory
          {
            sprintf(path,"CWD /%s\r\n",trim_host);
            sprintf(RETR,"RETR /%s\r\n",trim_host);
          }
          else                                                              //Change variables to point to parent directory
          {
            strcpy(path,"CWD /\r\n");
            strcpy(RETR,"RETR /\r\n");
          }  
        ftp_sock=socket(AF_INET,SOCK_STREAM,0);                                //initialize the socket for webserver connection.
        if(ftp_sock<0)                                                         //Error opening web socket.
        {
          perror("\nError in opening control socket ");
          close(proxy_sock);
          close(action_sock);
          close(ftp_sock);
          exit(1);                                                             //Can't continue without web socket initialization, Exit!.
        }
        webserverftp=gethostbyname(host);                                         //Store host name in the webserver.
        if(webserverftp==NULL)                                                    //If bad host or error getting host.
        {
          printf("\nBad host or error in host");
          char bad_req[]="400 : BAD REQUEST\n\nNo Such host!";                 //Send bad request to browser thorugh proxy.
          write(action_sock,bad_req,strlen(bad_req));
          close(action_sock);
          close(ftp_sock);
          continue;                                                            //Skip loop once since error in host name.
        }
        bzero((char *)&ftp_addr,sizeof(ftp_addr));                             //Initializing web address object with 0.
        ftp_addr.sin_port=htons(21);                                          //Initialize with control port.
        ftp_addr.sin_family=AF_INET;                                           //initializing for code for the address family.
        bcopy((char *)webserverftp->h_addr,(char *)&ftp_addr.sin_addr.s_addr,webserverftp->h_length);    //Set all the feilds in Web address object.
        if(connect(ftp_sock,(struct sockaddr *)&ftp_addr,sizeof(ftp_addr))<0)                      //Connect to the web server.
        {
          perror("\nError connecting to ftp server");
          char bad_req[]="400 : BAD REQUEST\n\nConnection timed out";
          write(action_sock,bad_req,strlen(bad_req));    //Write to browser for not a valid request.
          close(action_sock);                            //close the proxy socket to reset for next iteration.
          close(ftp_sock);
          continue;                                                              //Skip loop once since error connecting to webserver.
        }
        do                                            //Read the response from web server and write to browser till response is written completely.
        {
          bzero(buffer,sizeof(buffer));
          FD_ZERO(&fd);                                                                //Initialize fd_set values
          FD_SET(ftp_sock,&fd);                                                        //
          do                                                                         //Read until all the data is read
          {
            n=0;  
            tv.tv_sec=1;                                                            //}Initialize timeout variables for select
            tv.tv_usec=0;                                                         //}
            bzero(temp_buffer,sizeof(temp_buffer));                                                //Initialize temp buffer with zero
            sel_ret=select(ftp_sock+1,&fd,NULL,NULL,&tv);                            //Wait for data
            if(sel_ret)
            {
              n=read(ftp_sock,temp_buffer,sizeof(temp_buffer)-1);                                 //Read contents from the proxy through ftp socket.
              strcat(buffer,temp_buffer);                                                      //Concatenate everything to make the entire buffer.
            }
          }while(n>0);
          //sscanf(buffer,"%s ",code);
          code[0]=buffer[0];                                                                    //}
          code[1]=buffer[1];                                                                    //}Store server return codes in variable code.
          code[2]=buffer[2];                                                                    //}
          if(buffer[3]>='0'&&buffer[3]<='9')                                                    //}
          {                                                                                     //}
            code[3]=buffer[3];                                                                  //}  
            code[4]='\0';                                                                       //}
          }                                                                                     //}
          else                                                                                  //}
            code[3]='\0';                                                                       //}
          
          if(strcmp(code,"220")==0)                     //Conditions for each return codes checked and handled.                                             
            write(ftp_sock,"USER anonymous\r\n",16);
          else if(strcmp(code,"331")==0) 
            write(ftp_sock,"PASS anonymous@example.com\r\n",28);
          else if(strcmp(code,"230")==0)
            write(ftp_sock,path,strlen(path));
          else if(strcmp(code,"10054")==0||strcmp(code,"10060")==0||strcmp(code,"10061")==0||strcmp(code,"10068")==0||strcmp(code,"530")==0||strcmp(code,"430")==0)
          {
            write(ftp_sock,"QUIT\r\n",6);
            char bad_req[]="400 : Authentication or connection issues";                 
            write(action_sock,bad_req,strlen(bad_req));                                //Send Connection issues to browser thorugh proxy.
            close(action_sock);                          //close the accept socket to reset for next iteration.
            close(web_sock);                             //close the webserver socket to reset for next iteration.
            close(ftp_sock);                            ////close the ftp socket to reset for next iteration.
            goto skip;   
          }
          else if(strcmp(code,"250")==0||strcmp(code,"550")==0)
          {
            if(strcmp(code,"550")==0)
            {
              isfile=true;
              while(RETR[strlen(RETR)-3]=='/')
              {
                RETR[strlen(RETR)-3]='\r';
                RETR[strlen(RETR)-2]='\n';
                RETR[strlen(RETR)-1]='\0';
              }
            }
            write(ftp_sock,"PASV\r\n",6);
          }
          if(strcmp(code,"227")==0)                                                //Extract port number from passive return code
          {
            char trim_port[1024];
            trim_host=strtok(buffer,"(");
            trim_host=strtok(NULL,")");
            sprintf(trim_port,"%s",trim_host);
            trim_host=strtok(trim_port,",");
            trim_host=strtok(NULL,",");
            trim_host=strtok(NULL,",");
            trim_host=strtok(NULL,",");
            trim_host=strtok(NULL,",");
            port=atoi((char*)trim_host)*256;
            trim_host=strtok(NULL,",");
            port+=atoi((char*)trim_host);
          }
        }while(strcmp(code,"227")!=0);
      }
      web_sock=socket(AF_INET,SOCK_STREAM,0);                                //initialize the socket for webserver connection.
      if(web_sock<0)                                                         //Error opening web socket.
      {
        perror("\nError in opening web socket ");
        close(proxy_sock);
        close(action_sock);
        close(web_sock);
        close(ftp_sock);
        exit(1);                                                             //Can't continue without web socket initialization, Exit!.
      }
      webserver=gethostbyname(host);                                         //Store host name in the webserver.
      if(webserver==NULL)                                                    //If bad host or error getting host.
      {
        printf("\nBad host or error in host");
        char bad_req[]="400 : BAD REQUEST\n\nNo Such host!";                 //Send bad request to browser thorugh proxy.
        write(action_sock,bad_req,strlen(bad_req));
        close(action_sock);
        close(web_sock);
        close(ftp_sock);
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
        close(web_sock);
        close(ftp_sock);
        continue;                                                              //Skip loop once since error connecting to webserver.
      }
      if(isftp)                                                                //if ftp link
      {
        if(isfile)                                                             //If file retreval requested
          write(ftp_sock,RETR,strlen(RETR));
        else                                                                   //else List files of directory
          write(ftp_sock,"LIST -l\r\n",9);
        bzero(buffer,sizeof(buffer));
        FD_ZERO(&fd);                                                          //}Initialize fd_set values
        FD_SET(ftp_sock,&fd);                                                  //}
        do                                                                         //Read until all the data is read
        {
          n=0;
          tv.tv_sec=1;                                                            //}Initilize timout values for select
          tv.tv_usec=0;                                                           //}
          bzero(temp_buffer,sizeof(temp_buffer));                                                //Initialize temp buffer with zero
          sel_ret=select(ftp_sock+1,&fd,NULL,NULL,&tv);                                        //Wait for the data
          if(sel_ret)
          {
            n=read(ftp_sock,temp_buffer,sizeof(temp_buffer)-1);                                 //Read contents from the proxy through accepting socket.
            strcat(buffer,temp_buffer);                                                      //Concatenate everything to make the entire buffer.
          }
        }while(n>0);
        code[0]=buffer[0];                                                                //}
        code[1]=buffer[1];                                                                //}
        code[2]=buffer[2];                                                                //}Set return code to variable code.
        if(buffer[3]>='0'&&buffer[3]<='9')                                                //}
        {                                                                                 //}
          code[3]=buffer[3];                                                              //}
          code[4]='\0';                                                                   //}
        }                                                                                 //}
        else                                                                              //}
          code[3]='\0';                                                                   //}
        if(strcmp(code,"550")==0||strcmp(code,"421")==0||strcmp(code,"425")==0||strcmp(code,"450")==0||strcmp(code,"451")==0||strcmp(code,"452")==0||strcmp(code,"502")==0||strcmp(code,"501")==0||strcmp(code,"504")==0||strcmp(code,"534")==0||strcmp(code,"551")==0||strcmp(code,"552")==0||strcmp(code,"553")==0||buffer[0]=='\0')
        {
          write(ftp_sock,"QUIT\r\n",6);
          char bad_req[]="400 : BAD REQUEST\n\n";                 //Send bad request to browser thorugh proxy.
          write(action_sock,bad_req,strlen(bad_req));
          write(action_sock,buffer,strlen(buffer));
          close(action_sock);                          //close the proxy socket to reset for next iteration.
          close(web_sock);                             //close the webserver socket to reset for next iteration.
          close(ftp_sock);                            ////close the ftp socket to reset for next iteration.
          continue;   
        }
      }
      else
      {
        write(web_sock,buffer,strlen(buffer));          //Write the modified request to the webserver.
      }
      FD_ZERO(&fd);                                      //}Initialize fd_set values.
      FD_SET(web_sock,&fd);                              //}            
      do                                                 //Read the response from web server and write to browser till response is written completely.
      {
        n=0;
        tv.tv_sec=1;                                      //}Set timeout values for select 
        tv.tv_usec=0;                                     //}
        bzero(temp_buffer,sizeof(temp_buffer));                                                //Initialize temp buffer with zero
        sel_ret=select(web_sock+1,&fd,NULL,NULL,&tv);                                          //Wait for input.
        if(sel_ret)
        {
          n=read(web_sock,temp_buffer,sizeof(temp_buffer)-1);                                  //Read data from web server
          write(action_sock,temp_buffer,strlen(temp_buffer));                                  //write data to browser through proxy.
        }
      }while(n>0);                                                                           //Exit when data is completely read.
      if(isftp)                                                                    //If ftp link
        write(ftp_sock,"QUIT\r\n",6);                                              //Send quit request to the ftp server
      close(action_sock);                          //close the proxy socket to reset for next iteration.
      close(web_sock);                             //close the webserver socket to reset for next iteration.
      close(ftp_sock);                            //close the ftp socket to reset for next iteration.
      skip:;
    }
    else                                           //If not GET request or not HTTP protocol in request.
    {
      char bad_req[]="400 : BAD REQUEST\n\nOnly HTTP and FTP Protocols and GET requests accepted";
      write(action_sock,bad_req,strlen(bad_req));    //Write to browser for not a valid request.
      close(action_sock);                            //close the proxy socket to reset for next iteration.
    }
  }
  close(proxy_sock);                                  //Free Socket.
  return 0;
}
