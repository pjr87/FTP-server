/*
 * Phillip Ryan
 *
 * FTPserver.h
 *
 */

#ifndef FTPSERVER_H
#define FTPSERVER_H

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include "Socket.h"

using namespace std;

class FTPserver
{
public:
	//------------------------------------------------------------
	// FTP commands
	//------------------------------------------------------------
	/*
	 * The argument field is a Telnet string identifying the user.
		The user identification is that which is required by the
		server for access to its file system. This command will
		normally be the first command transmitted by the user after
		the control connections are made (some servers may require
		this). Additional identification information in the form of
		a password and/or an account command may also be required by
		some servers. Servers may allow a new USER command to be
		entered at any point in order to change the access control
		and/or accounting information. This has the effect of
		flushing any user, password, and account information already
		supplied and beginning the login sequence again. All
		transfer parameters are unchanged and any file transfer in
		progress is completed under the old access control
		parameters.
	 */
	int USER(string username);

	/*
	 * The argument field is a Telnet string specifying the user’s
		password. This command must be immediately preceded by the
		user name command, and, for some sites, completes the user’s
		identification for access control. Since password
		information is quite sensitive, it is desirable in general
		to "mask" it or suppress typeout. It appears that the
		server has no foolproof way to achieve this. It is
		therefore the responsibility of the user-FTP process to hide
		the sensitive password information.
	 */
	int PASS(string password);

	/*
	 * This command allows the user to work with a different
		directory or dataset for file storage or retrieval without
		altering his login or accounting information. Transfer
		parameters are similarly unchanged. The argument is a
		pathname specifying a directory or other system dependent
		file group designator.
	 */
	int CWD(string pathname);

	/*
	 * This command is a special case of CWD, and is included to
		simplify the implementation of programs for transferring
		directory trees between operating systems having different
		syntaxes for naming the parent directory. The reply codes
		shall be identical to the reply codes of CWD. See
		Appendix II for further details.
	 */
	int CDUP();

	/*
	 * This command terminates a USER and if file transfer is not
		in progress, the server closes the control connection. If
		file transfer is in progress, the connection will remain
		open for result response and the server will then close it.
		If the user-process is transferring files for several USERs
		but does not wish to close and then reopen connections for
		each, then the REIN command should be used instead of QUIT.
		An unexpected close on the control connection will cause the
		server to take the effective action of an abort (ABOR) and a
		logout (QUIT).
	 */
	int QUIT();

	/*
	 * This command causes the name of the current working
		directory to be returned in the reply.
	 */
	int PWD();

	/*
	 * This command causes a list to be sent from the server to the
		passive DTP. If the pathname specifies a directory or other
		group of files, the server should transfer a list of files
		in the specified directory. If the pathname specifies a
		file then the server should send current information on the
		file. A null argument implies the user’s current working or
		default directory. The data transfer is over the data
		connection in type ASCII or type EBCDIC. (The user must
		ensure that the TYPE is appropriately ASCII or EBCDIC).
		Since the information on a file may vary widely from system
		to system, this information may be hard to use automatically
		in a program, but may be quite useful to a human user.
	 */
	int LIST(string pathname);

	/*
	 * This command shall cause the server to send helpful
		information regarding its implementation status over the
		control connection to the user. The command may take an
		argument (e.g., any command name) and return more specific
		information as a response. The reply is type 211 or 214.
		It is suggested that HELP be allowed before entering a USER
		command. The server may use this reply to specify
		site-dependent parameters, e.g., in response to HELP SITE.
	 */
	int HELP(string help);

	//------------------------------------------------------------
	// Retreiving a file FTP commands
	//------------------------------------------------------------
	/*
	 * This command requests the server-DTP to "listen" on a data
		port (which is not its default data port) and to wait for a
		connection rather than initiate one upon receipt of a
		transfer command. The response to this command includes the
		host and port address this server is listening on.
	 */
	int PASV();

	/*
	 * ipType(optional) - 1(ipv4) or 2(ipv6) or ALL
	 */
	int EPSV(string arg);

	/*
	 * The argument is a HOST-PORT specification for the data port
		to be used in data connection. There are defaults for both
		the user and server data ports, and under normal
		circumstances this command and its reply are not needed. If
		this command is used, the argument is the concatenation of a
		32-bit internet host address and a 16-bit TCP port address.
		This address information is broken into 8-bit fields and the
		value of each field is transmitted as a decimal number (in
		character string representation). The fields are separated
		by commas. A port command would be:
		PORT h1,h2,h3,h4,p1,p2
		where h1 is the high order 8 bits of the internet host
		address.
	 */
	int PORT(string arg);

	/*
	 * ipType - 1(ipv4) or 2(ipv6)
	 * host - ip address
	 * port - port
	 *
	 * EPRT |1|132.235.1.2|6275|
	 * EPRT |2|1080::8:800:200C:417A|5282|
	 */
	int EPRT(string arg);

	/*
	 * This command causes the server-DTP to transfer a copy of the
		file, specified in the pathname, to the server- or user-DTP
		at the other end of the data connection. The status and
		contents of the file at the server site shall be unaffected.
	 */
	int RETR(string pathname);

	/*
	 * Tells the client the OS the server uses
	 */
	int SYST();

	/*
	 * Tells the client the feature list
	 */
	int FEAT();

	Socket control; //Socket which sends control messages to FTP server
	Socket dataSock; //Socket which handles data between FTP and server
	int clientData;

	//This variable defines the state of a connection
	// errno - error, break out
	// 0 - not connected
	// 1 - connected
	// 2 - USER ok
	// 3 - PASS ok, ready for any command
	int state;

	int port; //control port

	vector<string> tokenize(string line);
private:
	Log* log; //singleton allows to output to log file
};

#endif
