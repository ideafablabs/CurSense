# CurSense
Using an ESP8266 to monitor the current being used by our laser to turn on and off the exhaust fan

The hardware has a current sense coil that can sense 100A with a 2000:1 ratio. I ran my wire through the coil twice and put a 22ohm
resistor  as the load. This gives me about .15v per 5A. I offset the output which will be max .4VAC by .55VDC so our ADC will read about
539 or so as zero.

the simple code will sample every 10ms and then subtract 539 to get a value that should be between +- 450 I then get the absolute value
we then look to see if this value is greater then the max we have seen and if it is we will set the max to that value. If it isn't then
we decrement the max that we saved and wait for the next sample. 

This will give us a very simple envelope follower.

We will then keep track of the amount of current we have seen and how long has it been since we have seen it and turn on or off the 
power to the fan.
