/*
	Original author of the starter code
    Tanzir Ahmed
    Department of Computer Science & Engineering
    Texas A&M University
    Date: 2/8/20
	
	Please include your Name, UIN, and the date below
	Name:
	UIN:
	Date:
*/
#include "common.h"
#include "FIFORequestChannel.h"

using namespace std;


int main (int argc, char *argv[]) {
	int opt;
	int p = -1;
	double t = -1;
	int e = -1;
	int m = MAX_MESSAGE; //default value for buffer capacity is MAX SIZE
	bool new_chan = false; //turns true if the user wants to create a new channel to recieve the data from
	vector <FIFORequestChannel*> channels;
	string filename = "";


	while ((opt = getopt(argc, argv, "p:t:e:f:m:c")) != -1) {
		switch (opt) {
			case 'p':
				p = atoi (optarg);
				break;
			case 't':
				t = atof (optarg);
				break;
			case 'e':
				e = atoi (optarg);
				break;
			case 'f':
				filename = optarg;
				break;
			case 'm':
				m = atoi(optarg);
				break;
			case 'c':
				new_chan = true;
				break;
		}
	}
	
    

	pid_t pid = fork();
	if(pid < 0){
		//error checking
		std::cerr << "Fork failed" << std::endl;
        return 1;
	}
	else if (pid==0){
		//child process (parent)
        char *args[] = {const_cast<char *>("./server"),const_cast<char*>("-m"), const_cast<char*>(to_string(m/*buffercapacity*/).c_str()), nullptr};/**/
        if (execvp(args[0], args) < 0) //overwrites entirety of below file
        {
            perror("execvp");
            exit(EXIT_FAILURE);
        }
	}
	//parent process (client)
	else{
		FIFORequestChannel cont_chan("control", FIFORequestChannel::CLIENT_SIDE); //[called dynamically, must delete]
		channels.push_back(&cont_chan);

		if(new_chan){
			//send newchannel request to the server
			MESSAGE_TYPE nc = NEWCHANNEL_MSG;
			cont_chan.cwrite(&nc, sizeof(MESSAGE_TYPE));
			//create a variable to hold the name
			char channelName [101]; //make it a cstring
			//cread the response from the server
			cont_chan.cread(channelName, sizeof(string));
			//call FIFORequestChannel constructor with the name from the server
			FIFORequestChannel* chan = new FIFORequestChannel(channelName, FIFORequestChannel::CLIENT_SIDE); //[called dynamically, must delete]
			//[once created this new channel, we should run all of the below code on this new channel]
			channels.push_back(chan);
		}
		
		FIFORequestChannel chan = *(channels.back()); //[we are going to use the last channel in the vector so we don't have to repeat the below code]


		if (p != -1 && t != -1 && e != -1){ //[Single datapoint, only run if p,t,e != -1 (e.g. user entered everything)]
		// example data point request
			char buf[MAX_MESSAGE]; // 256
			datamsg x(p, t, e);
			
			memcpy(buf, &x, sizeof(datamsg)); //[copy request into buffer]
			chan.cwrite(buf, sizeof(datamsg)); // question [write buffer to server through control channel]
			double reply;
			chan.cread(&reply, sizeof(double)); //answer [double to store value that server gives back]
			cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << reply << endl; //[send to terminal]
		}

		else if (p != -1){ //[Single datapoint, only run if p,t,e != -1 (e.g. user entered everything)]
		// example data point request
			char buf[MAX_MESSAGE]; // 256
			ofstream CSVout;
			CSVout.open("received/x1.csv");
			double temp = 0;
			for (int i = 0; i < 1000; i++){
				datamsg x1(p, temp, 1);
				memcpy(buf, &x1, sizeof(datamsg)); //[copy request into buffer]
				chan.cwrite(buf, sizeof(datamsg)); // question [write buffer to server through control channel]
				double ecg1;
				chan.cread(&ecg1, sizeof(double)); //answer [double to store value that server gives back]
				
				datamsg x2(p, temp, 2);
				memcpy(buf, &x2, sizeof(datamsg)); //[copy request into buffer]
				chan.cwrite(buf, sizeof(datamsg)); // question [write buffer to server through control channel]
				double ecg2;
				chan.cread(&ecg2, sizeof(double)); //answer [double to store value that server gives back]

				CSVout << temp << "," << ecg1 << "," << ecg2 << endl;

				temp += 0.004;
				
			}
			CSVout.close();
		}

		else if (filename != ""){
			

			filemsg fm(0, 0);
			string fname = filename; //[need to change this to the value the user has given under -f (filename)]
			
			int len = sizeof(filemsg) + (fname.size() + 1); //[size of message you're sending, the file you want it from, and +1 for the null terminator]
			char* buf2 = new char[len];
			memcpy(buf2, &fm, sizeof(filemsg)); //[copy file message into buffer]
			strcpy(buf2 + sizeof(filemsg), fname.c_str()); //[copy file name into buffer]
			chan.cwrite(buf2, len);  // I want the file length; [send buffer to the server]


			int64_t filesize = 0;
			
			chan.cread(&filesize, sizeof(int64_t)); //[read response from server with cread, response should be size of file]
			int timesToRun = filesize/m;

			int64_t offset = 0;
			int64_t length = filesize;
			char* buf3 = new char[m];

			ofstream fileTransfer;
			fileTransfer.open("received/" + filename);
			for(int i = 0; i < timesToRun; i++){
				filemsg* file_req = (filemsg*)buf2;
				file_req->offset = offset;
				file_req->length = m;
				chan.cwrite(buf2,len);
				chan.cread(buf3,m);

				for (int i = 0; i < m; i++){
					fileTransfer << buf3[i];
				}
				
				length -= m;
				offset += m;
			}
			if (length > 0){
				filemsg* file_req = (filemsg*)buf2;
				file_req->offset = offset;
				file_req->length = length;
				chan.cwrite(buf2, len);
				chan.cread(buf3, length);

				for (int i = 0; i < length; i++){
					fileTransfer << buf3[i];
				}
			}
			delete[] buf2;
			delete[] buf3;
			fileTransfer.close();
		}

		

		channels.erase(channels.begin());

		MESSAGE_TYPE mess = QUIT_MSG;
		for (auto channel : channels){
			channel->cwrite(&mess, sizeof(MESSAGE_TYPE));
			delete channel;
		}
		cont_chan.cwrite(&mess, sizeof(MESSAGE_TYPE)); //cont chan is always the first one
		
		waitpid(pid, NULL, 0);
	}
}
