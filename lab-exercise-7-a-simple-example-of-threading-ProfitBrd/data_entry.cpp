#include <vector> // vector, push_back, at
#include <string> // string
#include <iostream> // cin, getline
#include <fstream> // ofstream
#include <unistd.h> // getopt, exit, EXIT_FAILURE
#include <assert.h> // assert
#include <thread> // thread, join
#include <sstream> // stringstream

#include "BoundedBuffer.h" // BoundedBuffer class

#define MAX_MSG_LEN 256

using namespace std;

/************** Helper Function Declarations **************/

void parse_column_names(vector<string>& _colnames, const string& _opt_input);
void write_to_file(const string& _filename, const string& _text, bool _first_input=false);

char* quitmsg = (char*)"Exit\0";
int bufferSize;
/************** Thread Function Definitions **************/

// "primary thread will be a UI data entry point"
void ui_thread_function(BoundedBuffer* bb) {
    // TODO: implement UI Thread Function
    //in an infinite loop, continuously prompt user for input
    string input;
    cout << "Enter Data -->" << endl;
    while (true){
        cin >> input;
        //If input is Exit, break.
        if (input == "Exit"){
            break;
        }
        else{
            //Else, push input into bb
            bb->push((char*)input.c_str(), input.size());
        }
        cout << "Enter Data -->" << endl;
    }
}

// "second thread will be the data processing thread"
// "will open, write to, and close a csv file"
void data_thread_function(BoundedBuffer* bb, string filename, const vector<string>& colnames) {
    // TODO: implement Data Thread Function
    // (NOTE: use "write_to_file" function to write to file)
    //infinite loop, pop from bb
    char msg[MAX_MSG_LEN];

    int numberColumns = (int)colnames.size();
    
    //write the columns, with the ending stopping at newline
    for (int i = 0; i < numberColumns - 1; i++){
        if (i == 0){
            write_to_file(filename, colnames.at(i) + ", ", true);
        }
        else{
            write_to_file(filename, colnames.at(i) + ", ", false);
        }
    }
    write_to_file(filename, colnames.at(numberColumns-1) + "\n", 0);

    int currentColumn = 0;


    while (true){
        //size of popped message
        int sizePopped = bb->pop(msg, MAX_MSG_LEN);
        
        //add null at end of buf
        msg[sizePopped] = '\0';
        string temp = msg;

        //see if buf == "Exit"
        if (temp == "Exit"){
            break;
        }

        //add data to csv file
        //add comma or newline based on curr_col
        if (currentColumn == numberColumns-1){
            write_to_file(filename, temp + "\n");
        }
        else{
            write_to_file(filename, temp + ", ");
        }

        //update current column, making sure it doesn't go over max amount of columns
        ++currentColumn;
        currentColumn = currentColumn % numberColumns;

    }
    // If it's the exit packet, break(compare to exit)
    //Else, call write_to_file
}

/************** Main Function **************/

int main(int argc, char* argv[]) {

    // variables to be used throughout the program
    vector<string> colnames; // column names
    string fname; // filename
    BoundedBuffer* bb = new BoundedBuffer(3); // BoundedBuffer with cap of 3

    // read flags from input
    int opt;
    while ((opt = getopt(argc, argv, "c:f:")) != -1) {
        switch (opt) {
            case 'c': // parse col names into vector "colnames"
                parse_column_names(colnames, optarg);
                break;
            case 'f':
                fname = optarg;
                break;
            default: // invalid input, based on https://linux.die.net/man/3/getopt
                fprintf(stderr, "Usage: %s [-c colnames] [-f filename]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // TODO: instantiate ui and data threads
    vector<thread> t_vec;
    t_vec.push_back(thread(ui_thread_function, bb));
    
    // Vector doesn't work because of rvalues (I think its the fname not being passed by reference)
    // vector<thread> t_vec2;
    // t_vec2.push_back(thread(data_thread_function, bb, std::move(fname), &colnames));
    thread data_thread(data_thread_function, bb, fname, colnames);

    // TODO: join ui_thread
    t_vec.at(0).join();
    
    // TODO: "Once the user has entered 'Exit', the main thread will
    // "send a signal through the message queue to stop the data thread"
    bb->push(quitmsg, 5);

    // TODO: join data thread
    data_thread.join();

    // CLEANUP: delete members on heap
    delete bb;
}

/************** Helper Function Definitions **************/

// function to parse column names into vector
// input: _colnames (vector of column name strings), _opt_input(input from optarg for -c)
void parse_column_names(vector<string>& _colnames, const string& _opt_input) {
    stringstream sstream(_opt_input);
    string tmp;
    while (sstream >> tmp) {
        _colnames.push_back(tmp);
    }
}

// function to append "text" to end of file
// input: filename (name of file), text (text to add to file), first_input (whether or not this is the first input of the file)
void write_to_file(const string& _filename, const string& _text, bool _first_input) {
    // based on https://stackoverflow.com/questions/26084885/appending-to-a-file-with-ofstream
    // open file to either append or clear file
    ofstream ofile;
    if (_first_input)
        ofile.open(_filename);
    else
        ofile.open(_filename, ofstream::app);
    if (!ofile.is_open()) {
        perror("ofstream open");
        exit(-1);
    }

    // sleep for a random period up to 5 seconds
    usleep(rand() % 5000);

    // add data to csv
    ofile << _text;

    // close file
    ofile.close();
}