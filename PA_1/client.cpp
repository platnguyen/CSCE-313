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
#include <sys/time.h>
#include <iomanip>
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
	int maxMSG = MAX_MESSAGE;
	char* filename;
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
				maxMSG = atoi(optarg);
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
	FIFORequestChannel* newChan = &chan;
	if (new_channel_flag) {
		MESSAGE_TYPE new_chan = NEWCHANNEL_MSG;
		chan.cwrite(&new_chan, sizeof(MESSAGE_TYPE));
		char newChannelName[100];
		chan.cread(newChannelName, sizeof(newChannelName));
		newChan = new FIFORequestChannel(newChannelName, FIFORequestChannel::CLIENT_SIDE);
	}


	if (time_flag == false && new_channel_flag == false && file_flag == false) {
		//To get 1000 points of data for a patient
		ofstream thousand_points;
		thousand_points.open("received/x1.csv");
		for (double t = 0; t < 4; t += 0.004) {
			thousand_points << t << ",";
			datamsg d(p, t, 1);
			newChan->cwrite(&d, sizeof(datamsg));
			double ret;
			newChan->cread(&ret, sizeof(double));
			thousand_points << ret << ",";
			datamsg d2(p, t, 2);
			newChan->cwrite(&d2, sizeof(datamsg));
			double ret2;
			newChan->cread(&ret2, sizeof(double));
			thousand_points << ret2 << endl;

		}
		thousand_points.close();
	}else if (time_flag == true || (file_flag == false && new_channel_flag == false)) {
		//For only one data point
		datamsg x(p,t,e);
		newChan->cwrite(&x, sizeof(datamsg));
		double reply; 
		newChan->cread(&reply, sizeof(double));
		cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << reply << endl;
	}else if (time_flag == false && file_flag == true) {
		//Create the request messege to get file size
		filemsg f(0, 0);
		int file_len = sizeof(filemsg) + strlen(filename) + 1;
		char* buf = new char[file_len];
		memcpy(buf, &f, sizeof(filemsg));
		strcpy(buf + sizeof(filemsg), filename);
		//Send the size request and read the response
		newChan->cwrite(buf, file_len);
		__int64_t fs;
		newChan->cread(&fs, sizeof(__int64_t));

		string outputFilePath = string("received/") + string(filename);
		FILE* fp = fopen(outputFilePath.c_str(), "wb");
		if (!fp) {
			cerr << "Failed to open output file. \n";
		}
		char* ret_buffer = new char[maxMSG];
		__int64_t received_so_far = 0;

		struct timeval start, end;
		gettimeofday(&start, NULL);
		filemsg* fm = (filemsg*) buf;
		while (received_so_far < fs){
			fm->offset = received_so_far;
			__int64_t remainingBytes = fs - received_so_far;
			if (remainingBytes < maxMSG) {
				fm->length = remainingBytes;
			}else {
				fm->length = maxMSG;
			}
		
		newChan->cwrite(buf, file_len);
		newChan->cread(ret_buffer, fm->length);
		fwrite(ret_buffer, 1, fm->length, fp);
		received_so_far += fm->length;
		}
		gettimeofday(&end, NULL);
		fclose(fp);

		double timeTaken;
		timeTaken = (end.tv_sec - start.tv_sec) * 1e6;
		timeTaken = (timeTaken + (end.tv_usec - start.tv_usec)) * 1e-6;
		cout << "File transfer completed." << endl;
		cout << "Time taken by program is : " << fixed << timeTaken << setprecision(6);
		cout << " sec" << endl;
		delete [] ret_buffer;
		delete [] buf;
		
	}
	
	// closing the channel    
	
	if (new_channel_flag) {
		MESSAGE_TYPE mNew = QUIT_MSG;
		newChan->cwrite(&mNew, sizeof(MESSAGE_TYPE));
		delete newChan;
	}
	MESSAGE_TYPE m = QUIT_MSG;
	chan.cwrite(&m, sizeof(MESSAGE_TYPE));

}

	
