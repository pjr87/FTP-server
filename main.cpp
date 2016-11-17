/*
 * Phillip Ryan
 * CS 472
 * Homework 3 - FTP Server
 *
 * main.cpp
 *
 */

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <iterator>
#include <algorithm>
#include <numeric>

#include "FTPserver.h"
#include "log.h"

using namespace std;

void *clientFTP(void *arg){
	int retVal = 0;
	int socket = *((int*)arg);
	string tmp;
	FTPserver connected;
	Log* log = Log::getInstance(); //Get Singelton

	//Send welcome
	//build command
	string command = "220 Welcome to Phillip Ryan's FTP server";
	command += "\r\n";

	connected.control.sock = socket;
	retVal = connected.control.Send(command);
	if( retVal != 0 ){
		tmp = "Error when sending welcome message";
		tmp += strerror(errno);
		log->output(tmp, 3);
		connected.state = retVal;
	}
	else{
		tmp = "Sent welcome message";
		connected.state = 1;
		log->output(tmp, 3);
	}

	//See header file for definition of state
	while(connected.state > 0){
		//Receive reply from client, parse for a response
		string data = connected.control.Recv(2046);
		if( data.empty() ){
			tmp = "Error when receiving reply: ";
			tmp += strerror(errno);
			log->output(tmp, 3);
		}
		else{
			tmp = "Received reply: ";
			tmp += data;
			log->output(tmp, 3);
		}

		//tokenize the string by spaces
		vector<string> input = connected.tokenize(data);

		//Store first value of entered command as command name
		string command = input.front();

		//Store rest of command if neccesary
		string a1, a2, a3;
		try{
			a1 = input.at(1);
			a2 = input.at(2);
			a3 = input.at(3);
		}
		catch(std::out_of_range& err){}

		//Take the command value and make it upper case
		for(int i=0; i<command.length(); i++){
			command[i] = toupper(command[i]);
		}

		//Determine what command was entered and call appropriate function
		if(command.compare("QUIT") == 0){
			log->output("Quitting ftp client", 3);
			break;
		}
		else if(command.compare("PASV") == 0){
			log->output("PASV command", 3);
			retVal = connected.PASV();
		}
		else if(command.compare("EPSV") == 0){
			log->output("EPSV command", 3);
			retVal = connected.EPSV(a1);
		}
		else if(command.compare("PORT") == 0){
			log->output("PORT command", 3);
			retVal = connected.PORT(a1);
		}
		else if(command.compare("EPRT") == 0){
			log->output("EPRT command", 3);
			retVal = connected.EPRT(a1);
		}
		else if(command.compare("RETR") == 0){
			log->output("RETR command", 3);
			retVal = connected.RETR(a1);
		}
		else if(command.compare("LIST") == 0){
			log->output("LIST command", 3);
			retVal = connected.LIST(a1);
		}
		else if(command.compare("CDUP") == 0){
			log->output("CDUP command", 3);
			retVal = connected.CDUP();
		}
		else if(command.compare("CWD") == 0){
			log->output("CWD command", 3);
			retVal = connected.CWD(a1);
		}
		else if(command.compare("HELP") == 0){
			log->output("HELP command", 3);
			retVal = connected.HELP(a1);
		}
		else if(command.compare("PWD") == 0){
			log->output("PWD command", 3);
			retVal = connected.PWD();
		}
		else if(command.compare("USER") == 0){
			log->output("USER command", 3);
			retVal = connected.USER(a1);
		}
		else if(command.compare("PASS") == 0){
			log->output("PASS command", 3);
			retVal = connected.PASS(a1);
		}
		else if(command.compare("SYST") == 0){
			log->output("SYST command", 3);
			retVal = connected.SYST();
		}
		else if(command.compare("FEAT") == 0){
			log->output("FEAT command", 3);
			retVal = connected.FEAT();
		}
		else{
			//If command is not recognized then report error and ask
			log->output("Error command not recognized", 3);
		}
	}

	free(arg);
}

int main(int argc, char** argv ){
	int retVal = 0;
	string configFile, line, tmp;

	//Parse commandline options
	if( argc < 1 ){
		//If ipaddress and log file name are not present
		cout << "Error, program requires at least 1 inputs" << endl;
		return 0;
	}
	else{
		//Store log file name
		configFile = argv[1];
	}

	//Setup log file for output
	Log* log = Log::getInstance(); //Get Singelton
	retVal = log->readConfig(configFile); //See Log class for details
	if( retVal < 0 ){
		cout << "Error config file is invalid" << endl;
		return retVal;
	}
	string outPut = "input: " + configFile;
	log->output(outPut, 3);

	//Attempt to connect to the FTP server
	FTPserver *server = new FTPserver();
	retVal = server->control.openListen(to_string(log->port));
	if( retVal != 0 ){
		tmp = "Error when connecting to FTP server: ";
		tmp += strerror(retVal);
		log->output(tmp, 3);
		return retVal;
	}

	//Your server is listening for FTP clients
	tmp = "Your server is listening for FTP clients on ";
	tmp += to_string(log->port);
	log->output(tmp, 3);

	int numConn = 0;

	//Server will accept connections, until QUIT is entered
	int clientSock = 0;
	string clientIP;
	vector<pthread_t> threadID(1024);
	while(clientSock = server->control.acceptListen() ){
		//Call accept and start a thread for each accepted client, will hang here
		if( clientSock < 0 ){
			tmp = "Error when accepting: ";
			tmp += strerror(errno);
			log->output(tmp, 3);
		}
		else{
			clientIP = server->control.getIpAddress(clientSock);
			tmp = "Client accepted from ";
			tmp += clientIP;
			log->output(tmp, 3);

			int *arg = (int*)malloc(sizeof(*arg));
			*arg = clientSock;

			pthread_create(&threadID[numConn], 0, clientFTP, arg);
			numConn++;
		}
	}

	for(int i=0; i<numConn; i++){
		pthread_join(threadID[i], NULL);
	}

	//cleanup
	server->control.closeSocket();

	if( server != nullptr ){
		delete server;
	}

	return retVal;
}
