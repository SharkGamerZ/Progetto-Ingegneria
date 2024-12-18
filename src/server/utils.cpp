#include "utils.hpp"


void logError (string filename, int line, string error) {
    // Gets the time into string format
    time_t now = time(0);
    tm* tm = localtime(&now);
    stringstream time;
    time << put_time(tm, "%Y-%m-%d %H:%M:%S");


    // Builds the error message
    error = "Error in " + filename + " at line " + to_string(line) + ": " + error;


    // Logs the error onto the file
    ofstream file;
    file.open("../monitors/error.log", ios::app);
    file << "[" << time.str() << "]" << error << endl;
    file.close();
}
