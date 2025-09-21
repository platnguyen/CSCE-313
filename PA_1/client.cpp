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
	
	if (time_flag == false && new_channel_flag == false && file_flag == false) {
		//To get 1000 points of data for a patient
		ofstream thousand_points;
		thousand_points.open("received/x1.csv");
		for (double t = 0; t < 4; t += 0.004) {
			thousand_points << t << ",";
			datamsg d(p, t, 1);
			chan.cwrite(&d, sizeof(datamsg));
			double ret;
			chan.cread(&ret, sizeof(double));
			thousand_points << ret << ",";
			datamsg d2(p, t, 2);
			chan.cwrite(&d2, sizeof(datamsg));
			double ret2;
			chan.cread(&ret2, sizeof(double));
			thousand_points << ret2 << endl;

		}
		thousand_points.close();
	}else if (time_flag == true || (file_flag == false && new_channel_flag == false)) {
		//For only one data point
		datamsg x(p,t,e);
		chan.cwrite(&x, sizeof(datamsg));
		double reply; 
		chan.cread(&reply, sizeof(double));
		cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << reply << endl;
	}else if (time_flag == false && file_flag == true) {
		filemsg f(0, 0);
		int file_len = sizeof(filemsg) + strlen(filename) + 1;
		char* buf = new char[file_len];
		memcpy(buf, &f, sizeof(filemsg));
		strcpy(buf + sizeof(filemsg), filename);
		chan.cwrite(buf, file_len);
		__int64_t fs;
		chan.cread(&fs, sizeof(__int64_t));
		int num_msgs = ceil(double(fs)/maxMSG);
		filemsg* fm = (filemsg*) buf;
		if (num_msgs == 1){
			fm->offset = 0;
			fm->length = fs;
		} else {
			fm->length = maxMSG;
			fm->offset = 0;
		}
		__int64_t lastCount = fs - maxMSG* (num_msgs-1);
		chan.cwrite(buf, file_len);
		char* ret_buffer = new char[maxMSG];
		chan.cread(ret_buffer,maxMSG);
		string outputFilePath = string("received/") + string(filename);
		FILE* fp = fopen(outputFilePath.c_str(), "wb");
		fwrite(ret_buffer, 1, fm->length, fp);


		struct timeval start, end;
		gettimeofday(&start, NULL);
		for (int i = 1; i < num_msgs; i++){
			if (i == num_msgs-1){
				fm->length = lastCount;
				ret_buffer = new char[lastCount];
				fm->offset += maxMSG;
				chan.cwrite(buf, file_len);
				chan.cread(ret_buffer,maxMSG);
				fwrite(ret_buffer, 1, fm->length, fp);
			}else{
				fm->offset += maxMSG;
				chan.cwrite(buf, file_len);
				chan.cread(ret_buffer, maxMSG);
				fwrite(ret_buffer, 1, fm->length, fp);
			}
		}
		gettimeofday(&end, NULL);
		double time_taken;
		time_taken = (end.tv_sec - start.tv_sec) * 1e6;
		time_taken = (time_taken + (end.tv_usec - start.tv_usec)) + 1e-6;
		delete [] ret_buffer;
		delete [] buf;
	}
    
	// closing the channel    
    MESSAGE_TYPE m = QUIT_MSG;
    chan.cwrite(&m, sizeof(MESSAGE_TYPE));

	
}
