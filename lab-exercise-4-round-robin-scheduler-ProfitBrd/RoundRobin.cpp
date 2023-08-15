#include "RoundRobin.h"

/*
This is a constructor for RoundRobin Scheduler, you should use the extractProcessInfo function first
to load process information to process_info and then sort process by arrival time;
Also initialize time_quantum
*/
RoundRobin::RoundRobin(string file, int time_quantum)
{
	set_time_quantum(time_quantum);
	extractProcessInfo(file);
}

// Schedule tasks based on RoundRobin Rule
// the jobs are put in the order the arrived
// Make sure you print out the information like we put in the document
void RoundRobin::schedule_tasks()
{

	
	int systemTime = 0;
	queue<shared_ptr<Process>> processVector;
	while(processVec.size() != 0){
		bool loopCompleted = false;
		for (int i = 0; i < time_quantum; i++){
			
			//if it's not scheduled yet
			if (processVec.front()->get_arrival_time() > systemTime){
				print(systemTime, -1, processVec.front()->is_Completed());
				++systemTime;
				i=-1;
				continue;
			}
			//if it's 0 length to start out, make it NOP
			if (processVec.front()->get_remaining_time() == 0 && i == 0){
				print(systemTime, processVec.front()->getPid(), true);
				break;
			}

			//regular program
			print(systemTime, processVec.front()->getPid(), processVec.front()->is_Completed());
			if (processVec.front()->is_Completed()){
				break;
			}
			processVec.front()->Run(1); //update is_completed and time_left
			++systemTime;


			if (i == time_quantum-1){
				loopCompleted = true;
			}
		}
		if (processVec.empty()){
			break;
		}
		if (processVec.front()->is_Completed() && loopCompleted){
			print(systemTime, processVec.front()->getPid(), processVec.front()->is_Completed());
		}
		if (!processVec.front()->is_Completed() && !(processVec.front()->get_remaining_time() == 0)){ //if it's completed and it wasn't NOP (because is_completed wasn't run)
			processVec.push(processVec.front());
		}
		processVec.pop();
		
	}
}

/***************************
ALL FUNCTIONS UNDER THIS LINE ARE COMPLETED FOR YOU
You can modify them if you'd like, though :)
***************************/

// Default constructor
RoundRobin::RoundRobin()
{
	time_quantum = 0;
}

// Time quantum setter
void RoundRobin::set_time_quantum(int quantum)
{
	this->time_quantum = quantum;
}

// Time quantum getter
int RoundRobin::get_time_quantum()
{
	return time_quantum;
}

// Print function for outputting system time as part of the schedule tasks function
void RoundRobin::print(int system_time, int pid, bool isComplete)
{
	string s_pid = pid == -1 ? "NOP" : to_string(pid);
	cout << "System Time [" << system_time << "].........Process[PID=" << s_pid << "] ";
	if (isComplete)
		cout << "finished its job!" << endl;
	else
		cout << "is Running" << endl;
}

// Read a process file to extract process information
// All content goes to proces_info vector
void RoundRobin::extractProcessInfo(string file)
{
	// open file
	ifstream processFile(file);
	if (!processFile.is_open())
	{
		perror("could not open file");
		exit(1);
	}

	// read contents and populate process_info vector
	string curr_line, temp_num;
	int curr_pid, curr_arrival_time, curr_burst_time;
	while (getline(processFile, curr_line))
	{
		// use string stream to seperate by comma
		stringstream ss(curr_line);
		getline(ss, temp_num, ',');
		curr_pid = stoi(temp_num);
		getline(ss, temp_num, ',');
		curr_arrival_time = stoi(temp_num);
		getline(ss, temp_num, ',');
		curr_burst_time = stoi(temp_num);
		shared_ptr<Process> p(new Process(curr_pid, curr_arrival_time, curr_burst_time));

		processVec.push(p);
	}

	// close file
	processFile.close();
}