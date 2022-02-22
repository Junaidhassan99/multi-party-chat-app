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

#include<fstream>
#include <sys/stat.h>

#define PORT 9002
#define MAXFETCHCHARARRAYLENGTH 4096
#define MAXFILESIZE 512000 //500 KB


using namespace std;

//global variables
string chatString="";
bool isRunning=true;
bool isFirstTimeRunning=true;

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

string getClientName(){

	string userName;
	cout<<"Enter Your Name: ";
	getline(cin, userName);
	
	if (userName.length()>0)
	{
    		userName[0] = std::toupper(userName[0]) ;
    		for (size_t i = 1; i < userName.length(); i++){
        		userName[i] = std::tolower(userName[i]);
		}
	}	
	
	return userName;
}

string getCurrentTime(){

	//get current time
	auto start = std::chrono::system_clock::now();
    	// Some computation here
    	auto end = std::chrono::system_clock::now();
	std::time_t end_time = std::chrono::system_clock::to_time_t(end);
	
	return string(std::ctime(&end_time)).erase(string(std::ctime(&end_time)).length()-1);
}

void messagesSender(int server_socket_id,string userName){

while(1){

	cout<<"-------------------------"<<endl;
	cout<<"User Name: "<<userName<<endl<<endl;
	
	//get message
	
	string message="";
	if(isRunning){
	
	if(!isFirstTimeRunning){
	cout<<"Your Message: ";
	getline(cin, message);
	}else{
	message="$refh";
	isFirstTimeRunning=false;
	}
	
	system("clear");
	}else{
	
	system("clear");
	cout<<"User Have Exited!"<<endl<<endl;
	//break;
	
	}
	
	
	if(message[0]!='$'){
	//concatinate message with user name
	message=userName+": "+message+" ("+getCurrentTime()+") ";
	}else if(message.substr(0, 5) == "$path"){
	message=userName+":"+message;
	}else{
	//this is a command and send it as it is to the server
	//commands are handled here
	}
	
	
	
	//convert string message to char array
	int n = message.length();
    	char char_array[n + 1];
    	strcpy(char_array, message.c_str());
    	
	
	//send message
	//3- data exchanage
	send(server_socket_id,char_array,sizeof(char_array),0);
	
	//send the file here
	
	
	if(getFilePath(message).substr(0, 5) == "$path"){
	
	
	
	fstream fileSend;
	
	fileSend.open(getFilePath(message).substr(6,message.length()), ios::in | ios::binary);
            if(fileSend.is_open()){
               //cout<<"[LOG] : File is ready to Transmit";
               
             	std::string contents((std::istreambuf_iterator<char>(fileSend)), std::istreambuf_iterator<char>());
        	//cout<<"Size of data to be transmitted = "<<contents.length()<<" Bytes."<<endl;

        	//cout<<"Sending Data..."<<endl;

        	int bytes_sent = send(server_socket_id , contents.c_str() , contents.length() , 0 );
        	//cout<<"Size of data transmitted = "<<bytes_sent<<" Bytes."<<endl;

        	//cout<<"FILE TRANSFER SUCCESSFULL"<<endl;
        	
            }
            else{
                cout<<"[ERROR] : File loading failed, Exititng";
                exit(EXIT_FAILURE);
            }
            
         fileSend.close();
		
	}else if(message.substr(0, 5) == "$getf"){
	
	fstream fileRecv;
	
	fileRecv.open("./client_data/"+message.substr(6,message.length()), ios::out | ios::trunc | ios::binary);
	
            if(fileRecv.is_open()){
               //cout<<"[LOG] : File is Opened (Recv)";
               
             	//1024*500 = 500 kb
		char buffer[MAXFILESIZE] = {};
        	int valread = read(server_socket_id , buffer, MAXFILESIZE);
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
         
	}else{
		//do nothing
	}
	
	
	if(message.substr(0, 5) != "$getf"){
	//get back whole group chat
	char fetchCharArray[MAXFETCHCHARARRAYLENGTH];
	recv(server_socket_id,&fetchCharArray,sizeof(fetchCharArray),0);
	chatString=string(fetchCharArray);
	}
	
	cout<<chatString<<endl<<endl;
	
	if(message.substr(0, 5) == "$exit"){
	
		system("clear");
		isRunning=false;
		message="";
		
	}
	
	
	
}
	
}


//THIS IS CLIENT
int main(){

	//starting code
	system("clear");
	//create data folder
	mkdir("./client_data/", 0777);
	
	cout<<"Client is Running"<<endl<<endl;
	
	//1- create socket
	int server_socket_id=socket(AF_INET,SOCK_STREAM,0);
	
	//AF_INET: IPv4
	//SOCK_STREAM: TCP
	
	if(server_socket_id==-1){
		cout<<"ERROR WHILE CREATING SCOKET"<<endl;
	}
	
	//2- connect
	struct sockaddr_in s_addr;
	s_addr.sin_family = AF_INET; //Protocol family
	s_addr.sin_port = htons(PORT); //Remember the port number in server 		//application!
	//inet_aton(“127.0.0.1”, &s_addr.sin_addr); =>Not Working
	// INADDR_ANY = Server address on local machine
	s_addr.sin_addr.s_addr = INADDR_ANY;
	
	if (connect(server_socket_id, (struct sockaddr *) &s_addr, sizeof(s_addr)) == -1) 		{
		cout<<"ERROR WHILE CONNECTING"<<endl;
	}
	
	string userName=getClientName();
	cout<<endl;
	
	system("clear");
	
	messagesSender(server_socket_id,userName);
	
	/*char clientSendMsg2[256];
	recv(socket_id,&clientSendMsg2,sizeof(clientSendMsg2),0);
	cout<<clientSendMsg2<<endl;*/
	
	close(server_socket_id);
	
	
	return 0;
}
