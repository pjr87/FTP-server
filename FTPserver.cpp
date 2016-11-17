/*
 * Phillip Ryan
 *
 * FTPserver.cpp
 *
 */

#include "FTPserver.h"

std::string getOsName()
{
    #ifdef _WIN32
    return "Windows 32-bit";
    #elif _WIN64
    return "Windows 64-bit";
    #elif __unix || __unix__
    return "Unix";
    #elif __APPLE__ || __MACH__
    return "Mac OSX";
    #elif __linux__
    return "Linux";
    #elif __FreeBSD__
    return "FreeBSD";
    #else
    return "Other";
    #endif
}

//Helper functions which tokenize a string and return as a vector
std::vector<string> FTPserver::tokenize(string line){
	istringstream buf(line);
	istream_iterator<string> beg(buf),end;

	return vector<string>(beg,end);
}

//Helper functions which tokenize a string and return as a vector
std::vector<string> tokener(string line, char token){
	replace(line.begin(), line.end(), token, ' ');
	istringstream buf(line);
	istream_iterator<string> beg(buf),end;

	return vector<string>(beg,end);
}

int FTPserver::USER(string username){
	int retVal = 0;
	string tmp;

	log = Log::getInstance(); //Get Singelton

	ifstream usersFile;
	std::ios_base::iostate exceptionMask = usersFile.exceptions() | std::ios::failbit;
	usersFile.exceptions(exceptionMask);
	if (!usersFile.is_open()){
		try{
			usersFile.open(log->usernamefile);
		}
		catch(const ios_base::failure& e){
			string error = "Failure to open file ";
			error += strerror(errno);
			log->output(error, 3);
		}
	}

	string line, user, pass;

	while( getline(usersFile, line) ){
		cout << line << endl;
		//tokenize the string by spaces
		vector<string> input = tokenize(line);

		//Store rest of command if neccesary
		try{
			user = input.at(0);
			pass = input.at(1);
		}
		catch(std::out_of_range& err){}

		if( user.compare(username) == 0){
			state = 2;
		}
	}

	if( state == 2 ){
		string command = "331 Please specify the password.";
		command += "\r\n";

		retVal = control.Send(command);
		if( retVal != 0 ){
			tmp = "Error when sending USER reply";
			tmp += strerror(errno);
			log->output(tmp, 3);
			state = retVal;
		}
		else{
			tmp = "Sent";
			tmp += command;
			log->output(tmp, 3);
		}
	}
	else{
		string command = "500 Error while authenticating.";
		command += "\r\n";

		tmp = "FAILED login, invalid username";
		log->output(tmp, 1);

		retVal = control.Send(command);
		if( retVal != 0 ){
			tmp = "Error when sending USER reply";
			tmp += strerror(errno);
			log->output(tmp, 3);
			state = retVal;
		}
		else{
			tmp = "Sent ";
			tmp += command;
			log->output(tmp, 3);
		}
	}

	usersFile.close();

	return retVal;
}

int FTPserver::PASS(string password){
	int retVal = 0;
	string tmp;

	log = Log::getInstance(); //Get Singelton

	ifstream usersFile;
		std::ios_base::iostate exceptionMask = usersFile.exceptions() | std::ios::failbit;
		usersFile.exceptions(exceptionMask);
		if (!usersFile.is_open()){
			try{
				usersFile.open(log->usernamefile);
			}
			catch(const ios_base::failure& e){
				string error = "Failure to open file ";
				error += strerror(errno);
				log->output(error, 3);
			}
		}

	string line, user, pass;

	while( getline(usersFile, line) ){
		//tokenize the string by spaces
		vector<string> input = tokenize(line);

		//Store rest of command if neccesary
		try{
			user = input.at(0);
			pass = input.at(1);
		}
		catch(std::out_of_range& err){}

		if( pass.compare(password) == 0){
			state = 3;
		}
	}

	if( state == 3 ){
		string command = "230 Login successful.";
		command += "\r\n";

		retVal = control.Send(command);
		if( retVal != 0 ){
			tmp = "Error when sending PASS reply";
			tmp += strerror(errno);
			log->output(tmp, 3);
			state = retVal;
		}
		else{
			tmp = "Sent";
			tmp += command;
			log->output(tmp, 3);
		}
	}
	else{
		string command = "500 Error while authenticating.";
		command += "\r\n";

		tmp = "FAILED login, invalid password";
		log->output(tmp, 1);

		retVal = control.Send(command);
		if( retVal != 0 ){
			tmp = "Error when sending USER reply";
			tmp += strerror(errno);
			log->output(tmp, 3);
			state = retVal;
		}
		else{
			tmp = "Sent";
			tmp += command;
			log->output(tmp, 3);
		}
	}

	usersFile.close();

	return retVal;
}

int FTPserver::SYST(){
	int retVal = 0;
	string tmp;

	log = Log::getInstance(); //Get Singelton

	string command = "215 " + getOsName();
	command += "\r\n";

	retVal = control.Send(command);
	if( retVal != 0 ){
		tmp = "Error when sending SYST reply";
		tmp += strerror(errno);
		log->output(tmp, 3);
		retVal = errno;
	}
	else{
		tmp = "Sent";
		tmp += command;
		log->output(tmp, 3);
	}

	return retVal;
}

int FTPserver::FEAT(){
	int retVal = 0;
	string tmp;

	log = Log::getInstance(); //Get Singelton

	string command = "500";
	command += "\r\n";

	retVal = control.Send(command);
	if( retVal != 0 ){
		tmp = "Error when sending FEAT reply";
		tmp += strerror(errno);
		log->output(tmp, 3);
		retVal = errno;
	}
	else{
		tmp = "Sent";
		tmp += command;
		log->output(tmp, 3);
	}

	return retVal;
}

int FTPserver::CWD(string pathname){
	int retVal = 0;
	string tmp, command;

	log = Log::getInstance(); //Get Singelton

	retVal = chdir(pathname.c_str());
	if( retVal == 0 ){
		//build command
		command = "250 Directory changed successfully";
		command += "\r\n";
	}
	else{
		//build command
		command = "500 Directory change failed";
		command += "\r\n";
	}

	//send command
	retVal = control.Send(command);
	if( retVal != 0 ){
		tmp = "Error when sending CWD command: ";
		tmp += strerror(errno);
		log->output(tmp, 3);
	}
	else{
		tmp = "Sent";
		tmp += command;
		log->output(tmp, 3);
	}

	return retVal;
}

int FTPserver::CDUP(){
	int retVal = 0;
	string tmp, command;

	log = Log::getInstance(); //Get Singelton

	retVal = chdir("..");
	if( retVal == 0 ){
		//build command
		command = "200 Directory changed successfully";
		command += "\r\n";
	}
	else{
		//build command
		command = "500 Directory change failed";
		command += "\r\n";
	}

	//send command
	retVal = control.Send(command);
	if( retVal != 0 ){
		tmp = "Error when sending CDUP command: ";
		tmp += strerror(errno);
		log->output(tmp, 3);
	}
	else{
		tmp = "Sent";
		tmp += command;
		log->output(tmp, 3);
	}

	return retVal;
}

int FTPserver::QUIT(){
	int retVal = 0;
	string tmp;

	log = Log::getInstance(); //Get Singelton

	return retVal;
}

int FTPserver::PWD(){
	int retVal = 0;
	string tmp;

	log = Log::getInstance(); //Get Singelton
	string command;
	char buffer[256];
	char *answer = getcwd(buffer, sizeof(buffer));
	string s_cwd;
	if (answer)
	{
	    s_cwd = answer;
	    command = "257 ";
		command += s_cwd;
		command += "\r\n";
	}
	else{
		 command = "500 Error while getting current directory";
		 command += "\r\n";
	}

	retVal = control.Send(command);
	if( retVal != 0 ){
		tmp = "Error when sending PWD reply";
		tmp += strerror(errno);
		log->output(tmp, 3);
		retVal = errno;
	}
	else{
		tmp = "Sent ";
		tmp += command;
		log->output(tmp, 3);
	}

	return retVal;
}

int FTPserver::LIST(string pathname){
	int retVal = 0;
	string tmp;

	log = Log::getInstance(); //Get Singelton

	string files = "";
	DIR *dpdf;
	struct dirent *epdf;

	if( pathname.empty( )){
		dpdf = opendir("./");
		if (dpdf != NULL){
		   while (epdf = readdir(dpdf)){
		      files += epdf->d_name;
		      files += " ";
		   }
		}
	}
	else{
		dpdf = opendir(pathname.c_str());
		if (dpdf != NULL){
		   while (epdf = readdir(dpdf)){
			  files += epdf->d_name;
			  files += " ";
		   }
		}
	}

	string command = "125";
	command += files;
	command += "\r\n";

	retVal = control.Send(command);
	if( retVal != 0 ){
		tmp = "Error when sending FEAT reply";
		tmp += strerror(errno);
		log->output(tmp, 3);
		retVal = errno;
	}
	else{
		tmp = "Sent";
		tmp += command;
		log->output(tmp, 3);
	}

	return retVal;
}

int FTPserver::HELP(string help){
	int retVal = 0;
	string tmp;

	log = Log::getInstance(); //Get Singelton

	string command = "211 Please refer to README for help";
	command += "\r\n";

	retVal = control.Send(command);
	if( retVal != 0 ){
		tmp = "Error when sending HELP reply";
		tmp += strerror(errno);
		log->output(tmp, 3);
		retVal = errno;
	}
	else{
		tmp = "Sent";
		tmp += command;
		log->output(tmp, 3);
	}

	return retVal;
}


int FTPserver::PASV(){
	int retVal = 0;
	string tmp;

	log = Log::getInstance(); //Get Singelton
	string command;
	//open listen data socket
	if(log->pasv_mode == 1){
		retVal = dataSock.openListen("0");
		if( retVal != 0 ){
			tmp = "Error when opening port: ";
			tmp += strerror(retVal);
			log->output(tmp, 3);
			return retVal;
			command = "500 error opening socket ";
		}
		else{
			command = "227 Entering Passive Mode ";
			string host = control.getIpAddress(control.sock);
			replace( host.begin(), host.end(), '.', ',');
			command += "(";
			command += host;
			command += ",";
			int port = dataSock.getPort(dataSock.sock);
			int p1 = port / 256;
			int p2 = port % 256;
			command += to_string(p1);
			command += ",";
			command += to_string(p2);
			command += ")\r\n";
		}

		retVal = control.Send(command);
		if( retVal != 0 ){
			tmp = "Error when sending PASV reply";
			tmp += strerror(errno);
			log->output(tmp, 3);
			retVal = errno;
		}
		else{
			tmp = "Sent ";
			tmp += command;
			log->output(tmp, 3);
		}
	}
	else{
		command = "502 command not allowed, use PORT ";
		retVal = control.Send(command);
		if( retVal != 0 ){
			tmp = "Error when sending PASV reply";
			tmp += strerror(errno);
			log->output(tmp, 3);
			retVal = errno;
		}
		else{
			tmp = "Sent ";
			tmp += command;
			log->output(tmp, 3);
		}
	}

	clientData = dataSock.acceptListen();

	return retVal;
}

//Used for retreiving files
int FTPserver::EPSV(string ipType){
	int retVal = 0;
	string tmp;

	log = Log::getInstance(); //Get Singelton

	//open listen data socket
	retVal = dataSock.openListen("0");
	if( retVal != 0 ){
		tmp = "Error when opening port: ";
		tmp += strerror(retVal);
		log->output(tmp, 3);
		return retVal;
	}

	string command = "229 Entering Extended Passive Mode ";
	string host = control.getIpAddress(control.sock);
	replace( host.begin(), host.end(), '.', ',');
	command += "(";
	int port = dataSock.getPort(dataSock.sock);
	command += "|";
	command += "|";
	command += "|";
	command += to_string(port);
	command += "|";
	command += ")\r\n";

	retVal = control.Send(command);
	if( retVal != 0 ){
		tmp = "Error when sending EPSV reply";
		tmp += strerror(errno);
		log->output(tmp, 3);
		retVal = errno;
	}
	else{
		tmp = "Sent ";
		tmp += command;
		log->output(tmp, 3);
	}

	clientData = dataSock.acceptListen();

	return retVal;
}

int FTPserver::EPRT(string arg){
	int retVal = 0;
	string tmp;

	log = Log::getInstance(); //Get Singelton

	//Read response from server and parse the response for ip and port
	int i = arg.find("(");
	int j = arg.find(")");
	string dataInfo = arg.substr(i+1, ((j-1)-i)); //returns every inside ()
	vector<string> hostPort = tokener(dataInfo, '|');
	string ipType = hostPort.at(0);
	string host = hostPort.at(1);
	string port = hostPort.at(2);

	//Attempt to connect to the FTP server
	retVal = dataSock.openConnect(host, port);
	if( retVal != 0 ){
		tmp = "Error when connecting to FTP data port: ";
		tmp += strerror(retVal);
		log->output(tmp, 3);
		return retVal;
	}
	else{
		string command = "200 EPRT command successful. Consider using EPSV.";
		command += "\r\n";

		retVal = control.Send(command);
		if( retVal != 0 ){
			tmp = "Error when sending EPRT reply";
			tmp += strerror(errno);
			log->output(tmp, 3);
			retVal = errno;
		}
		else{
			tmp = "Sent";
			tmp += command;
			log->output(tmp, 3);
		}
	}

	return retVal;
}

int FTPserver::PORT(string arg){
	int retVal = 0;
	string tmp;
	string command;

	log = Log::getInstance(); //Get Singelton

	//Check if positive reply
	//Read response from server and parse the response for ip and port
	if(log->port_mode == 1){
		int i = arg.find("(");
		int j = arg.find(")");
		string dataInfo = arg.substr(i+1, j-1); //returns every inside ()
		vector<string> hostPort = tokener(dataInfo, ',');
		string ip;
		ip = hostPort.at(0);
		ip += ".";
		ip += hostPort.at(1);
		ip += ".";
		ip += hostPort.at(2);
		ip += ".";
		ip += hostPort.at(3);
		string p1 = hostPort.at(4);
		string p2 = hostPort.at(5);
		int port = (stoi(p1)*256) + stoi(p2);

		//Attempt to connect to the FTP client
		retVal = dataSock.openConnect(ip, to_string(port));
		if( retVal != 0 ){
			tmp = "Error when connecting to FTP data port: ";
			tmp += strerror(retVal);
			log->output(tmp, 3);
			return retVal;
		}
		else{
			command = "200 PORT command successful. Consider using PASV.";
			command += "\r\n";

			retVal = control.Send(command);
			if( retVal != 0 ){
				tmp = "Error when sending PORT reply";
				tmp += strerror(errno);
				log->output(tmp, 3);
				retVal = errno;
			}
			else{
				tmp = "Sent ";
				tmp += command;
				log->output(tmp, 3);
			}
		}
	}
	else{
		command = "502 command not allowed, use PASV";
		retVal = control.Send(command);
		if( retVal != 0 ){
			tmp = "Error when sending PORT reply";
			tmp += strerror(errno);
			log->output(tmp, 3);
			retVal = errno;
		}
		else{
			tmp = "Sent ";
			tmp += command;
			log->output(tmp, 3);
		}
	}

	return retVal;
}

int FTPserver::RETR(string pathname){
	int retVal = 0;
	string tmp, command;

	log = Log::getInstance(); //Get Singelton

	ifstream file(pathname, ios::binary | ios::ate);

	if(file){
		command = "150 Opening BINARY mode data connection for ";
		command += pathname;
		command += " (";
		struct stat filestat;
		stat(pathname.c_str(), &filestat);
		command += to_string(filestat.st_size);
		command += " bytes).";
		command += "\r\n";

		retVal = control.Send(command);
		if( retVal != 0 ){
			tmp = "Error when sending PORT reply";
			tmp += strerror(errno);
			log->output(tmp, 3);
			retVal = errno;
		}
		else{
			tmp = "Sent ";
			tmp += command;
			log->output(tmp, 3);

			//Send file datasocket
			std::ifstream file(pathname, std::ios_base::in|std::ios_base::ate);
			long file_length = file.tellg();
			file.seekg(0, std::ios_base::beg);
			file.clear();

			char *buf = new char[file_length];
			file.read(buf, file_length);

			//send
			if(clientData != 0){
				send(clientData, buf, strlen(buf), 0);
				retVal = errno;
			}
			else
				retVal = dataSock.Send(buf);

			if( retVal != 0 ){
				tmp = "Error when sending file";
				tmp += strerror(errno);
				log->output(tmp, 3);
				retVal = errno;

				//send error on control sock
				//Send full message send
				string command = "500 Transfer failed.";
				command += "\r\n";

				retVal = control.Send(command);
				if( retVal != 0 ){
					tmp = "Error when sending RETR reply";
					tmp += strerror(errno);
					log->output(tmp, 3);
					retVal = errno;
				}
				else{
					tmp = "Sent ";
					tmp += command;
					log->output(tmp, 3);
				}
			}
			else{
				//Send full message send
				string command = "226 Transfer complete.";
				command += "\r\n";

				retVal = control.Send(command);
				if( retVal != 0 ){
					tmp = "Error when sending RETR reply";
					tmp += strerror(errno);
					log->output(tmp, 3);
					retVal = errno;
				}
				else{
					tmp = "Sent";
					tmp += command;
					log->output(tmp, 3);
				}
			}
		}
	}
	else{
		command = "500 File does not exist.";
		command += "\r\n";

		retVal = control.Send(command);
		if( retVal != 0 ){
			tmp = "Error when sending PORT reply";
			tmp += strerror(errno);
			log->output(tmp, 3);
			retVal = errno;
		}
		else{
			tmp = "Sent ";
			tmp += command;
			log->output(tmp, 3);
		}
	}

	return retVal;
}
