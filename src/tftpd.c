#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>


void sendErrorMessage(int sockfd, struct sockaddr_in client, socklen_t len, int errorCode){
	printf("Start of error function \n");
	
	char err[100];
	char* errorMessage;
	memset(err, 0, sizeof(err));
	err[1] = 5;
	err[3] = errorCode;
	// many of these we wont use for now, but it is good to have them for future implementation
	if (errorCode == 0){
		errorMessage = "This is not defined in our server";
	} else if (errorCode == 1) {
		errorMessage = "File not found";
	} else if (errorCode == 2) {
                errorMessage = "Access violation";
        } else if (errorCode == 3) {
                errorMessage = "Disk full or allocation exceeded";
        } else if (errorCode == 4) {
                errorMessage = "Illegal TFTP operation";
        } else if (errorCode == 5) {
                errorMessage = "Unknown transfer ID";
        } else if (errorCode == 6) {
                errorMessage = "File already exists";
        } else {
                errorMessage = "No such user";
        }
	strcpy(4 + err, errorMessage);
	sendto(sockfd, err, sizeof(err), 0, (struct sockaddr *)&client, len);
	printf("End of error function \n");
}


int main (int argc, char *argv[])
{
    typedef enum { false, true } bool;
    if (argc < 3) {
	printf("You are missing some arguments \n");
	printf("You should be sending port to listen on and directory containing the files to server \n");
	return -1;
    }   
    short aaa = 129;
    printf("short: %hi \n ", aaa); 	
    printf("starting \n");
    int sockfd;
    struct sockaddr_in server, client;
    char message[512];
    int portNumber = atoi(argv[1]);
    printf("portNumber: %d \n", portNumber);

    // Create and bind a UDP socket.
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;	
 
    // Network functions need arguments in network byte order instead 
    // of host byte order. The macros htonl, htons convert the values.
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(portNumber);
    int bindStatus = bind(sockfd, (struct sockaddr *) &server, (socklen_t) sizeof(server));
    
    if (bindStatus < 0){ // bind failed and the execution of the program is terminated
        //error
	printf("bind failed \n");
	return -1;
    }
    for (;;) {
	printf("Waiting for a RRQ code\n");
        // Receive up to one byte less than declared, because it will
        // be NUL-terminated later.
        socklen_t len = (socklen_t) sizeof(client);
        ssize_t n = recvfrom(sockfd, message, sizeof(message) - 1,
                             0, (struct sockaddr *) &client, &len);
	
	//nr 1 and 2 are the opcode
	int opcode = message[1];
	if (opcode == 1) { //opced is a RRQ code
		
		//printf("opcode: %d \n", opcode);
		char* fileName = 2 + message;
       		printf("FileName: %s\n", fileName);
		int fileNameLength = strlen(fileName);
		char* mode = 2 + fileNameLength + 1 + message;
		printf("mode: %s\n", mode);
		/*if (strcmp(mode, "mail") != 0) {
			printf("this server will not accept RRQ with mode mail \n");
			sendErrorMessage(sockfd, client, len, 0);
			continue;
		}*/
		message[n] = '\0';
        	fflush(stdout);
		char* ipNumber = inet_ntoa(client.sin_addr);
		int port = ntohs(client.sin_port);
		printf("file \"%s\" requested frome %s:%d \n", fileName, ipNumber, port);  
		
		
		char pathToFile[50];
		strcpy(pathToFile, argv[2]);
		strcat(pathToFile, "/");
		strcat(pathToFile, fileName);
		printf("PathToFile: %s \n", pathToFile);
		FILE * file;
		file = fopen(pathToFile, "r");
		if (file == NULL) {
			printf("Error with file reading \n");
			sendErrorMessage(sockfd, client, len, 1);
			continue;
		}
		unsigned short  currentBlockNumber = 1;
		unsigned short  ackBlockNumber;
		//sending data for loop	
		for(;;) {
			printf("Sending next message \n");
			char messageToSend[512];
			memset(messageToSend, 0, sizeof messageToSend);
			memset(message, 0, sizeof message);
			messageToSend[0] = 0;
        		messageToSend[1] = 3;	 
			messageToSend[2] = (currentBlockNumber >> 8) & 0xff;		//TODO: skoða með að breyta þessu
			messageToSend[3] = (currentBlockNumber) & 0xff;
			int fileSize = fread(messageToSend + 4, 1, 512, file);
			if(fileSize < 512) {
				sendto(sockfd, messageToSend, fileSize + 4, 0, (struct sockaddr *)&client, len);
				break;
				//TODO: þetta er síðasta skilaboðið 
				//Þurfum að hætta að senda gögn
			}
			bool sameAckBlockNumber = true;
			while(sameAckBlockNumber) {
				ssize_t returnCode = sendto(sockfd, messageToSend, fileSize + 4, 0, (struct sockaddr *)&client, len);
				if(returnCode < 0 ) {
					//TODO: 
				}
	
				ssize_t ack_return_code = recvfrom(sockfd, &message, sizeof(message), 0, (struct sockaddr *)&client, &len);
				if(ack_return_code == 0) {
					//TODO: no message
				}
				if(ack_return_code < 0) {
					//TODO: ERROR
				}
				opcode = message[1];
				ackBlockNumber = (message[2] << 8);
				ackBlockNumber = ackBlockNumber | message[3];
				printf("ackBlockNumber = %hu \n", ackBlockNumber);
				printf("currentBlockNu = %hu \n", currentBlockNumber);
				if(opcode == 4) { // 4 means that this is a ACK packet
					currentBlockNumber++;
					sameAckBlockNumber = false;
					/*if (ackBlockNumber == currentBlockNumber) { // This should be true
						currentBlockNumber++;
						sameAckBlockNumber = falser
					}*/
				}
			}
			
		}
	}
	else if (opcode == 2) { //2 is a WRQ which is not allowed on our server
		printf("You may not upload files with this server \n");
		int errorCode = 0;
		sendErrorMessage(sockfd, client, len, errorCode);
	}

    }
	
}

