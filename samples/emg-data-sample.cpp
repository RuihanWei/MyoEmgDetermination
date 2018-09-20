// Copyright (C) 2013-2014 Thalmic Labs Inc.
// Distributed under the Myo SDK license agreement. See LICENSE.txt for details.

// This sample illustrates how to use EMG data. EMG streaming is only supported for one Myo at a time.

#include <array>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <array>
#include <stdexcept>
#include <algorithm>
#include <stdexcept>
#include <fstream>
#include <string>
#include <string>
#include<cmath>
#include<windows.h>
#include <time.h>
#include<ctime>

#include <myo/myo.hpp>

using namespace std;

int track = 0;
double threshold = 12.0;
const int interval = 38;
int tot[interval + 5][8];
int triggercount = 0;
bool lasttriggered = false;
//muscle cell action potential lasts about 2-5m

class DataCollector : public myo::DeviceListener {
public:
    DataCollector()
    : emgSamples()
    {
		openFiles();
    }

	void openFiles() {
		time_t timestamp = std::time(0);

		// Open file for EMG log
		if (emgFile.is_open()) {
			emgFile.close();
		}
		std::ostringstream emgFileString;
		emgFileString << "emg12-" << timestamp << ".csv";
		emgFile.open(emgFileString.str(), std::ios::out);
		emgFile << "timestamp,emg1,emg2,emg3,emg4,emg5,emg6,emg7,emg8" << std::endl;
	} 

    // onUnpair() is called whenever the Myo is disconnected from Myo Connect by the user.
    void onUnpair(myo::Myo* myo, uint64_t timestamp)
    {
        // We've lost a Myo.
        // Let's clean up some leftover state.
        emgSamples.fill(0);
    }

    // onEmgData() is called whenever a paired Myo has provided new EMG data, and EMG streaming is enabled.
    void onEmgData(myo::Myo* myo, uint64_t timestamp, const int8_t* emg)
    {
		emgFile << timestamp;
        for (int i = 0; i < 8; i++) {
			emgFile << ',' << static_cast<int>(emg[i]);
            emgSamples[i] = emg[i];
			tot[track][i] = abs(static_cast<int>(emgSamples[i]));
        }
		track++;
		emgFile << std::endl;
		
    }

    // There are other virtual functions in DeviceListener that we could override here, like onAccelerometerData().
    // For this example, the functions overridden above are sufficient.

    // We define this function to print the current values that were updated by the on...() functions above.
    void print()
    {
        // Clear the current line
        std::cout << '\r';

        // OLD Print out the EMG data.
        /*for (size_t i = 0; i < emgSamples.size(); i++) {
            std::ostringstream oss;
            oss << static_cast<int>(emgSamples[i]);
            std::string emgString = oss.str();

            std::cout << '[' << emgString << std::string(4 - emgString.size(), ' ') << ']';
        } */

		int sum = 0;

		int average [8];
		for (int j = 0; j < 8; j++) {
			for (int i = 0; i < interval; i++) {
				sum += tot[i][j];
			}
			average[j] = sum / 8;
			sum = 0;
		}

		for (size_t i = 0; i < emgSamples.size(); i++) {
			sum += average[i];
		}
		sum = sum / static_cast<int>(emgSamples.size());

		// print average EMG data
		for (size_t i = 0; i < emgSamples.size(); i++) {
		std::ostringstream oss;
		oss << average[i];
		std::string emgString = oss.str();

		std::cout << '[' << emgString << std::string(4 - emgString.size(), ' ') << ']';
		}

		//std::cout << std::endl;
		
		if (sum >= threshold) {
			if (!lasttriggered) triggercount++;
			std::cout << "TRIGGERED" << std::flush;
			lasttriggered = true;
		}
		else  {
			std::cout << "NO ACTION" << std::flush;
			lasttriggered = false;
		}
		std::cout << "            trigger count: " << triggercount << flush;
        std::cout << std::flush;
    }

    // The values of this array is set by onEmgData() above.
    std::array<int8_t, 8> emgSamples;
	std::ofstream emgFile;
};

int main(int argc, char** argv)
{
	//char * a;

	//system("capture.exe");
	std::cout << "please input threshold as a number: ";
	cin >> threshold;
	//cout << endl;
	//system("pause");
	
	//std::cout << "hi" << std::endl;
    // We catch any exceptions that might occur below -- see the catch statement for more details.
    try {
    // First, we create a Hub with our application identifier. Be sure not to use the com.example namespace when
    // publishing your application. The Hub provides access to one or more Myos.
    myo::Hub hub("com.example.emg-data-sample");

    std::cout << "Attempting to find a Myo..." << std::endl;

    // Next, we attempt to find a Myo to use. If a Myo is already paired in Myo Connect, this will return that Myo
    // immediately.
    // waitForMyo() takes a timeout value in milliseconds. In this case we will try to find a Myo for 10 seconds, and
    // if that fails, the function will return a null pointer.
    myo::Myo* myo = hub.waitForMyo(10000/20);

    // If waitForMyo() returned a null pointer, we failed to find a Myo, so exit with an error message.
    if (!myo) {
        throw std::runtime_error("Unable to find a Myo!");
    }

    // We've found a Myo.
    std::cout << "Connected to a Myo armband!" << std::endl << std::endl;

    // Next we enable EMG streaming on the found Myo.
    myo->setStreamEmg(myo::Myo::streamEmgEnabled);

    // Next we construct an instance of our DeviceListener, so that we can register it with the Hub.
    DataCollector collector;

    // Hub::addListener() takes the address of any object whose class inherits from DeviceListener, and will cause
    // Hub::run() to send events to all registered device listeners.
    hub.addListener(&collector);

    // Finally we enter our main loop.
    while (1) {
        // In each iteration of our main loop, we run the Myo event loop for a set number of milliseconds.
        // In this case, we wish to update our display 50 times a second, so we run for 1000/20 milliseconds.
        hub.run(1000/20);
        // After processing events, we call the print() member function we defined above to print out the values we've
        // obtained from any events that have occurred.
		if (track ==  interval-1)
			cout << "track == " << endl;
			collector.print();
			track = 0;
    }

    // If a standard exception occurred, we print out its message and exit.
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        std::cerr << "Press enter to continue.";
        std::cin.ignore();
        return 1;
    }
}
