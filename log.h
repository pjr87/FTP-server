/*
 * Phillip Ryan
 *
 * log.h
 *
 */

#ifndef LOG_H
#define LOG_H

#include <iostream>
#include <fstream>
#include <string>
#include <dirent.h>
#include <vector>
#include <sstream>
#include <time.h>
#include <cstdio>

using namespace std;

class Log
{
public:
	//Singleton helpers
	static Log *getInstance();

	~Log(){
		instanceFlag = false;
	}

	//Parse Config File and store variables
	int readConfig(string file);

	//Sets the file which will hold the output log
	int setLog(string file);

	//takes string and outputs
	//option - determines where output is sent
	//	1 - only logfile
	//	2 - only screen
	//	3 - both logfile and screen
	void output(string text, int option);

	//close logging
	int close();

	// Config file variables
	int port;
	int numlogfiles = 5;
	string usernamefile;
	int port_mode = 0;
	int pasv_mode = 1;

private:
	//Singleton helpers
	Log(){}; //Private constructor
	static Log *instance;
	static bool instanceFlag;

	string configFile;
	string logFile = "/var/spool/log";
	ofstream outFile;
};

#endif
