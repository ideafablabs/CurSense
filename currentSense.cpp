/**
 * The MIT License (MIT)
 * Copyright (c) 2017 Michael L. Weiss
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "currentSense.h"

/* from measuring */
#define ANALOG_BASE 539;

static void _currentSample(void *arg)
{
  int16_t value;
  // arg is a pointer to the class object
  
  class currentSense *cs = static_cast <currentSense *>(arg);
    
  /* we need to read the analog value and keep a 20 MAX/MIN samples for an average */

  value = analogRead(A0) - ANALOG_BASE;

  // jump into C++ code 
  
  cs->_currentSampleCpp(value);
}

void currentSense::_currentSampleCpp(int16_t value)
{
  if (value < 0)
  {
    value = -value;
  }

  if (value > sampleMax)
  {
      value = sampleMax;
  }
  
  /* now we have the absolute value */

  if ( value > _curMax)
  {
    _curMax = value;
  }
  else
  {
    /* I think we need to decrement this so we will get a new max at some point */
    if (_curMax > 0)
    {
      _curMax--;
    }
  }

  /*
   * If we are below the threshold and we have not been below it for the threshold time
   * then we should increment the time we are below treshold and if it is now been 
   * long enough below threshold, then turn off the output
   * 
   * if we are not below threshold then turn on the output
   */

  if ((_curMax <= _curThreshold) && ((_curTimeBelowThreshold / samplesPerSecond) < _curThresholdTime))
  {
    // we should be adding up time below the threshold

    _curTimeBelowThreshold++;

    if ((_curTimeBelowThreshold / samplesPerSecond) >= _curThresholdTime)
    {
      // turn it off
      digitalWrite(0, LOW);
      _output = false;
    }
  }
  else if( _curMax > _curThreshold)
  {
    // only turn it on, if it is off

    if (!_output)
    {
      // don't wait turn it on now 
      digitalWrite(0, HIGH);
      _curTimeBelowThreshold = 0;  // reset the time below threshold
      _output = true;
    }
  }
}

void currentSense::begin(void) {
  _curMax = 0;
  pinMode(0, OUTPUT);
  digitalWrite(0, HIGH);    // turn on the output initially
  _output = true;
  _currentCheck.attach((float) 1 / (float) samplesPerSecond, _currentSample, static_cast <void *> (this));
}

void currentSense::end(void) {
	
	_currentCheck.detach();
}

uint16_t currentSense::getCurrent(void) {
  return _curMax;
}

uint16_t currentSense::getThreshold(void) {
  return _curThreshold;
}

bool currentSense::setThreshold(uint16_t newThreshold) {
  _curThreshold = newThreshold;
}

uint16_t currentSense::getThresholdTime(void) {
  return _curThresholdTime;
}

bool currentSense::setThresholdTime(uint16_t newThresholdTime) {
  _curThresholdTime = newThresholdTime;
}

uint16_t currentSense::getTimeBelowThreshold(void) {
  return _curTimeBelowThreshold / samplesPerSecond;
}

bool currentSense::getOutput(void) {
  return _output;
}

