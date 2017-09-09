#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

/*
void sendFile (int sockfd, struct sockaddr_in client, socklen_t len, FILE* file, char* message) {
    printf("start of sendFile function \n");
    //for(;;) {
	char messageToSend[512];
        memset(messageToSend, 0, sizeof(messageToSend));
        //memset(message, 0, sizeof(message));
        messageToSend[0] = 0;
        messageToSend[1] = 3;
        messageToSend[2] = 0;
        messageToSend[3] = 1;
        int fileSize = fread(messageToSend + 4, 1, 512, file);
	printf("middle of for loop \n");
        ssize_t returnCode = sendto(sockfd, messageToSend, fileSize + 4, 0, (struct sockaddr *)&client, len);
	printf("after sendto \n");
        ssize_t ack_return_code = recvfrom(sockfd, &message, sizeof(message), 0, (struct sockaddr *)&client, &len);
        printf("after recvfrom \n");
	//printf("ackcode = %d \n", message[1]);
	//sendto(sockfd, message, (size_t) n, 0,
        //       (struct sockaddr *) &client, len);
        printf("end of for loop \n");
    //}

}
*/
int main (int argc, char *argv[])
{
    	
    printf("starting \n");
    int sockfd;
    struct sockaddr_in server, client;
    char message[512];

    // Create and bind a UDP socket.
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    
    // Network functions need arguments in network byte order instead
    // of host byte order. The macros htonl, htons convert the values.
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(59545);
    int bindStatus = bind(sockfd, (struct sockaddr *) &server, (socklen_t) sizeof(server));
    
    if(bindStatus < 0){
        //error
	printf("bind failed \n");
    }
    
    for (;;) {
		printf("beginning of for loop \n");
        // Receive up to one byte less than declared, because it will
        // be NUL-terminated later.
        socklen_t len = (socklen_t) sizeof(client);
	
        ssize_t n = recvfrom(sockfd, message, sizeof(message) - 1,
                             0, (struct sockaddr *) &client, &len);
 	unsigned char* address = (unsigned char *)&client.sin_addr.s_addr;
	printf("client stuff: %d\n", address[0]);

	//nr 1 and 2 are the opcode
	int opcode = message[1];
	printf("opcode: %d \n", opcode);
	char* fileName = 2 + message;
       	printf("FileName: %s\n", fileName);
	int fileNameLength = strlen(fileName);
	char* mode = 2 + fileNameLength + 1 + message;
	printf("mode: %s\n", mode);
	message[n] = '\0';
       	fprintf(stdout, "Received:\n%s\n", message);
        fflush(stdout);
		
        // convert message to upper case.
        //for (int i = 0; i < n; ++i) {
        //    message[i] = toupper(message[i]);
        //}
	//printf("message: %s \n", message);
	
	
	char* pathToFile = "data/example_data2";
	FILE * file;
	file = fopen(pathToFile, "r");
	for(;;) {
		char messageToSend[512];
		memset(messageToSend, 0, sizeof messageToSend);
		memset(message, 0, sizeof message);
		messageToSend[0] = 0;
        	messageToSend[1] = 3;
        	messageToSend[2] = 0;
        	messageToSend[3] = 1;
		int fileSize = fread(messageToSend + 4, 1, 512, file);
		
		ssize_t returnCode = sendto(sockfd, messageToSend, fileSize + 4, 0, (struct sockaddr *)&client, len);
		ssize_t ack_return_code = recvfrom(sockfd, &message, sizeof(message), 0, (struct sockaddr *)&client, &len);
		printf("ackcode = %d \n", message[1]);
		//sendto(sockfd, message, (size_t) n, 0,
        	//       (struct sockaddr *) &client, len);
		printf("end of for loop \n");
		//sendFile(sockfd, client, len, file, message);
	}
    }
	
}
