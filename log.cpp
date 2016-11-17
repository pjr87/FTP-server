/*
 * Phillip Ryan
 *
 * log.cpp
 *
 */

#include "log.h"

//Helper functions which tokenize a string and return as a vector
std::vector<string> splitByEqual(string line){
	char token = '=';
	 stringstream ss(line);
	string item;
	vector<string> tokens;
	while (getline(ss, item, token)) {
		tokens.push_back(item);
	}
	return tokens;
}

string tmplog;
static int filter(const struct dirent* dir_ent)
{
    if (!strcmp(dir_ent->d_name, ".") || !strcmp(dir_ent->d_name, "..")) return 0;
    std::string fname = dir_ent->d_name;

    if (fname.find(tmplog) == std::string::npos) return 0;

    return 1;
}

Log* Log::instance = NULL;
bool Log::instanceFlag = false;
Log *Log::getInstance(){
	if( !instanceFlag ){
		instance = new Log();
		instanceFlag = true;
		return instance;
	}
	else
		return instance;
}

//Parse Config File and store variables
int Log::readConfig(string file){
	cout << "reading config " << file << endl;
	int retVal = 0;
	configFile = file;
	std::ifstream infile(configFile);
	string tmpLogFile;

	std::string line;
	while (std::getline(infile, line))
	{
		//Ignore line if it begins with a #, and not empty
	    if(line.find("#") != 0 && !line.empty()){
	    	vector<string> tmp = splitByEqual(line);
	    	string key = tmp.at(0);
	    	string value = tmp.at(1);
	    	value.erase(std::remove(value.begin(), value.end(), ' '), value.end());
	    	if( value.empty() )
	    		return -1;
	    	if(key.compare("logdirectory") == 0){
	    		tmpLogFile = value;
	    	}
	    	else if(key.compare("port") == 0){
	    		port = stoi(value);
	    	}
	    	else if(key.compare("numlogfiles") == 0){
	    		numlogfiles = stoi(value);
	    	}
	    	else if(key.compare("usernamefile") == 0){
	    		usernamefile = value;
	    	}
	    	else if(key.compare("port_mode") == 0){
	    		//Take the command value and make it upper case
				for(int i=0; i<value.length(); i++){
					value[i] = toupper(value[i]);
				}
				cout << "Value " << value << endl;
	    		if(value.compare("NO") == 0)
	    			port_mode = 0;
	    		else
	    			port_mode = 1;
	    	}
	    	else if(key.compare("pasv_mode") == 0){
	    		//Take the command value and make it upper case
				for(int i=0; i<value.length(); i++){
					value[i] = toupper(value[i]);
				}
	    		if(value.compare("NO") == 0)
					pasv_mode = 0;
				else
					pasv_mode = 1;
	    	}
	    }
	}

	retVal = setLog(tmpLogFile);

	if(port_mode == 0 && pasv_mode == 0){
		cout << "Error config file fault, cannot set both port and pasv to NO" << endl;
		retVal = -1;
	}

	return retVal;
}

//Sets the file which will hold the output log
int Log::setLog(string file){
	cout << "Set log " << file << endl;
	int retVal = 0;
	logFile = file;

	//Find number of files that exist currently
	struct dirent **namelist = NULL;
	std::vector<std::string> v;
	std::vector<std::string>::iterator  it;

	tmplog = file;
	//Get all the number of files starting with logFile
	int numEntries = scandir(".", &namelist, filter, alphasort);
	if(errno < 0 && errno != 2){
		cout << "error " << strerror(errno) << endl;
		return errno;
	}
	else if( errno == 2 ){
		//no file exists, create one
		cout << "No file exists, creating new log file" << endl;
	}
	else{
		cout << "Numfiles " << numEntries << endl;

		for (int i=0; i<numEntries; i++) {
			std::string fname = namelist[i]->d_name;

			v.push_back(fname);
		}
		free(namelist);

		//remove extra files
		for(int i=numEntries-1; i >= 0; i--){
			//If file is larger then allowed file
			if( i+1 > numlogfiles ){
				cout << "Removing " << v.at(i) << endl;
				remove(v.at(i).c_str());
			}
			//Else move file suffix up one
			else{
				string oldName = v.at(i);
				string newName = v.at(i);
				int start = newName.find(".");
				int end = newName.length();
				newName.replace(start+1, end, to_string(i+1));
				retVal = rename(oldName.c_str(), newName.c_str());
			}
		}
	}
	//Rename logFil variable
	logFile = logFile + ".0";

	//re-open new file
	outFile.open(logFile, ios_base::app);
	errno = 0;
	return retVal;
}

//takes string and outputs
//option - determines where output is sent
//	1 - only logfile
//	2 - only screen
//	3 - both logfile and screen
void Log::output(string text, int option){
	char date[80];
	time_t t = time(0);
	struct tm tstruct = *localtime(&t);
	strftime(date, sizeof(date), "%Y-%m-%d.%X", &tstruct);

	switch(option){
	case 1:
		outFile << date << " " << text << endl;
		break;
	case 2:
		cout << text << endl;
		break;
	case 3:
		outFile << date << " " << text << endl;
		cout << text << endl;
		break;
	default:
		break;
	}
}

//close logging
int Log::close(){
	int retVal = 0;

	outFile.close();

	return retVal;
}
