### Base station components

![](/Images/s01.jpg)

1. RFID module RC522. Pretty common.

2. PCB. Can be orderd viae EasyEDA. Gerber files are in the repository.

3. Microcircuits

a. Microcontroller Atmega328pAU

b. DS3231SN clock

c. Voltage regulator MCP1700T-33.

d. Transistor BC817 SOT-23. Or similar.

4. Passive components in the 0805 format.

a. Resistor 100 Ohm 1 pc

b. Resistor 3.3 kOhm 3 pcs

c. Resistor 33 kOhm 2 pcs

d. 0.1 uF capacitors 4 pcs

e. 1 μF capacitor 2 pcs

g. Inductor 2.2 μH, max current > 500 mA (for example LQH32MN2R2K) 2 pcs, optional for greater antenna power

5. Output components

a. PLS-8 pins (come with RFID board)

b. Connector SIP-6. Can be "obtained" from SIP-40

c. LED 3mm. I'm using blue.

d. Beeper TR1205Y or HC0903A.

6. Battery compartment 3xAA or 3xAAA (the latter is recommended)

7. The case is Gainta G1020BF. A good body made of ABS plastic, can be doped well for your purposes. You can find another one.

Equipment, which is useful for assembly, is given [on a separate page](/Doc/en/Equipment.md)

(Translation below was not updated.)

### Soldering the main board

Going in the bundle to the RFID module pins need to be cut with wire cutters, and bend the LED with the letter G and cut off excess ends. Do not forget about the polarity. The long end of the LED is a plus, in the photo below it is on top.

![](/Images/s03.jpg)

![](/Images/s04.jpg)

We spread all the places of future soldering with flux. Flux is better to smear more.

![](/Images/s05.jpg) 

Next solder conditionally large components: LED, connector PBD-6 (you need to slightly bend the legs), buzzer, pins, battery compartment.
 
![](/Images/s06.jpg)
![](/Images/s07.jpg)

Next solder passive components - resistors and capacitor 0805. Use "third hand". If there is no experience of soldering small components, it would be good to practice first on some unnecessary board. Solder, desolder different parts. You can watch videos on YouTube or somewhere else, the information is pretty much.

![](/Images/s08.jpg) 

Well, the most important part is soldering chips. First the stabilizer, then the watch and the atmegu. Stabilizer and clock are simply soldered. With Atmega, you can tinker if the soldering iron is not very (like mine), then you can make the clips between the adjacent legs, then they have to be removed with a braid. The main thing is not to hurry and try to do everything neatly, it is good to dissolve all the legs. You will most likely need a magnifying glass to check the quality of the soldering.

![](/Images/s09.jpg)

### Bootloader and firmware upload

I upload the boatloader with another Arduino, you can do it with the help of special programmers. Below is described the process using Arduino UNO, in the same way you can use other Arduino, for example, Nano or Pro mini.

First, upload the ArduinoAsISP firmware with Arduino UNO
(File - Examlpes - ArduinoAsISP). On Arduino UNO place a 10 μF capacitor between RST and GND. Connect the pins 11,12,13 to MOSI, MISO, SCK to the programmable board, respectively. Pin 10 connect to the RST of the programmable board and power the board (5 V).

Next, install Optiboot in the IDE:
insert URL with:
https://github.com/Optiboot/optiboot/releases/download/v6.2/package_optiboot_optiboot-additional_index.json
in the "Additional Boards Manager URLs" in File-Prefernces
After this firmware can be installed through the menu Tools - Board - Boards Manager
For the firmware you need to choose:
Board: Optiboot on 32 pins
Processor: ATmega328p
CPU Speed: 8Mhz (int)
Programmer: Arduino As ISP

Click Burn bootloader
If it's OK, an inscription appears
Done burning boatloader
If not, then where is the error, check the correct connection, soldering

Next, disconnect the UNO from the programmable board. In the IDE, change the programmer to "AVRISP mkII". Upload sketch Stantion.ino.
Connect an adapter to the USB-UART board. Connect RST, RX, TX, GND, 3.3V, in some adapters the marking of pins is done nervously and you need to connect RX-RX, TX-TX, in other RX-TX, TX-RX. Also, problems can appear if the RST pin does not work in the adapter, you need to read the reviews before purchasing. Click the sketch download button.

If all is well, then the station should signal three times (which indicates that the clock is working) and after 5 seconds again, again a long time to squeak. If this is the case, then you can proceed with the RFID board. If not, then try to connect the 5 volt power directly to the atmega (output of the RFID comb) and flash it, sometimes it helps. If not, check the soldering.

![](/Images/s10.jpg)

![](/Images/s11.jpg)

![](/Images/s12.jpg)

After successfully loading the bootloader and firmware, you can solder the RFID module. But first he needs to break / emit the LED and the pull-up resistor to the RST line, otherwise all this will consume a lot of current while the device is running.

Also it is necessary to replace the inductances L1 and L2 by analogous 2.2 μH but with a large working current to increase the power of the antennas, for example Murata LQH32MN2R2K

![](/Images/s12_1.JPG)

Apply flux and solder

![](/Images/s13.jpg) 

![](/Images/s14.jpg)

Then we test the work of the station. Preliminary it is necessary to record the master chips of the station number, update the time and a couple of ordinary chips. After inserting the batteries, the station flashes three times, which is also indicated by the fact that the clock is not set correctly. Then in 5 seconds there will be a long signal, signaling that the program has entered the work cycle. By default, this is the sleep mode, so the station will react to the chip setting station number only after 25 seconds after it is presented, but you can immediately put the station on the chip before inserting the batteries. The station will record the new number will reboot, and is now able to respond to conventional chips. We bring the chip, wait until 25 seconds, the station goes into working mode. But the clock is still knocked down. In the working cycle, we present the master-chip of time and after rebooting we present the second usual chip. We read it at the gateway and make sure that it works correctly. If so, the station can be laundered. If not, then you need to look for possible errors, primarily in the RFID module spike.

Antenna power is 48 dB by default, in some stations this power is redundant and leads to the fact that the chips are no longer marked when you bring them close. For stations with this problem, it is necessary to gradually reduce power until an optimal response is achieved. For this in line
uint8_t gain need to reduce the parameter by one (to 0x06 << 4), if the response is still ineffective, try to reduce it further.
 
![](/Images/s15.jpg)

In principle, if the flux is inactive, then the flux can also not be washed off. I washed in alcohol, but there are many different liquids with which you can flush the flux.

![](/Images/s16.jpg)
 
After the flux is washed away, the board looks quite neat. You can pack it.
 
![](/Images/s17.jpg) 

![](/Images/s18.jpg)

### Installation in the case

In the case, you need to cut off the side racks so that the battery pack gets into it. I'm doing this with a clerical knife, cut the plastic well. Try on our board and drill a hole for a 3mm LED.

![](/Images/s20.jpg)

![](/Images/s21.jpg)

Mix epoxy glue and smudge the place for diode insertion

![](/Images/s22.jpg) 

![](/Images/s23.jpg) 

Insert the board, the diode to coat with thicker glue to fit well. On the other hand, wipe the diode with a napkin. And wait until the glue stiffens

![](/Images/s24.jpg)

![](/Images/s25.jpg)

After the glue has solidified, it is possible to fill the compound. In principle, you can do without it. For example, if you blur the joint of the housing with a silicone sealant. Or, in general, glue the lid to the body and make the casing one-time for the life of the batteries (about a year) and change the batteries along with the casing, which may be justified in view of its low cost and reliability of the received stations.

![](/Images/s26.jpg)

At 1 station we measure 30 ml of compound

![](/Images/s27.jpg)

And 1 ml of hardener. Mix the hardener with the compound.

![](/Images/s28.jpg)

![](/Images/s29.jpg)

And fill our board. It should turn out that all contacts are filled with silicone. There are only PBD-6 and a peeple. Inside PBD, too, can be silicone, but it will not interfere with the re-casting of the firmware.

![](/Images/s30.jpg)

![](/Images/s31.jpg)

In a day the compound solidifies completely. In this case, all the details can be seen, in which case the compound can easily be unearthed and repaired.

![](/Images/s32.jpg)

The battery compartment is smeared with petroleum jelly or other thick hydrophobic oil and we insert batteries.
 
![](/Images/s34.jpg)

And twist our body. Parts lie tight, you may need a little effort to close the case.
 
![](/Images/s35.jpg)

![](/Images/s36.jpg)

We paste the label and the light reflectors in the lower part of the station (closer to the antenna) And there is room for the number on top. The station is ready.

![](/Images/s37.jpg)
