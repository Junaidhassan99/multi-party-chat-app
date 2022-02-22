#include<iostream>
#include<cstring>
#include<stdio.h>
#include<stdlib.h>
#include<string>
#include <algorithm>

#include <chrono>
#include <ctime>  


#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>

#include <pthread.h>

#include<fstream>
#include <sys/stat.h>

#define PORT 9002
#define TOTALCLIENTS 5
#define MAXMESSAGECHARARRAYLENGTH 512
#define MAXFILESIZE 512000 //500 KB

using namespace std;

//global variables
string fetchString="";

string getCurrentTime(){

	//get current time
	auto start = std::chrono::system_clock::now();
    	// Some computation here
    	auto end = std::chrono::system_clock::now();
	std::time_t end_time = std::chrono::system_clock::to_time_t(end);
	
	return string(std::ctime(&end_time)).erase(string(std::ctime(&end_time)).length()-1);
}

string getFilePath(string filePathString){
	//string filePath="$path ./send/q.txt";
	
	reverse(filePathString.begin(), filePathString.end());
	
	string pathOfFile="";
	for(int i=0;i<filePathString.length();i++){
		if(filePathString[i]!=':'){
			pathOfFile+=filePathString[i];
		}else{
			break;
		}
	}
	
	reverse(pathOfFile.begin(), pathOfFile.end());
	

    	return pathOfFile;
	
}

string getFileNameFromPath(string filePath){
	//string filePath="$path ./send/q.txt";
	
	reverse(filePath.begin(), filePath.end());
	
	string nameOfFile="";
	for(int i=0;i<filePath.length();i++){
		if(filePath[i]!='/'){
			nameOfFile+=filePath[i];
		}else{
			break;
		}
	}
	
	reverse(nameOfFile.begin(), nameOfFile.end());
	

    	return nameOfFile;
	
}

string getUserNameFromPath(string recvString){

	string nameOfUser="";
	for(int i=0;i<recvString.length();i++){
		if(recvString[i]!=':'){
			nameOfUser+=recvString[i];
		}else{
			break;
		}
	}
	
	return nameOfUser;
	
}

void messagesReciever(int client_socket_id){

while(1){

	//system(cls) do not work on linux
	//system("clear");
	
	//5- data exchanage
	char clientSendMsg[MAXMESSAGECHARARRAYLENGTH];
	recv(client_socket_id,&clientSendMsg,sizeof(clientSendMsg),0);
	
	
	
	//message starting with $ is a command
	if(string(clientSendMsg)!="$refh"&&clientSendMsg[0]!='$'&&getFilePath(string(clientSendMsg)).substr(0, 5) != "$path"){
	
	fetchString=fetchString+"\n"+string(clientSendMsg);
	
	cout<<endl<<fetchString<<endl;
	
	}else if(getFilePath(string(clientSendMsg)).substr(0, 5) == "$path"){
	fetchString=fetchString+"\n"+getUserNameFromPath(string(clientSendMsg))+": \""+getFileNameFromPath(string(clientSendMsg))+"\" file has been sent ("+getCurrentTime()+")";
	}else{
	//do nothing and code will automatically refresh chat
	}
	
	
	//recv the file here
	
	
	if(getFilePath(string(clientSendMsg)).substr(0, 5) == "$path"){
	fstream fileRecv;
	
	fileRecv.open("./server_data/"+getFileNameFromPath(string(clientSendMsg)), ios::out | ios::trunc | ios::binary);
	
            if(fileRecv.is_open()){
               //cout<<"[LOG] : File is Opened (Recv)";
               
             	//1024*500 = 500 kb
		char buffer[MAXFILESIZE] = {};
        	int valread = read(client_socket_id , buffer, MAXFILESIZE);
        	//cout<<"Data received = "<<valread<<" bytes"<<endl;
        	//cout<<"Saving data to file."<<endl;
            
        	fileRecv<<buffer;
        	//cout<<"FILE SAVE SUCCESSFULL"<<endl;
        	
            }
            else{
                cout<<"[ERROR] : File loading failed (Recv)";
                exit(EXIT_FAILURE);
            }
            
         fileRecv.close();
		
	}else if(string(clientSendMsg).substr(0, 5) == "$getf"){
	
		fstream fileSend;
	
	fileSend.open("./server_data/"+string(clientSendMsg).substr(6,string(clientSendMsg).length()), ios::in | ios::binary);
            if(fileSend.is_open()){
               //cout<<"[LOG] : File is ready to Transmit";
               
             	std::string contents((std::istreambuf_iterator<char>(fileSend)), std::istreambuf_iterator<char>());
        	//cout<<"Size of data to be transmitted = "<<contents.length()<<" Bytes."<<endl;

        	//cout<<"Sending Data..."<<endl;

        	int bytes_sent = send(client_socket_id , contents.c_str() , contents.length() , 0 );
        	//cout<<"Size of data transmitted = "<<bytes_sent<<" Bytes."<<endl;

        	//cout<<"FILE TRANSFER SUCCESSFULL"<<endl;
        	
            }
            else{
                cout<<"[ERROR] : File loading failed, Exititng";
                exit(EXIT_FAILURE);
            }
            
         fileSend.close();
         
	}else if(string(clientSendMsg).substr(0, 5) == "$exit"){
		
		close(client_socket_id);
	
	}else{
		//do nothing
	}
	
	if(string(clientSendMsg).substr(0, 5) != "$getf"){
	//convert string message to char array
	int n = fetchString.length();
    	char char_array[n + 1];
    	strcpy(char_array, fetchString.c_str());
	
	//send grp chat to client that typed message
	send(client_socket_id,char_array,sizeof(char_array),0);
	}
	
}
	
}

void* recieverThreadFunction(void *mSocket_id) {



	long socket_id=(long) mSocket_id;

	messagesReciever(socket_id);
	
	return 0;
	
	
}


//THIS IS SERVER
int main(){

	//starting code
	system("clear");
	//create data folder
	mkdir("./server_data/", 0777);
	
	cout<<"Server is Running"<<endl<<endl;
	
	//1- create socket
	int server_socket_id=socket(AF_INET,SOCK_STREAM,0);
	
	//AF_INET: IPv4
	//SOCK_STREAM: TCP
	
	if(server_socket_id==-1){
		cout<<"ERROR WHILE CREATING SCOKET"<<endl;
	}
	
	//this prevents error binding for rerun
	int enable = 1;
	if (setsockopt(server_socket_id, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0){
    		cout<<("setsockopt(SO_REUSEADDR) failed");
    	}
	
	//2- bindind socket
	//bind socket to a port of address defined by this structure
	struct sockaddr_in addr;

	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_family = AF_INET; //IPv4
	addr.sin_port = htons(PORT); //Bind to port 9002
	
	if (bind(server_socket_id, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
		cout<<"ERROR WHILE BINDING SCOKET"<<endl;
	}
	
	//3- listening
	//TOTALCLIENTS //max no of clients

	if (listen(server_socket_id, TOTALCLIENTS) == -1) {
		cout<<"ERROR WHILE LISTENING"<<endl;

	}
	
	/*int yes=1;

	if (setsockopt(server_socket_id, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
    		cout<<"setsockopt";
    		exit(1);
	}*/
	
	//4- accept
	long client_socket_id[TOTALCLIENTS];
	
	
	//For changing TOTALCLIENTS must add approperate number of object here
	pthread_t tid0;
  	pthread_t tid1;
  	pthread_t tid2;
  	pthread_t tid3;
  	pthread_t tid4;
  	pthread_t * pthreads[] = {&tid0,&tid1,&tid2,&tid3,&tid4};
  	//pthread_t * pthreads[TOTALCLIENTS];

  	for (int i = 0; i < TOTALCLIENTS; i++){
  	
  	struct sockaddr_in cliaddr;
	socklen_t cliaddr_len = sizeof(cliaddr);

	client_socket_id[i] = accept(server_socket_id, (struct sockaddr *) &cliaddr, &cliaddr_len);
	if (client_socket_id[i] == -1) {
	// an error occurred

	}

        pthread_create(pthreads[i],NULL,recieverThreadFunction,(void *) client_socket_id[i]);
  	}
	
	
	/*char clientSendMsg2[MAXMESSAGECHARARRAYLENGTH]="Hello Client!";
	send(client_socket_id,clientSendMsg2,sizeof(clientSendMsg2),0);*/
	
	close(server_socket_id);
	
	for (int i = 0; i < TOTALCLIENTS; i++){
	close(client_socket_id[i]);
	}
	
	return 0;
}
