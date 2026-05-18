# User Guide – OpenNerve Gen2 Development Board

This guide describes how to use the OpenNerve Gen2 PCBA as a benchtop development board, powered by USB-C connector, and controlled via BLE using the OpenNerve Windows App.

## How to power the OpenNerve Dev Board

Power can be delivered to the OpenNerve Dev Board over USB-C, avoiding the need for a battery and bypassing the magnetic switch. To power the board by USB-C, you must first attach a jumper between VCHG_RAIL and the outside pin of VBAT2R (see picture below). Then, carefully insert USB-C.

<img width="400" height="400" alt="VCHG update" src="https://github.com/user-attachments/assets/46d7a1ad-cd4a-40ae-9627-fd3bb57334ea" />

<img width="369" height="318" alt="image" src="https://github.com/user-attachments/assets/d07d34a2-3391-41c1-802e-f2275c0f76ab" />

## OpenNerve Windows App Overview
You can run the windows app by either opening the solution file (ONrecorder.sln) in Visual Studios and clicking the green arrow at the top or by running directly from the ONrecorded.exe file. If you are using the App for the first time you will need to clone the code from the repository, open it in Visual Studios, then press the green arrow to compile before the .exe file is generated. Code for the Windows App can be found here: https://github.com/CARSSCenter/OpenNerve-Windows-App

On startup, the App will open to a selection screen listing all OpenNerve devices within BLE range. Each device will be identified by its Bluetooth UUID. Select a device and press connect to start the handshake process.

<img width="420" height="296" alt="Scanning Screen" src="https://github.com/user-attachments/assets/d48af301-5953-4ad3-80e1-aab19b2eb42b" />

Once connected, the control screen will appear. The App and the OpenNerve board will undergo a handshake to authenticate, which may take several seconds. Once authenticated, the "Connected" message will appear and you will be able to record measurements, edit parameters, start and stop stimulation.

<img width="1907" height="1012" alt="Controller Measuring" src="https://github.com/user-attachments/assets/6a237935-b60a-450d-bf4c-d6a954198b60" />

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

There is also an I2C on the OpenNerve board that can be used to communicate with digital sensors. Currently, the only sensor supported is an accelerometer-based motion sensing lead (https://github.com/CARSSCenter/OpenNerve-Standard-Leads/tree/main/AMS%20Lead%20PCBA).

To record signals from one of the sensors, select the desired sensor using the radio buttons at the top of the Windows app. **Start Measuring** will display measurement data in the viewing window. To save this data, select the **Save Data** check button. Data will be saved in a .csv file to the desktop, with a filename defined by the user.

<img width="487" height="142" alt="Measuring" src="https://github.com/user-attachments/assets/03173506-995d-4867-ad3e-6d2cccd36aca" />

Note: signal dropout may occur if there is poor BLE signal connection between the board and the computer. If the signal freezes, try stopping measurement, bringing the board closer to the computer, and restarting.


## Stimulation
The App allows a user to set stimulation parameters and start or stop stimulation. The left side of the controls define parameters for square wave stimulation. Square waves may be applied between any two combinations of Channels 1, 2, 3, or 4. Press **Load Params** to read the current stimulation settings off of the OpenNerve board
* Note: this will take several seconds.

To change a parameter, change the value in the text box and then press **Set**. 
* Note: parameters cannot be changed during stimulation. You may adjust parameters when stimulation is ongoing, but you will need to press Stop Stimulation and then Start Stimulation for the changes to take effect.
* It is often useful to press **Load** after changing a setting to make sure that the OpenNerve board received it correctly.

To start stimulation, press **Start Stimulation**. It will take 2-3 seconds for stimulation to start. Once started, the button will turn red and say **Stop Stimulation**. To stop stimulation, press the button again. Stimulation should stop within 1 second.

Parameter explanation (square wave):
* Cathode – the first electrode under test
* Anode – the second electrode under test
* Amp – current amplitude in milliAmps
* Width – pulse width in microseconds
* Freq – frequency in Hz
* Ramp – the time in which current will ramp from zero to the target amplitude. Set to 0 for testing.
* Train On – the time in seconds at which stimulation will continue. Set to any number for testing
* Train Off – the time in seconds at which stimulation will pause between train cycles. Set to zero for testing

The right side contains a check box saying **use sine wave**. Checking this box puts the device in sine wave mode. In this mode, channels 1 and 2 provide square wave stimulation and channels 3 and 4 provide sine wave stimulation.
* Note: in the current software version (as of May 2026), if Use Sine Wave is checked then channels 1 and 2 will output a square wave and channels 3 and 4 will output a sine wave no matter what channels are selected.

Parameter explanation (sine wave):
* Amp (P-P): peak to peak amplitude of the sine wave. For example, setting this to “4” will set the sine wave to vary from +2 mA to -2 mA.
* Freq: sine wave frequency, from 1 to 10 Hz
* On time: how long to turn the sine wave on
* Off time: off-interval for sine wave. Set to 0 for continuous output

<img width="502" height="712" alt="Sine Parameters" src="https://github.com/user-attachments/assets/f021e19c-974a-4cd0-a940-0a8edbccff8d" />

## Impedance
The “get impedance” button will return the impedance magnitude between the two channels selected in the stimulation side using a series 500us pulse width test pulse. The approximate impedance range that can be measured is 100 Ohms to 5kOhms. See the “Impedance Measurement Strategy” on GitHub for details of how impedance measurement works.

## Developer Options
There is a **Dev Options** check box at the bottom of the form that may display options for development, or features that have not yet been tested. It will also display "Download Logs" and "Clear Logs" buttons. The **Download Logs** button downloads all logs recorded internally in the IPG and saves them to a file in the same folder as ONrecorded.exe. The **Clear Logs** button erases all logs on the IPG.

<img width="1066" height="155" alt="Dev options" src="https://github.com/user-attachments/assets/a56b417d-6485-4593-9f2c-bfb68e20d2aa" />
