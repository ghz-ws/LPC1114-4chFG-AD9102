#include "mbed.h"

BufferedSerial uart0(P1_7, P1_6,115200);  //TX, RX
SPI spi(P0_9, P0_8, P0_6);    //mosi, miso, sclk
DigitalOut cs1(P0_2);
DigitalOut cs2(P0_3);
DigitalOut cs3(P0_7);
DigitalOut cs4(P0_11);
DigitalOut trig(P1_2);

//cs control func.
void cs_hi(uint8_t num);
void cs_lo(uint8_t num);

//uart read buf
const uint8_t buf_size=21;
void buf_read(uint8_t num); //uart read func.
char read_buf[buf_size];    //uart read buf
void buf2val();             //buf to vals change func. return to 'freq' and 'pha' global var 
uint32_t freq;              //Hz
uint16_t pha,ampl;          //deg. loaded mV
int16_t dof;                //loaded mV
uint8_t type;               //wave type. 0 sin, 1 rampup, 2 rampdown, 3 tri, 4 noise, 5 const

//DDS control
const uint8_t adj_val[4]={32,32,32,32}; //Full scale adjust value
#define res 4           //res=67108864/2^24
uint8_t i;
void adj(uint8_t ch,uint8_t set);       //Full scale adjust set func.
void waveset(uint8_t ch, uint8_t type, uint32_t freq, uint16_t pha, uint16_t ampl, int16_t dof);    //waveset func.
void recset(uint8_t ch);    //not used

//reg. adr. table
const uint16_t REFADJ=0x03;
const uint16_t RAMUPDATE=0x1D;
const uint16_t PAT_STATUS=0x1E;
const uint16_t DACDOF=0x25;
const uint16_t WAVE_CONFIG=0x27;
const uint16_t DAC_CST=0x31;
const uint16_t DAC_DGAIN=0x35;
const uint16_t SAW_CONFIG=0x37;
const uint16_t DDS_TW32=0x3E;
const uint16_t DDS_TW1=0x3F;
const uint16_t DDS_PW=0x43;
const uint16_t DDS_CONFIG=0x45;
const uint16_t STOP_ADDR=0x5E;
const uint16_t PAT_PERIOD=0x29;

int main(){
    trig=1;
    for(i=1;i<=4;++i) cs_hi(i); //CS init
    spi.format(16,0);           //spi mode setting. 2byte(16bit) transfer, mode 0
    
    for(i=1;i<=4;++i) adj(i,adj_val[i-1]);  //refadj

    while (true){
        for(i=1;i<=4;++i){
            buf_read(buf_size);//uart buf read
            buf2val();
            waveset(i,type,freq,pha,ampl,dof);
        }
        trig=1;
        trig=0; 
    }
}

//uart char number read func.
void buf_read(uint8_t num){
    char local_buf[1];
    uint8_t i;
    for (i=0;i<num;++i){
        uart0.read(local_buf,1);
        read_buf[i]=local_buf[0];
    }
}

//buf to val change func.
void buf2val(){
    uint8_t i,j;
    uint32_t pow10;
    freq=0;
    pha=0;
    ampl=0;
    dof=0;
    type=0;
    type=read_buf[0]-48;
    for(i=0;i<8;++i){
        pow10=1;
        for(j=0;j<7-i;++j){
            pow10=10*pow10;
        }
        freq=freq+(read_buf[i+1]-48)*pow10;
    }
    for(i=0;i<3;++i){
        pow10=1;
        for(j=0;j<2-i;++j){
            pow10=10*pow10;
        }
        pha=pha+(read_buf[i+9]-48)*pow10;
    }
    for(i=0;i<4;++i){
        pow10=1;
        for(j=0;j<3-i;++j){
            pow10=10*pow10;
        }
        ampl=ampl+(read_buf[i+12]-48)*pow10;
    }
    for(i=0;i<4;++i){
        pow10=1;
        for(j=0;j<3-i;++j){
            pow10=10*pow10;
        }
        dof=dof+(read_buf[i+17]-48)*pow10;
    }
    if(read_buf[16]==43)dof=dof;
    else if(read_buf[16]==45)dof=-1*dof;
}

//cs control func.
void cs_hi(uint8_t num){
    if(num==1) cs1=1;
    else if(num==2) cs2=1;
    else if(num==3) cs3=1;
    else if(num==4) cs4=1;
}
void cs_lo(uint8_t num){
    if(num==1) cs1=0;
    else if(num==2) cs2=0;
    else if(num==3) cs3=0;
    else if(num==4) cs4=0;
}

//refadj
void adj(uint8_t ch,uint8_t set){
    uint16_t buf;
    cs_lo(i);
    buf=REFADJ;
    spi.write(buf);
    buf=set;
    spi.write(buf);
    cs_hi(i);
}

//waveset func.
void waveset(uint8_t ch, uint8_t type, uint32_t freq, uint16_t pha, uint16_t ampl, int16_t dof){
    uint16_t buf;
    uint32_t reg;
    uint8_t prest, wavetype, sawtype;//wavetype 0=const, 1=saw, 2=radom, 3=dds //sawtype 0=ramp up, 1=ramp down, 2=triangle
    uint8_t step;
    uint16_t dgain;
    if(pha>=360)pha=360;
    if(ampl>=2200)ampl=2200;
    if(dof>=1100)dof=1100;
    if(dof<=-1100)dof=-1100;
    if(type==0){        //sin
        prest=3;
        wavetype=1;
        sawtype=0;
        if(freq>30000000)freq=30000000;
    }else if(type==1){  //ramp up
        prest=1;
        wavetype=1;
        sawtype=0;
        if(freq>4096)freq=4096;
        step=4096/freq;
    }else if(type==2){  //ramp down
        prest=1;
        wavetype=1;
        sawtype=1;
        if(freq>4096)freq=4096;
        step=4096/freq;
    }else if(type==3){  //triangle
        prest=1;
        wavetype=1;
        sawtype=2;
        if(freq>2048)freq=2048;
        step=2048/freq;
    }else if(type==4){  //noise
        prest=2;
        wavetype=1;
        sawtype=0;
        ampl=ampl*13;
    }else if(type==5){  //const
        prest=0;
        wavetype=1;
        sawtype=0;
    }else if(type==6){  //rect
        prest=3;
        wavetype=0;
        sawtype=0;
    }
    
    //waveform config
    cs_lo(ch);
    buf=WAVE_CONFIG;
    spi.write(buf);
    buf=(1<<8)+(prest<<4)+wavetype;
    spi.write(buf);
    cs_hi(ch);

    //saw config
    cs_lo(ch);
    buf=SAW_CONFIG;
    spi.write(buf);
    buf=(1<<8)+(step<<2)+sawtype;
    spi.write(buf);
    cs_hi(ch);

    //freq config
    reg=freq/res;
    cs_lo(ch);
    buf=DDS_TW32;
    spi.write(buf);
    buf=(reg>>8)&0xffff;
    spi.write(buf);
    cs_hi(ch);

    cs_lo(ch);
    buf=DDS_TW1;
    spi.write(buf);
    buf=(reg<<8)&0xffff;
    spi.write(buf);
    cs_hi(ch);

    //pha config
    reg=pha*65536/360;
    cs_lo(ch);
    buf=DDS_PW;
    spi.write(buf);
    buf=reg&0xffff;
    spi.write(buf);
    cs_hi(ch);

    //d gain config
    reg=1000*ampl/-2320;   //1000=2320mVpp (50ohm loaded)
    cs_lo(ch);
    buf=DAC_DGAIN;
    spi.write(buf);
    buf=(reg<<4)&0xffff;
    spi.write(buf);
    cs_hi(ch);

    //d offset config
    reg=1000*dof/-583;  //1000=-583mV (50ohm loaded)
    cs_lo(ch);
    buf=DACDOF;
    spi.write(buf);
    buf=(reg<<4)&0xffff;
    spi.write(buf);
    cs_hi(ch);

    //const val config
    reg=1000*dof/-583;
    cs_lo(ch);
    buf=DAC_CST;
    spi.write(buf);
    buf=(reg<<4)&0xffff;
    spi.write(buf);
    cs_hi(ch);

    //run
    cs_lo(ch);
    buf=PAT_STATUS;
    spi.write(buf);
    buf=0x1;
    spi.write(buf);
    cs_hi(ch);
    
    //io update
    cs_lo(ch);
    buf=RAMUPDATE;
    spi.write(buf);
    buf=0x1;
    spi.write(buf);
    cs_hi(ch);
}

//Write rectanglar pattern to sram
void recset(uint8_t ch){
    uint16_t buf;
    //sram write
    cs_lo(ch);
    buf=PAT_STATUS;
    spi.write(buf);
    buf=1<<2;
    spi.write(buf);
    cs_hi(ch);
    
    cs_lo(ch);
    buf=RAMUPDATE;
    spi.write(buf);
    buf=0x1;
    spi.write(buf);
    cs_hi(ch);
    
    cs_lo(ch);
    buf=0x6000;
    spi.write(buf);
    buf=0x0;
    spi.write(buf);
    buf=0x6005;
    spi.write(buf);
    buf=0xfff0;
    spi.write(buf);
    cs_hi(ch);

    cs_lo(ch);
    buf=PAT_STATUS;
    spi.write(buf);
    buf=0;
    spi.write(buf);
    cs_hi(ch);

    cs_lo(ch);
    buf=RAMUPDATE;
    spi.write(buf);
    buf=0x1;
    spi.write(buf);
    cs_hi(ch);

    //rect. config
    cs_lo(ch);
    buf=DDS_CONFIG;
    spi.write(buf);
    buf=1<<2;
    spi.write(buf);
    cs_hi(ch);
    cs_lo(ch);
    buf=STOP_ADDR;
    spi.write(buf);
    buf=0x5<<4;
    spi.write(buf);
    cs_hi(ch);
    cs_lo(ch);
    buf=PAT_PERIOD;
    spi.write(buf);
    buf=0xa;
    spi.write(buf);
    cs_hi(ch);

    cs_lo(ch);
    buf=RAMUPDATE;
    spi.write(buf);
    buf=0x1;
    spi.write(buf);
    cs_hi(ch);
}