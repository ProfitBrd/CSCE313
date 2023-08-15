// your PA3 client code here
#include <fstream>
#include <iostream>
#include <thread>
#include <sys/time.h>
#include <sys/wait.h>

#include "BoundedBuffer.h"
#include "common.h"
#include "Histogram.h"
#include "HistogramCollection.h"
#include "TCPRequestChannel.h"

// ecgno to use for datamsgs
#define EGCNO 1

using namespace std;

struct messageStruct{
    int idNumber;
    double value;
};
void patient_thread_function (int patient_number, int num_requests, BoundedBuffer* BoundedBuffer) {
    // functionality of the patient threads
    double time = 0;
    for (int i = 0; i < num_requests; i++){
        datamsg temp(patient_number, time, EGCNO);
        BoundedBuffer->push((char*)&temp, sizeof(temp));
        time += 0.004;
    }
}

void file_thread_function (BoundedBuffer* request, string name, int fileSize, int bufferSize) {
    // functionality of the file thread

    //file size
    
    //open output file; allocate the memory fseek; close the file
    //while offset < file_size, produce a filemsg(offset, m)+filename and push to request buffer
    //      -incrementing offset; be careful with final message
    int messageLength = 1 + sizeof(filemsg) + name.size();
    int numberTimesBufferFits = fileSize/bufferSize;
    int bufferRemainder = fileSize%bufferSize;
    string directory = "received/" + name;
    FILE* fileWritingTo = fopen(directory.c_str(), "w");
    fseek(fileWritingTo, fileSize, SEEK_SET);
    fclose(fileWritingTo);
    for (int i = 0; i < numberTimesBufferFits; ++i){
        filemsg temp = filemsg(i*bufferSize, bufferSize);
        char buffer[messageLength];
        memcpy(buffer, &temp, sizeof(filemsg));
        strcpy(buffer + sizeof(filemsg), name.c_str()); 
        request->push(buffer, messageLength);
    }

    if (bufferRemainder != 0){
        filemsg lastMessage = filemsg((__int64_t) (fileSize-bufferRemainder), bufferRemainder);
        char buffer[messageLength];
        memcpy(buffer, &lastMessage, sizeof(filemsg));
        strcpy(buffer + sizeof(filemsg), name.c_str()); 
        request->push(buffer, messageLength);
    }
    

}

void worker_thread_function (TCPRequestChannel* serverChan, BoundedBuffer* request, BoundedBuffer* response, int bufferSize) {
    // functionality of the worker threads
    while(true){
        char buffer[bufferSize];
        int size = request->pop(buffer, bufferSize);
        MESSAGE_TYPE m = *((MESSAGE_TYPE*) buffer);
        if (m == DATA_MSG) {
            datamsg* originalMessage = (datamsg*)buffer;
            serverChan->cwrite(buffer, bufferSize);
            double responseBuffer;
            serverChan->cread(&responseBuffer, sizeof(double));
            messageStruct thingToPush;
            thingToPush.idNumber = originalMessage->person;
            thingToPush.value = responseBuffer;
            char* castingThingToPush = (char*)&thingToPush;
            response->push(castingThingToPush, sizeof(messageStruct));
        }
        if(m == QUIT_MSG){
            MESSAGE_TYPE mess = QUIT_MSG;
            serverChan->cwrite(&mess, sizeof(MESSAGE_TYPE));
            break;
        }
        if (m == FILE_MSG){
            //cast message to a filemessage
            filemsg* fileMessage = (filemsg*)buffer;
            //string to hold the filename (which is the data you just passed through)
            string fname = (char*)(fileMessage+1);
            //[size of message you're sending, the file you want it from, and +1 for the null terminator]
            int length = sizeof(filemsg) + (fname.size() + 1);
            //make the filename the directory it's going to put the files
            fname = "received/" + fname;

            //push and pull data from channels
            serverChan->cwrite(fileMessage, length);
            char receiveBuffer[bufferSize];
            serverChan->cread(receiveBuffer, bufferSize);
            
            FILE* openFile = fopen(fname.c_str(), "r+"); //wb

            fseek(openFile, fileMessage->offset, SEEK_SET); //in the openFile we use the offset embedded in the fileMessage and go to the start (SEEK_SET)

            fwrite(receiveBuffer, 1, fileMessage->length, openFile);

            fclose(openFile);
        }
    }
	
}

void histogram_thread_function (BoundedBuffer* response, HistogramCollection* graph) {
    // functionality of the histogram threads
    //forever loop
    while(true){
        char buf[sizeof(messageStruct)];
        response->pop(buf, sizeof(messageStruct));
        messageStruct* message = (messageStruct*) buf;
        if (message->value == -400 || message->idNumber == -400) break;
        graph->update(message->idNumber,message->value);
    }
}


int main (int argc, char* argv[]) {
    int n = 1000;	// default number of requests per "patient"
    int p = 10;		// number of patients [1,15]
    int w = 100;	// default number of worker threads
	int h = 20;		// default number of histogram threads
    int b = 20;		// default capacity of the request buffer (should be changed)
	int m = MAX_MESSAGE;	// default capacity of the message buffer
    string a = "127.0.0.1";
    string r = "8080";
	string f = "";	// name of file to be transferred
    
    // read arguments
    int opt;
	while ((opt = getopt(argc, argv, "n:p:w:h:b:m:f:r:a:")) != -1) {
		switch (opt) {
			case 'n':
				n = atoi(optarg);
                break;
			case 'p':
				p = atoi(optarg);
                break;
			case 'w':
				w = atoi(optarg);
                break;
			case 'h':
				h = atoi(optarg);
				break;
			case 'b':
				b = atoi(optarg);
                break;
			case 'm':
				m = atoi(optarg);
                break;
			case 'f':
				f = optarg;
                break;
            case 'r':
				r = optarg;
                break;
            case 'a':
				a = optarg;
                break;
		}
	}
    
    
	// initialize overhead (including the control channel)
	TCPRequestChannel* chan = new TCPRequestChannel(a, r);
    BoundedBuffer request_buffer(b);
    BoundedBuffer response_buffer(b);
	HistogramCollection hc;

    // making histograms and adding to collection
    for (int i = 0; i < p; i++) {
        Histogram* h = new Histogram(10, -2.0, 2.0);
        hc.add(h);
    }
	
	// record start time
    struct timeval start, end;
    gettimeofday(&start, 0);

    /* create all threads here */
    // vector<thread> patient_threads;
    // for (int i = 0; i < p; i++){
    //     patient_threads.push_back( thread(patient_thread_function, i, n, &request_buffer) );
    // }
    thread patient_threads[p];
    for (int i = 0; i < p; i++){
        patient_threads[i] = thread(patient_thread_function, i + 1, n, &request_buffer);
    }

    thread file_thread;
    if (f != ""){
        int messageLength = 1 + sizeof(filemsg) + f.size();
        char buffer[messageLength];
        filemsg fileMessage(0, 0); //gets total size of file
        memcpy(buffer, &fileMessage, sizeof(filemsg));
        strcpy(buffer + sizeof(filemsg), f.c_str());        
        chan->cwrite(buffer, messageLength);  // I want the file length; [send buffer to the server]
        __int64_t filesize = 0;
        chan->cread(&filesize, sizeof(__int64_t));
        file_thread = thread(file_thread_function, &request_buffer, f, filesize, m);
    }



    vector <TCPRequestChannel*> channels;
    for (int i = 0; i < w; i++){
        //send newchannel request to the server
        //------------------------ MESSAGE_TYPE nc = NEWCHANNEL_MSG;
        //------------------------ chan->cwrite(&nc, sizeof(MESSAGE_TYPE));
        //create a variable to hold the name
        // char channelName [101]; //make it a cstring
        //cread the response from the server
        // chan->cread(channelName, 101);
        //call FIFORequestChannel constructor with the name from the server
        TCPRequestChannel* newChan = new TCPRequestChannel(a, r); //[called dynamically, must delete]
        //[once created this new channel, we should run all of the below code on this new channel]
        channels.push_back(newChan);
    }

    thread worker_threads[w];
    for (int i = 0; i < w; i++){
        worker_threads[i] = thread(worker_thread_function, channels.at(i), &request_buffer, &response_buffer, m);
    }  


    thread histogram_threads[h];
    for (int i = 0; i < h; i++){
        histogram_threads[i] = thread(histogram_thread_function, &response_buffer, &hc);
    }


	/* join all threads here */
    // for (long unsigned int i = 0; i < t_vec2.size(); i++){
    //     t_vec2.at(i).join();
    // }
    for (long unsigned int i = 0; i < p; i++){
        patient_threads[i].join();
    }
    if (f != ""){
        file_thread.join();
    }
    for (int i = 0; i < w; i++){
        datamsg hello(1,0.0,EGCNO);
        hello.mtype = QUIT_MSG;
        char* temp = (char*)&hello;
        request_buffer.push(temp, sizeof(datamsg));
    }
    //channels.erase(channels.begin());    

    for (long unsigned int i = 0; i < w; i++){
        worker_threads[i].join();
    }
    for (long unsigned int i = 0; i < h; i++){
        messageStruct temp;
        temp.idNumber = -400;
        temp.value = -400;
        char* temp2 = (char*)&temp;
        response_buffer.push(temp2, sizeof(messageStruct));
    }
    for (long unsigned int i = 0; i < h; i++){
        histogram_threads[i].join();
    }
    
    

    for (auto channel : channels){
        delete channel;
    }
	// record end time
    gettimeofday(&end, 0);

    // print the results
	if (f == "") {
		hc.print();
	}
    int secs = ((1e6*end.tv_sec - 1e6*start.tv_sec) + (end.tv_usec - start.tv_usec)) / ((int) 1e6);
    int usecs = (int) ((1e6*end.tv_sec - 1e6*start.tv_sec) + (end.tv_usec - start.tv_usec)) % ((int) 1e6);
    cout << "Took " << secs << " seconds and " << usecs << " micro seconds" << endl;

	// quit and close control channel
    // MESSAGE_TYPE q = QUIT_MSG;
    // chan->cwrite ((char *) &q, sizeof (MESSAGE_TYPE));
    // cout << "All Done!" << endl;
    delete chan;
}
