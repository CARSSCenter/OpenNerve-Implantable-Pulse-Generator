We created a sine wave generator with tunable amplitude and frequency parameters suitable for testing neuromodulation hardware. The frequency ranges from 0.1 Hz to 5 kHz and the amplitude ranges from 1 mV to 100mV. We first wanted to use a quadrature oscillator circuit, but that required tuning three resistors and capacitors at the same time which was not plausible given the parts we had access to. Then, we pivoted to a different oscillator circuit that only required a a dual gang pot and a double pole rotary switch. Parts to adjust pairs of resistors and pairs of capacitors at the same time were much easier to come by. We implemented JFET Automatic Gain Control (AGC) to account for the loop gain required before the desired amplitude was achieved (setting the loop gain above 1 while amplitude was not reached yet and setting loop gain to be 1 when amplitude was reached) To do so, we needed to find out what resistance the JFET would be at given different GS voltages. We also had to ensure that our rectifier circuit would work and would output a DC voltage proportional to the amplitude of the sine wave Our testing for that is listed in our LTspice simulations (Draft 4 is the new double integrator oscillator, Draft 1 is the old quadrature oscillator, Draft 2 is another quadrature oscillator that didn't work, Draft 3 was to test how to induce noise into the circuit, and Draft 12 was to test the resistance of the JFET given different GS voltages. In the PCB folder, we have our schematic and our layout. We also have Arduino code for the digital version of the sine wave generator that is still being developed. Below are the simulations that we recorded in LTspice:


<img width="1438" height="363" alt="Screenshot 2026-07-23 at 4 37 10 PM" src="https://github.com/user-attachments/assets/fa10e590-4
double integrator oscillator simulation

<img width="1425" height="363" alt="Screenshot 2026-07-23 at 4 40 03 PM" src="https://github.com/user-attachments/assets/5d4dd51c-b829-4bb0-ab8e-538dcb648808" />
JFET resistance simulation

<img width="1430" height="405" alt="Screenshot 2026-07-23 at 4 39 08 PM" src="https://github.com/user-attachments/assets/d2779f9d-3571-4300-b94d-2109828cf336" />
6a8-4d98-8833-aaf59a94a413" />
failed quadrature oscillator simulation

List of Critical Components:

- OPA2188AID Op Amp (https://www.ti.com/lit/ds/symlink/opa2188.pdf?ts=1781621876407&ref_url=https%253A%252F%252Fwww.mouser.com%252F)
  - Zero drift, DC signals don't get integrated
- TLV172IDCK Op Amp (for buffers) (http://www.ti.com/lit/ds/symlink/tlv172.pdf)
  - Unity gain stable
- Rotary Encoder (for capacitor switches) (http://cdn-reichelt.de/documents/datenblatt/C200/DS-Serie%23LOR.pdf)
  - Negligable tracking error 
- USB_C Converter (https://www.usb.org/sites/default/files/documents/usb_type-c.zip)
  - Standard Component 
- SPI DAC (https://www.digikey.com/en/products/detail/analog-devices-inc/AD5683RARMZ/4759188?msockid=34bc058d50b96ab21c66132851e06b0d)
  - Satisfactory resolution 
- TT Electronics Dual Gang Pot (https://www.ttelectronics.com/TTElectronics/media/ProductFiles/Datasheet/PSxx.pdf)
  - Negligable tracking error
 
Future Tests: Upon receiving the manufactured board and the parts, we plan to first solder the components. The immediate next step is to ensure that our power converter is working when plugging in a USB-C cord. It should output 5V as Vcc. If not, we should connect the inductor to the oscilliscope to ensure our inductor is doing the switching movement.  Next, we should prove Vref to ensure that we are getting 2.5V. These two values are the most important to our schematic. Next, we can probe along different parts of our double integrator circuit to ensure that a sine waveform is actually being outputed. Next, we should probe our V_Sin_DC to ensure that we are getting a DC voltage proportional to the amplitude of the sine wave. Luckily, we have many test points that will making the debugging process a lot faster. 
