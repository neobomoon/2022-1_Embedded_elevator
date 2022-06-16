// DOOR CODE

#include "mbed.h"
#include <string>

#define PWM_MIN 560

#define PWM_MAX 2505
#define MAX_ROTATION 180

#define PWM_CLOSE 1400

// first
#define PWM_FIRST_OPEN_LEFT 2396
#define PWM_FIRST_OPEN_RIGHT 560
#define PWM_FIRST_CLOSE_LEFT 1400
#define PWM_FIRST_CLOSE_RIGHT 1400


// second
#define PWM_SECOND_OPEN_LEFT 550 // 550
#define PWM_SECOND_OPEN_RIGHT 2150 // 2150
#define PWM_SECOND_CLOSE_LEFT 1350 // 1172 // 1350
#define PWM_SECOND_CLOSE_RIGHT 1300 // 1438 // 1438 1350 
//#define PWM_SECOND_CLOSE_RIGHT 1200

#define PWM_THIRD_OPEN_RIGHT 1172
#define PWM_THIRD_OPEN_LEFT 1616
#define PWM_THIRD_CLOSE_RIGHT 2016 
#define PWM_THIRD_CLOSE_LEFT 727

// 1st Floor
PwmOut servoMotor1(D5); // left
PwmOut servoMotor2(D6); // right

// 2nd Floor
PwmOut servoMotor3(D9); // right
PwmOut servoMotor4(D10); // left

// 3rd Floor
PwmOut servoMotor5(D3); // right 
PwmOut servoMotor6(D11); // left


UnbufferedSerial pc(PA_2, PA_3, 115200);

UnbufferedSerial master(D8, D2, 115200);

char rx_buffer[80];
char tx_buffer[80];
int index = 0;
char floor_num;


void rx_handler(){
    char c ;
    if(master.read(&c, 1)){
        pc.write(&c, 1);
        pc.write("\r\n", 2);
        if(c >= '0' && c <='6'){
            floor_num = c;
        }
    }
}

int main()
{
  
  master.format(8, SerialBase::None, 1) ;
  sprintf(tx_buffer, "Door Program_Final (Servo Motor)!!!\r\n");
  pc.write(tx_buffer, strlen(tx_buffer));
  //pc.attach(rx_ISR);
  master.attach(rx_handler, SerialBase::RxIrq);
  servoMotor1.period_ms(20);
  servoMotor2.period_ms(20);
  
  servoMotor3.period_ms(20);
  servoMotor4.period_ms(20);
  
  servoMotor5.period_ms(20);
  servoMotor6.period_ms(20);
  
  servoMotor1.pulsewidth_us(PWM_FIRST_CLOSE_LEFT);
  servoMotor2.pulsewidth_us(PWM_FIRST_CLOSE_RIGHT);
  
  servoMotor3.pulsewidth_us(PWM_SECOND_CLOSE_LEFT);
  servoMotor4.pulsewidth_us(PWM_SECOND_CLOSE_RIGHT);
  
  servoMotor5.pulsewidth_us(PWM_THIRD_CLOSE_RIGHT);
  servoMotor6.pulsewidth_us(PWM_THIRD_CLOSE_LEFT);
  

  int PWM_ON_1;
  int PWM_ON_2;
  int PWM_ON_3;
  int PWM_ON_4;
  int PWM_ON_5;
  int PWM_ON_6;
  
  floor_num = 0;
  
  while(1){
        
    if(floor_num == '0' || floor_num == '1' || floor_num == '2' || floor_num == '3' || floor_num == '4' || floor_num == '5' || floor_num == '6'){
        sprintf(tx_buffer, "floor : %c\r\n", floor_num);
        pc.write(tx_buffer, strlen(tx_buffer));
        
        if(floor_num == '1'){
            PWM_ON_1 = PWM_FIRST_OPEN_LEFT;
            PWM_ON_2 = PWM_FIRST_OPEN_RIGHT;
        }else if (floor_num == '2') {
            PWM_ON_3 = PWM_SECOND_OPEN_LEFT;
            PWM_ON_4 = PWM_SECOND_OPEN_RIGHT;
        }else if (floor_num == '3') {
            PWM_ON_5 = PWM_THIRD_OPEN_RIGHT;
            PWM_ON_6 = PWM_THIRD_OPEN_LEFT;
        }else if(floor_num == '4'){
            PWM_ON_1 = PWM_FIRST_CLOSE_LEFT;
            PWM_ON_2 = PWM_FIRST_CLOSE_RIGHT;
        }else if (floor_num == '5') {
            PWM_ON_3 = PWM_SECOND_CLOSE_LEFT;
            PWM_ON_4 = PWM_SECOND_CLOSE_RIGHT;
        }else if (floor_num == '6') {
            PWM_ON_5 = PWM_THIRD_CLOSE_RIGHT;
            PWM_ON_6 = PWM_THIRD_CLOSE_LEFT;
        }
        
        if (floor_num == '1' || floor_num == '4' ) {
            servoMotor1.pulsewidth_us(PWM_ON_1);
            servoMotor2.pulsewidth_us(PWM_ON_2);
        }
        else if (floor_num == '2' || floor_num == '5') {
            servoMotor3.pulsewidth_us(PWM_ON_3);
            servoMotor4.pulsewidth_us(PWM_ON_4);
        }
        else if (floor_num == '3' || floor_num == '6') {
            servoMotor5.pulsewidth_us(PWM_ON_5);
            servoMotor6.pulsewidth_us(PWM_ON_6);
        }
        else if (floor_num == '0') {
            servoMotor1.pulsewidth_us(PWM_FIRST_CLOSE_LEFT);
            servoMotor2.pulsewidth_us(PWM_FIRST_CLOSE_RIGHT);
            servoMotor3.pulsewidth_us(PWM_SECOND_CLOSE_LEFT);
            servoMotor4.pulsewidth_us(PWM_SECOND_CLOSE_RIGHT);
            servoMotor5.pulsewidth_us(PWM_THIRD_CLOSE_RIGHT);
            servoMotor6.pulsewidth_us(PWM_THIRD_CLOSE_LEFT);
        }
         
    } 
    ThisThread::sleep_for(100ms);   
  }
}