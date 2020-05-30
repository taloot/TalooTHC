// Arduino RBD Capacitance Library v1.0.4 - Measure change in capacitance.
// https://github.com/alextaujenis/RBD_Capacitance
// Copyright 2015 Alex Taujenis
// MIT License

#ifndef RBD_CAPACITANCE_H
#define RBD_CAPACITANCE_H

#include <Arduino.h>

namespace RBD {
  class Capacitance {
    public:
      Capacitance(int send_pin, int receive_pin); // constructor
      void setSampleSize(int value); // how many readings to take per sample
      void start();                  // begin a sample and take many readings: alias of _startSampling()
      void update();                 // keep this class updating in real-time
      bool isFinished();             // if all readings have been taken for this sample: alias of _doneSampling()
      unsigned long getValue();      // the sum of readings divided by the sample size: alias of _getAverage()
      bool onChange();               // returns true once the getValue changes, then resets to false and waits for another change
    private:
      int _send_pin;
      int _receive_pin;
      int _sample_size  = 6;       // how many readings should be taken for one sample
      int _sample_count = 0;         // how many readings have been taken for the current sample
      bool _sampling    = false;     // true if taking a sample
      bool _charging    = false;     // true if charging the pin for a reading
      bool _reading     = false;     // true if finished with one reading
      unsigned long _sample_sum = 0; // temp value that holds the sum of all readings for this sample
      unsigned long _start;          // the time in microseconds that a reading was started
      unsigned long _total_reading;  // the time in microseconds how long a reading took
      unsigned long _total_average;  // the average time in microseconds for the last 'sample size' number of readings
      unsigned long _getReading();   // returns the value of a single reading when done
      unsigned long _getAverage();   // returns the sum of readings divided by the sample size
      void _startSampling();         // kick off the process to take many readings
      bool _doneSampling();          // returns true if all readings have been taken for this sample
      void _takeSample();            // manage the number of readings collected for this sample and calculate the moving average
      void _startReading();          // kick off the process to take a single reading
      bool _doneReading();           // returns true if finished taking a single reading
      void _takeReading();           // performs the actual reading of pin charge and discharge
  };
}
#endif
