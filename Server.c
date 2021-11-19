#define BUFSIZE 1000000

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void handle_socket(int fd);
struct {
    char *ext;
    char *filetype;
} extensions [] = {
    {"gif", "image/gif" },
    {"jpg", "image/jpeg"},
    {"jpeg","image/jpeg"},
    {"png", "image/png" },
    {"zip", "image/zip" },
    {"gz",  "image/gz"  },
    {"tar", "image/tar" },
    {"htm", "text/html" },
    {"html","text/html" },
    {"exe","text/plain" },
    {0,0} };
    
int main(int argc, char **argv)
{
	int i, pid, listenedfd, socketfd;
	int yes = 1;
	socklen_t length;
	static struct sockaddr_in cln_addr;
	static struct sockaddr_in svr_addr;
	
	signal(SIGCHLD, SIG_IGN); //avoid zombie
	if ( (listenedfd = socket(AF_INET, SOCK_STREAM, 0) ) <0 ){
		printf("socket_err\n");
		exit(3);
	}
	svr_addr.sin_family 	 = AF_INET;
	svr_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	svr_addr.sin_port = htons(8000);	
	if (setsockopt(listenedfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        fprintf(stderr,"setsockopt\n");
        exit(1);
    }

    if (bind(listenedfd, (struct sockaddr *)&svr_addr,sizeof(svr_addr))<0){
		printf("bind error\n");
 		exit(3);
	}
   if (listen(listenedfd,64)<0)	{
		printf("listen error\n");
        exit(3);
	}
	
  while(1) {
        length = sizeof(cln_addr);
        /* 等待client */
        if ((socketfd = accept(listenedfd, (struct sockaddr *)&cln_addr, &length))<0){
			printf("accept error\n");
            exit(3);
		}
        /* build child process to handle  */
        if ((pid = fork()) < 0) {
            exit(3);
        } else {
            if (pid == 0) {  // child 
                close(listenedfd);
                handle_socket(socketfd);
            } else { // parent 
                close(socketfd);
            }
        }
    }
}	
	

void handle_socket(int fd)
{
    int j, file_fd, buflen, len;
    long i, ret;
    char * fstr;
    static char buffer[BUFSIZE+1];
    ret = read(fd,buffer,BUFSIZE);   /* 讀取瀏覽器要求 */
	write(1,buffer,BUFSIZE);
    if (ret==0||ret==-1)
        exit(3);
    else if (ret>0&&ret<BUFSIZE)
        buffer[ret] = '\0';
    else
        buffer[0] = 0;
        
        
    if (!strncmp(buffer,"GET ",4)){
        for(i=4;i<BUFSIZE;i++) {
            if(buffer[i] == ' ') {
                buffer[i] = 0;
                break;
                }
      	  }
        for (j=0;j<i-1;j++){
            if (buffer[j]=='.'&&buffer[j+1]=='.')
                exit(3);
        	}
        
        if (!strncmp(&buffer[0],"GET /\0",6)||!strncmp(&buffer[0],"get /\0",6) ){
            strcpy(buffer,"GET /index.html\0");
            }
            
        buflen = strlen(buffer);
        
        fstr = (char *)0;
        for(i=0;extensions[i].ext!=0;i++) {
        
            len = strlen(extensions[i].ext);
            if(!strncmp(&buffer[buflen-len], extensions[i].ext, len)) {
                fstr = extensions[i].filetype;
                break;
            }
        }
        if(fstr == 0) {
            fstr = extensions[i-1].filetype;
        }
        if((file_fd=open(&buffer[5],O_RDONLY))==-1){
            write(fd, "Open File Failedly", 19);
            }
        sprintf(buffer,"HTTP/1.0 200 OK\r\nContent-Type: %s\r\n\r\n", fstr);
        write(fd,buffer,strlen(buffer));
        while ((ret=read(file_fd, buffer, BUFSIZE))>0) {
            	write(fd,buffer,ret);
        }
    }
    else if(!strncmp(buffer,"POST ",5)||!strncmp(buffer,"post ",5)) {
        char *tmp;
	    char filename[2048];
		int i = 0;
        char *tmp1;
        char body[BUFSIZE];
		char  path[100] = "./file/";
	    tmp1 = strstr(buffer,"filename=");
	    strcpy(filename, tmp1+10);
	    for(int i = 0; i < strlen(filename); i++){
		    if(filename[i]=='"'){
			    filename[i] = 0;
			    break;
		    }
	   }
		strcat(path,filename);
        tmp = strstr(tmp1, "\n\r\n");
       	tmp+=3;
       	
		FILE *fp = fopen(path,"a+");
        while (*tmp!='\n'||*(tmp+1)!='-'||*(tmp+2)!='-'){
            fprintf(fp,"%c",*tmp);
            tmp++; 
        }
        fclose(fp);
        
        write(fd, "Upload  Successful!!",20);

		
    }

    exit(1);
}

	
	
