/*
 * Phillip Ryan
 *
 * Socket.cpp
 *
 */

#include "Socket.h"

/*
 * Helper functions which validate if string is an ip address
 */
bool validateIpAddress(const string &ipaddress){
	struct sockaddr_in sa;
	return inet_pton(AF_INET, ipaddress.c_str(), &(sa.sin_addr));
}

int Socket::openConnect(string hostname, string port){
	int retVal = 0;
	hp = NULL;

	log = Log::getInstance(); //Get Singelton

	string tmp = "Attempting to connect to ";
	tmp += hostname;
	tmp += " ";
	tmp += port;
	log->output(tmp, 3);

	//Create socket
	if( (sock = socket(AF_INET, SOCK_STREAM, 0)) < 0 ){
		tmp = "Error when opening socket: ";
		tmp += strerror(errno);
		log->output(tmp, 3);
		retVal = errno;
	}

	char *host = new char[hostname.length()+1];
	strcpy(host, hostname.c_str());

	if( !validateIpAddress(hostname) ){
		//Resolve the hostname to an IP address if neccesary
		if( (hp = gethostbyname(host)) == NULL ){
			tmp = "Error when resolving ip: ";
			tmp += strerror(errno);
			log->output(tmp, 3);
			retVal = errno;
		}
	}

	//Setup socket address
	memset(&addr, '0', sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(stoi(port));

	if( hp == NULL ){
		ipAddress = host;
		//Convert IP address from text to binary
		if( inet_pton(AF_INET, host, &addr.sin_addr) <= 0 ){
			tmp = "Error when converting host to binary: ";
			tmp += strerror(errno);
			log->output(tmp, 3);
			retVal = errno;
		}
	}
	else{
		ipAddress = hp->h_addr_list[0];
		//Convert IP address from text to binary
		if( inet_pton(AF_INET, hp->h_addr_list[0], &addr.sin_addr) <= 0 ){
			tmp = "Error when converting hp to binary: ";
			tmp += strerror(errno);
			log->output(tmp, 3);
			retVal = errno;
		}
	}

	//Establish a connection with the server
	if( connect(sock, (struct sockaddr*)&addr, sizeof(addr)) != 0 ){
		tmp = "Error when connecting to server: ";
		tmp += strerror(errno);
		log->output(tmp, 3);
		retVal = errno;
	}

	return retVal;
}

int Socket::openListen(string port){
	int retVal = 0, optval = 1;
	string tmp;
	int Port = stoi(port);

	log = Log::getInstance(); //Get Singelton

	//Create socket
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		tmp = "Error when opening socket: ";
		tmp += strerror(errno);
		log->output(tmp, 3);
	}

	//Eliminates address already in use error form bind
	if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int)) < 0){
		tmp = "Error when setting socket option: ";
		tmp += strerror(errno);
		log->output(tmp, 3);
	}

	//Socket file descriptor will be an endpoint for all request to port
	memset(&addr, '0', sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(Port);

	// bind to this address and port
	if(bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0){
		tmp = "Error when binding: ";
		tmp += strerror(errno);
		log->output(tmp, 3);
	}

	//listen
	if(listen(sock, 1024) < 0){
		tmp = "Error when listening: ";
		tmp += strerror(errno);
		log->output(tmp, 3);
	}

	retVal = errno;

	return retVal;
}

int Socket::acceptListen(){
	int clientSock = 0;
	sockaddr_in clientAddr;
	unsigned int length = sizeof(clientAddr);
	string tmp;

	//calls accept, returns client sock info
	clientSock = accept(sock, (sockaddr*)&clientAddr, &length);

	if(clientSock < 0){
		tmp = "Error when accepting: ";
		tmp += strerror(errno);
		log->output(tmp, 3);
	}
	else{
		char address[INET_ADDRSTRLEN];
		inet_ntop( AF_INET, &clientAddr, address, INET_ADDRSTRLEN);
		ipAddress = address;
	}

	return clientSock;
}

int Socket::Send(string data){
	int retVal = 0;
	string tmp;

	//send
	if( send(sock, data.c_str(), strlen( data.c_str() ), 0) < 0 ){
		tmp = "Error when sending: ";
		tmp += strerror(errno);
		log->output(tmp, 3);
		retVal = errno;
	}

	//remove empty white space
	int len = data.length();
	data = data.substr(0,len-2);

	return retVal;
}

string Socket::Recv(int maxSize){
	char buffer[maxSize];
	memset(buffer, 0, sizeof(buffer));
	string tmp;

	//call recv
	if( recv(sock, buffer, sizeof(buffer), 0) < 0 ){
		tmp = "Error when receiving: ";
		tmp += strerror(errno);
		log->output(tmp, 3);
	}

	//remove empty white space
	string retVal(buffer);

	int len = retVal.length();
	retVal = retVal.substr(0,len-2);

	return retVal;
}

int Socket::Send(int socket, string data){
	int retVal = 0;
	string tmp;

	//send
	if( send(socket, data.c_str(), strlen( data.c_str() ), 0) < 0 ){
		tmp = "Error when sending: ";
		tmp += strerror(errno);
		log->output(tmp, 3);
	}

	retVal = errno;

	//remove empty white space
	int len = data.length();
	data = data.substr(0,len-2);

	tmp = "SEND ";
	tmp += data;
	tmp += " from ";
	tmp += ipAddress;
	log->output(tmp, 1);

	return retVal;
}

string Socket::Recv(int socket, int maxSize){
	char buffer[maxSize];
	memset(buffer, 0, sizeof(buffer));
	string tmp;

	//call recv
	if( recv(socket, buffer, sizeof(buffer), 0) < 0 ){
		tmp = "Error when receiving: ";
		tmp += strerror(errno);
		log->output(tmp, 3);
	}

	//remove empty white space
	string retVal(buffer);

	int len = retVal.length();
	retVal = retVal.substr(0,len-2);

	return retVal;
}

void Socket::closeSocket(){
	close(sock);
}

string Socket::getIpAddress(int socket){
	string tmp;
	struct sockaddr_in ip;
	unsigned int ipSize = sizeof(ip);

	getpeername(socket, (struct sockaddr*)&ip, &ipSize);

	return inet_ntoa(ip.sin_addr);
}

int Socket::getPort(int socket){
	string tmp;
	struct sockaddr_in ip;
	unsigned int ipSize = sizeof(ip);

	getsockname(socket, (struct sockaddr *)&ip, &ipSize);

	return ntohs(ip.sin_port);
}
