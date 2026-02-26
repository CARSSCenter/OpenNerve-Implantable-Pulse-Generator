# User Guide – OpenNerve Gen2 Development Board

This guide describes how to use the OpenNerve Gen2 PCBA as a benchtop development board, powered by USB-C connector, and controlled via BLE using the OpenNerve Windows App.

## Stimulation
1.	Attach jumper between VRECT and the outside pin of VBAT2R (see picture below) to avoid needing magnet to wake up device
2.	Carefully insert USB-C port for power
3.	Open ONrecorder.sln in Visual Studios
4.	Press the green play button at the top of the GUI to start the program
5.	Device will automatically start a Bluetooth handshake that will last several seconds, with steps visible in the bottom left corner of the UI
  a.	During the handshake you will see messages transmitted and received in the “Messages” area of the form (see image below). Once the handshake is complete, you should see “Connected” or “Connected+” in both TX and RX. 
  b.	If no form appears, double check that the jumper is placed and that it is in the correct position (see image below)
  c.	If the Bluetooth handshake does not work, press Quit or the red Stop button on Visual Studios, press the reset button on the OpenNerve board, and try to start the program again
    i.	It is often necessary to press the reset button just before pressing the green Play button
6.	The left side of the controls define parameters for square wave stimulation. Square waves may be applied between any two combinations of Channels 1, 2, 3, or 4. Press “Get Params” to read the current stimulation settings off of the OpenNerve board
  a.	Note: this will take several seconds.
  b.	Parameter explanation:
    i.	Cathode – the first electrode under test
    ii.	Anode – the second electrode under test
    iii.	Amp – current amplitude in milliAmps
    iv.	Width – pulse width in microseconds
    v.	Freq – frequency in Hz
    vi.	Ramp – the time in which current will ramp from zero to the target amplitude. Set to 0 for testing.
    vii.	Train On – the time in seconds at which stimulation will continue. Set to any number for testing
    viii.	Train Off – the time in seconds at which stimulation will pause between train cycles. Set to zero for testing
7.	The right side contains a check box saying “use sine wave”. Checking this box puts the device in sine wave mode. In this mode, channels 1 and 2 provide square wave stimulation and channels 3 and 4 provide sine wave stimulation.
  a.	Note: in the current software version (as of Feb. 2026), if Use Sine Wave is checked then channels 1 and 2 will output a square wave and channels 3 and 4 will output a sine wave no matter what channels are selected.
  b.	Parameter explanation:
    i.	Amp (P-P): peak to peak amplitude of the sine wave. For example, setting this to “4” will set the sine wave to vary from +2 mA to -2 mA.
    ii.	Freq: sine wave frequency, from 1 to 10 Hz
    iii.	On time: how long to turn the sine wave on
    iv.	Off time: off-interval for sine wave. Set to 0 for continuous output
8.	To change a parameter, change the value in the text box and then press “Set”. 
  a.	Note: parameters cannot be changed during stimulation. You may adjust parameters when stimulation is ongoing, but you will need to press Stop Stimulation and then Start Stimulation for the changes to take effect.
  b.	It is often useful to press “Get” after changing a setting to make sure that the OpenNerve board received it correctly.
9.	To start stimulation, press “Start Stimulation”. It will take 2-3 seconds for stimulation to start.
10.	To stop stimulation, press “Stop Stimulation”. Stimulation should stop within 1 second.

<img width="219" height="252" alt="image" src="https://github.com/user-attachments/assets/58b53a4f-19fe-4ba3-8618-6404bfcfe1af" />

<img width="369" height="318" alt="image" src="https://github.com/user-attachments/assets/d07d34a2-3391-41c1-802e-f2275c0f76ab" />


## Impedance
The “get impedance” button will return the impedance magnitude between channels 1 and 2 using a 100us pulse width test stimulation. The approximate impedance range that can be measured is 100 Ohms to 5kOhms. See the “Impedance Measurement Strategy” on GitHub for details of how impedance measurement works.

## Sensing
OpenNerve has four differential amplifier analog front ends (AFEs) for recording biosignals. Two are exposed as header pins: ECG HR is designed to measure cardiac signals, and EMG1 is designed to measure muscle activity. Two others are only accessibly from holes in the board: ECG RR, which is designed to detect respiration and a second EMG AFE (EMG2). Depending on your specific model, the EMG AFEs may be optimized for neural signals (ENG) instead. The approximate filter coefficients and amplification for each AFE are below:
|AFE Specification	|ECG HR	|ECG RR	|ENG	|EMG|
|---|---|---|---|---|
|High-pass filter, Hz	|0.5	|0.09	|7.97	|0.4|
|Low-pass filter, Hz	|83.8	|0.73	|1,750	|665|
|Amplitude min LSB, ±µV	|2.8	|2.8	|0.37	|1.4|
|Amplitude max at 14bit, ±mV	|24	|24	|3	|48|
|Gain for ADC Vref=±3V	|125	|117	|1,005	|250|
|Sampling rate, Hz	|256	|256	|6,400	|2560|

To record signals from one of the amplifiers, select the desired AFE using the radio buttons at the top of the Windows app. Start Viewing will display measurement data in the viewing window. Start Saving will display data and will also save the data to a CSV file on the Desktop with the name entered in the text box.
Note: the software waits until 10k data points are received before updating the viewing window, so a delay may occur between measurement and the viewing port updating. 
Note: signal dropout may occur if there is poor BLE signal connection between the board and the computer. If you notice a delay or see discontinuities in the signal, try bringing the board closer to the computer.
