#pragma once

#include "Arduino.h"

#include "Ticker.h"

#ifdef __cplusplus
extern "C" {
#endif
void currentSample(void *);
#ifdef __cplusplus
};
#endif

const uint8_t samplesPerSecond = 100;
const uint16_t sampleMax = 512;

class currentSense {
  private:
  uint16_t _curMax;    // the envelope follower value at this moment
  uint16_t _curThreshold;
  uint16_t _curThresholdTime;
  uint32_t _curTimeBelowThreshold;
	Ticker _currentCheck;
  bool _output;
 
  public:
	currentSense(){
	  }
	~currentSense(){}
  void _currentSampleCpp(int16_t value);
  uint16_t getCurrent(void);
  uint16_t getThreshold(void);
  bool setThreshold(uint16_t);
  uint16_t getThresholdTime(void);
  bool setThresholdTime(uint16_t);
  uint16_t getTimeBelowThreshold(void);
  bool getOutput(void);
	
    /**
     * Starts the Ticker
     */
    void begin();

    /**
     * Stops the Ticker
     */
    void end();
};
