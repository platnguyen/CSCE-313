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
	bool time_flag = false;
	bool file_flag = false;
	bool new_channel_flag = false;
	bool new_buff_flag = false;
	char* new_buff;
//	int maxMSG = MAX_MESSAGE;
	string filename = "";
	while ((opt = getopt(argc, argv, "p:t:e:f:m:c:")) != -1) {
		switch (opt) {
			case 'p':
				p = atoi (optarg); //atoi is ascii to int
				break;
			case 't':
				t = atof (optarg); //atof is ascii to float
				time_flag = true;
				break;
			case 'e':
				e = atoi (optarg);
				break;
			case 'f':
				filename = optarg;
				file_flag = true;
				break;
			case 'm':
//				maxMSG = atoi(optarg);
				new_buff_flag = true;
				new_buff = optarg;
				break;
			case 'c':
				new_channel_flag = true;
				break;
		}
	}
	//Create child, check for error, and run ./server
	pid_t id = fork();
	if (id ==-1) {
	cerr << "fork failed\n";
	}
	if (id ==0) {
		if (new_buff_flag) {
			char* arg[] = {(char*)"./server", (char*)"-m", new_buff, NULL};
			execvp(arg[0], arg);
		}else {
		    	char* arg[] = {(char*)"./server", NULL};
			execvp(arg[0], arg);
		}
	}

    FIFORequestChannel chan("control", FIFORequestChannel::CLIENT_SIDE);
	
	if (time_flag == false && new_channel_flag == false && file_flag == false) {
		ofstream thousand_points;
		thousand_points.open("x1.csv");
		for (double t = 0; t < 4; t += 0.004) {
			datamsg d(p, t, e);
			chan.cwrite(&d, sizeof(datamsg));
			double ret;
			chan.cread(&ret, sizeof(double));
			thousand_points << ret << endl;
		}
		thousand_points.close();
	}else if (time_flag == true || (file_flag == false && new_channel_flag == false)) {

		datamsg x(p,t,e);
		chan.cwrite(&x, sizeof(datamsg));
		double reply; 
		chan.cread(&reply, sizeof(double));
		cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << reply << endl;
	}
	
    // sending a non-sense message, you need to change this
	filemsg fm(0, 0);
	string fname = "teslkansdlkjflasjdf.dat";
	//4.3 below
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
