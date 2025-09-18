/*
	Original author of the starter code
    Tanzir Ahmed
    Department of Computer Science & Engineering
    Texas A&M University
    Date: 2/8/20
	
	Please include your Name, UIN, and the date below
	Name:Platini Nguyen
	UIN:234003449
	Date:9/16/25
*/
#include "common.h"
#include "FIFORequestChannel.h"
#include <unistd.h>
using namespace std;


int main (int argc, char *argv[]) {
	int opt;
	int p = 1;
	double t = 0.0;
	int e = 1;
	//Create pipe and check for error
	int fd[2];
	if (pipe(fd) == -1) {
		cerr << "Pipe failed\n";
		return 1;
	}
	string filename = "";
	while ((opt = getopt(argc, argv, "p:t:e:f:")) != -1) {
		switch (opt) {
			case 'p':
				p = atoi (optarg); //atoi is ascii to int
				break;
			case 't':
				t = atof (optarg); //atof is ascii to float
				break;
			case 'e':
				e = atoi (optarg);
				break;
			case 'f':
				filename = optarg;
				break;
		}
	}
	//Create child, check for error, and run ./server
	pid_t id = fork();
	if (id ==-1) {
	cerr << "fork failed\n";
	}
	if (id ==0) {
	    	char* arg[] = {(char*)"./server", NULL};
		execvp(arg[0], arg);
		close(fd[0]);
		close(fd[1]);
	}

    FIFORequestChannel chan("control", FIFORequestChannel::CLIENT_SIDE);
	
	// example data point request
    char buf[MAX_MESSAGE]; // 256
    datamsg x(p,t,e);
	
	memcpy(buf, &x, sizeof(datamsg));
	chan.cwrite(buf, sizeof(datamsg)); // question
	double reply;
	chan.cread(&reply, sizeof(double)); //answer
	cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << reply << endl;
	
    // sending a non-sense message, you need to change this
	filemsg fm(0, 0);
	string fname = "teslkansdlkjflasjdf.dat";
	
	int len = sizeof(filemsg) + (fname.size() + 1);
	char* buf2 = new char[len];
	memcpy(buf2, &fm, sizeof(filemsg));
	strcpy(buf2 + sizeof(filemsg), fname.c_str());
	chan.cwrite(buf2, len);  // I want the file length;

	delete[] buf2;
	
	// closing the channel    
    MESSAGE_TYPE m = QUIT_MSG;
    chan.cwrite(&m, sizeof(MESSAGE_TYPE));

	
}
