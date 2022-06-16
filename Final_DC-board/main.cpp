#include "mbed.h"

#define MAXIMUM_BUFFER_SIZE 80 
char buf[MAXIMUM_BUFFER_SIZE] ; 

static UnbufferedSerial pc(CONSOLE_TX, CONSOLE_RX, 115200) ;
BusOut motor_out(D8, D9, D10, D11) ;

// receive data from phone
int i = 0 ;
char motor_buf[80] ;
char buffer[80] ;
int step = 0 ;
int dir ;
int flag ;
int index ;
void rx_handler() {
    char c ;
    if ( pc.read(&c, 1) ) {
        pc.write(&c, 1) ;
        if ( c == '\r' ){
            motor_buf[index] = '\0' ;
            index = 0 ;
            
            if ( strcmp(motor_buf, "1") || motor_buf[0] == '2' || motor_buf[0] =='3') {
                dir = atoi(motor_buf) ;  
                flag = 1 ;
                pc.write("Success dir is %d\n", dir) ;    
            }
            else {
                sprintf(buffer, "Input 1 or 2 or 3. Your input is \"%s\"\r\n", motor_buf) ;
                pc.write(buffer, strlen(buffer)) ;
            }
        }
        else if ( c == 8 ) {
            index-- ;
            pc.write(" ", 1) ;
            pc.write(&c, 1) ;
            motor_buf[index] = ' ' ;
        }
        else {
            motor_buf[index++] = c ;
        }
    }
}

int main (void) {
    dir = 0 ;
    flag = 0 ;
    index = 0 ;
    pc.attach(rx_handler, SerialBase::RxIrq) ;
    sprintf(buf, "Test for motor\r\n") ;
    pc.write(buf, strlen(buf)) ;
    
    while (1) {
        if ( dir == 1 || dir == 3 ) {
            switch(step) {
                case 0: motor_out = 0x1; break ;
                case 1: motor_out = 0x3; break ;
                case 2: motor_out = 0x2; break ;
                case 3: motor_out = 0x6; break ;
                case 4: motor_out = 0x4; break ;
                case 5: motor_out = 0xc; break ;
                case 6: motor_out = 0x8; break ;
                case 7: motor_out = 0x9; break ;
                default: motor_out = 0x0; break ;
            }
            
            if (dir == 1) step++; else step-- ;
            if (step > 7) step = 0 ;
            if (step < 0) step = 7 ;
            wait_us(1500) ;
        }
    }
}