#include "utils.hpp"


void logError (string error) {
    // Gets the time into string format
    time_t now = time(0);
    tm* tm = localtime(&now);
    stringstream time;
    time << put_time(tm, "%Y-%m-%d %H:%M:%S");

    // Logs the error onto the file
    ofstream file;
    file.open("error.log", ios::app);
    file << "[" << time.str() << "]" << error << endl;
    file.close();
}
