#include "mbed.h"

#define MAXIMUM_BUFFER_SIZE 80
char buf[MAXIMUM_BUFFER_SIZE];

UnbufferedSerial pc(CONSOLE_TX, CONSOLE_RX, 115200);
static UnbufferedSerial main_pc(PA_11,PA_12,115200);

BusOut motor_out(D8,D9,D10,D11);  // blue - pink - yellow - orange

int step = 0; 
int dir = 2; // direction
char rxBuf_pc[80];
int index = 0;
int flag = 0;

void rx_cb(void)
{
    char ch;
    main_pc.read(&ch,1);
    pc.write(&ch,1);
    
    if(ch == '\r'){
        ch = '\n';
        pc.write(&ch, 1);
        rxBuf_pc[index] = '\0';
        index = 0;  
        flag = 1;
    }else if(ch == 8){
        index--;
        pc.write(" ",1);
        pc.write(&ch,1);
        rxBuf_pc[index] = ' ';   
    }else{
        rxBuf_pc[index++] = ch;
    }
}

int main()
{   
    main_pc.attach(rx_cb);
    while(1)
    {   
        //2 up
        //1 stop
        //0 down 
        
        if(flag){
            
            if(!strcmp(rxBuf_pc,"0"))
            {
                dir = 0;
            }
            else if(!strcmp(rxBuf_pc,"1"))
            {
                dir = 2;
            }
            else if(!strcmp(rxBuf_pc,"2"))
            {
                dir = 1;
            }
            memset(rxBuf_pc,0,sizeof(rxBuf_pc));
            flag = 0;
        }

        if(dir <= 1){
            switch(step)
            { 
                case 0: motor_out = 0x1; break;  // 0001
                case 1: motor_out = 0x3; break;  // 0011
                case 2: motor_out = 0x2; break;  // 0010   
                case 3: motor_out = 0x6; break;  // 0110
                case 4: motor_out = 0x4; break;  // 0100
                case 5: motor_out = 0xC; break;  // 1100
                case 6: motor_out = 0x8; break;  // 1000
                case 7: motor_out = 0x9; break;  // 1001
                
                default: motor_out = 0x0; break; // 0000
            }
      
            if(dir) step++; else step--; 
            if(step>7)step=0; 
            if(step<0)step=7; 
            wait_us(1500);  // speed
        }
    }
}
