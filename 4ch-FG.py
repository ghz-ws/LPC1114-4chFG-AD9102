import serial
inst=serial.Serial("COM13",115200)
Type=[0,0,0,0]              ##0=sin, 1=Rump Up, 2=Ramp Down, 3=Triangle, 4=Noise, 5=Const
Freq=[1000,1000,1000,1000]  ##kHz unit
Pha=[0,0,0,0]               ##deg. unit
Ampl=[1000,1000,1000,1000]  ##mV unit. 50ohm loaded.
Ofs=[0,0,0,0]               ##mV unit. 50ohm loaded.

buf=f'{Type[0]:01}'+f'{Freq[0]*1000:08}'+f'{Pha[0]:03}'+f'{Ampl[0]:04}'+f'{Ofs[0]:+05}'+f'{Type[1]:01}'+f'{Freq[1]*1000:08}'+f'{Pha[1]:03}'+f'{Ampl[1]:04}'+f'{Ofs[1]:+05}'+f'{Type[2]:01}'+f'{Freq[2]*1000:08}'+f'{Pha[2]:03}'+f'{Ampl[2]:04}'+f'{Ofs[2]:+05}'+f'{Type[3]:01}'+f'{Freq[3]*1000:08}'+f'{Pha[3]:03}'+f'{Ampl[3]:04}'+f'{Ofs[3]:+05}'
inst.write(buf.encode())
print('Ch1 Type=',Type[0],'Freq=',Freq[0],'kHz, Pha=',Pha[0],'deg., Ampl=',Ampl[0],'mV, Ofs=',Ofs[0],'mV\nCh2 Type=',Type[1],'Freq=',Freq[1],'kHz, Pha=',Pha[1],'deg., Ampl=',Ampl[1],'mV, Ofs=',Ofs[1],'mV\nCh3 Type=',Type[2],'Freq=',Freq[2],'kHz, Pha=',Pha[2],'deg., Ampl=',Ampl[2],'mV, Ofs=',Ofs[2],'mV\nCh4 Type=',Type[3],'Freq=',Freq[3],'kHz, Pha=',Pha[3],'deg., Ampl=',Ampl[3],'mV, Ofs=',Ofs[3],'mV\n')