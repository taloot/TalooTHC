// Arduino RBD Capacitance Library v1.0.4 - Measure change in capacitance.
// https://github.com/alextaujenis/RBD_Capacitance
// Copyright 2015 Alex Taujenis
// MIT License

#include <Arduino.h>
#include <RBD_Capacitance.h> // https://github.com/alextaujenis/RBD_Capacitance

namespace RBD {
  Capacitance::Capacitance(int send_pin, int receive_pin) {
    // set send pin
    _send_pin = send_pin;
    // set receive pin
    _receive_pin = receive_pin;
    // set send pin to output
    pinMode(_send_pin, OUTPUT);
    // turn off the send pin
    digitalWrite(_send_pin, LOW);
    // set the receive pin to input
    pinMode(_receive_pin, INPUT);
  }

  void Capacitance::setSampleSize(int value) {
    if(value > 0) {
      // set the sample size
      _sample_size = value;
      // reset the sample variables
      _sample_sum   = 0;
      _sample_count = 0;
    }
  }

  void Capacitance::start() {
    _startSampling();
  }

  void Capacitance::update() {
    if(_sampling) {
      _takeSample();
    }
  }

  bool Capacitance::isFinished() {
    return _doneSampling();
  }

  unsigned long Capacitance::getValue() {
    return _getAverage();
  }

  bool Capacitance::onChange() {
    if(isFinished()) {
      start();
      return true;
    }
    else {
      return false;
    }
  }


  // private

  void Capacitance::_startSampling() {
    _sampling = true; // take many readings
    _startReading();  // take one reading
  }

  void Capacitance::_startReading() {
    _reading = true;
  }

  bool Capacitance::_doneSampling() {
    return !_sampling;
  }

  bool Capacitance::_doneReading() {
    return !_reading;
  }

  unsigned long Capacitance::_getReading() {
    return _total_reading; // current reading
  }

  unsigned long Capacitance::_getAverage() {
    return _total_average; // average of many readings
  }

  void Capacitance::_takeSample() {
    // keep reading
    _takeReading();

    // when a single reading has been collected
    if(_doneReading()) {
      // increment the sample count
      _sample_count++;
      // sum the reading
      _sample_sum += _getReading();

      // check if we can take another reading for this sample
      if(_sample_count < _sample_size) {
        // restart the reading process again
        _startReading();
      }
      // no more readings for this sample
      else {
        // save a moving average
        _total_average = _sample_sum / _sample_size;
        // reset counter variables
        _sample_sum   = 0;
        _sample_count = 0;
        // stop sampling
        _sampling = false;
      }
    }
  }

  void Capacitance::_takeReading() {
    // only fire this once per reading
    if(!_charging) {
      // CHARGE
      // turn on the send pin
      digitalWrite(_send_pin, HIGH);
      // start counter
      _start = micros();
      // start charging the pin
      _charging = true;
    }

    // wait until the receive pin is charged
    if(bool(digitalRead(_receive_pin))) {
      // save the time in microseconds
      _total_reading = micros() - _start;

      // STOP
      // turn off the send pin
      digitalWrite(_send_pin, LOW);
      // stop charging the pin
      _charging = false;
      // stop taking a reading
      _reading = false;

      // DISCHARGE
      // set receive pin to output
      pinMode(_receive_pin, OUTPUT);
      // set receive pin to low (ground)
      digitalWrite(_receive_pin, LOW);

      // RESET
      // set the receive pin back up to input
      pinMode(_receive_pin, INPUT);
    }
  }
}