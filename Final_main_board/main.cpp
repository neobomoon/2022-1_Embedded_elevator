// main
#include "mbed.h"
#include "SRF05.h"             
#include "Adafruit_SSD1306.h" 
#include "HX711.h" 
#define SCALE_VALUE 473000

#define MAX_WEIGHT 25

//SRF05 srf05(ARDUINO_UNO_D3, ARDUINO_UNO_D2); 
SRF05 srf05(D6, D5); 
UnbufferedSerial pc(CONSOLE_TX, CONSOLE_RX, 115200) ; 

class SPIPreInit : public SPI
{
public:
    SPIPreInit(PinName mosi, PinName miso, PinName clk) : SPI(mosi, miso, clk)
    { 
        format(8, 3) ; 
        frequency(2000000);  
    };   
}; 

SPIPreInit gSPI(D11, NC, D13) ; 
Adafruit_SSD1306_Spi gOLED(gSPI, D4, D7, D10, 64) ; 

UnbufferedSerial door(D8, D2, 115200) ; 
UnbufferedSerial motor(PA_11, PA_12, 115200) ;


// Open Button 
DigitalIn open_button(PB_13) ; 

// Bell Button & Buzzer 
int period_us;
int beat_ms;
InterruptIn emergency_button(PB_14) ; 
PwmOut buzzer(D9) ; 
Timer _timer ; 

int motor_direction ; 
DigitalOut redLight(D3);
// WeightSensor
HX711 scale(D14,D15);
float calibration_factor = 1000; //-7050 worked for my 440lb max scale setup
int averageSamples = 100;

char buffer[64]; 
char door_buf[32]; 
char motor_buf[32]; 
char next_floor ;
char curr_floor ;
int index = 0 ;
bool open_door ; 

void rx_handler(){
    char c ;
    if( pc.read(&c, 1) ){
        pc.write(&c, 1);
        
        if ( c == '\r' ){
            motor_buf[index] = '\0' ;
            index = 0 ;

            if ( motor_buf[0] == '1' || motor_buf[0] == '2' || motor_buf[0] =='3') {
                open_door = true ; 
                next_floor = motor_buf[0] ;

                pc.write("\n", 1) ;
                
                // current floor: variable name floor
                if ( next_floor > curr_floor ) { 
                    // up 
                    motor_direction = 2;  
                    sprintf(motor_buf, "%d\r", 2) ;
                    motor.write(motor_buf, strlen(motor_buf)) ;
                } else if ( next_floor == curr_floor ) { 
                    // stop 
                    motor_direction = 1;  
                    sprintf(motor_buf, "%d\r", 1) ;
                    motor.write(motor_buf, strlen(motor_buf)) ;
                } else { 
                    // down 
                    motor_direction = 0;  
                    sprintf(motor_buf, "%d\r", 0) ;
                    motor.write(motor_buf, strlen(motor_buf)) ;
                } 
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

void bell_interrupt_route() { 
    int btn_on = emergency_button.read() ; 
    
//    printf("Button read: %d\r\n" , btn_on) ; 
    if ( btn_on == 1) { 
        int curr_motor_direction = motor_direction ; 
        // Stop signal 
        
        sprintf(motor_buf, "%d\r", 1) ;
        motor.write(motor_buf, strlen(motor_buf)) ; 
        
        // Buzzer 
        _timer.reset();
        _timer.start() ; 
        buzzer = 1.0 - 0.05;
          
        period_us = 1000000 / 523.251;
        beat_ms = 62.5 * 4;
        buzzer.period_us(period_us);
//        for (int i = 0 ; i < 1000; i++) ;
        while(std::chrono::duration<float>(_timer.elapsed_time()).count() < 1 ) {}
        //ThisThread::sleep_for(chrono::milliseconds(beat_ms));
        
        _timer.stop();
        
        period_us = 1000000 / 293.665;
        buzzer.period_us(period_us);
        
        _timer.reset();
        _timer.start() ; 
        while(std::chrono::duration<float>(_timer.elapsed_time()).count() < 1 ) {}
        _timer.stop();
//        ThisThread::sleep_for(chrono::milliseconds(beat_ms));
        
        buzzer = 1.0;
        
        _timer.reset();
        _timer.start() ; 
        while(std::chrono::duration<float>(_timer.elapsed_time()).count() < 1 ) {}
        _timer.stop();
//        ThisThread::sleep_for(chrono::milliseconds(100));
        
        sprintf(motor_buf, "%d\r", curr_motor_direction) ; 
        motor.write(motor_buf,strlen(motor_buf)) ; 
    } 
} 

int main()
{
    // bell isr 
    buzzer = 1.0; 
    emergency_button.fall(&bell_interrupt_route);
    buzzer = 1.0; 
    
    next_floor = '1' ;
    curr_floor = '1' ;
    door.format(8, SerialBase::None, 1) ;
    
    ThisThread::sleep_for(5s) ; 
    gOLED.clearDisplay();
    gOLED.printf("\r\n\n\n\n\n\n");
    
    sprintf(buffer, "\r\nEmbeded Fianl Project.\r\n\r\n") ;
    pc.write(buffer, strlen(buffer)) ;
    
    // weight sensor initialization
    scale.setScale(0);
    scale.tare(); //Reset the scale to 0  
    
    // 처음에 1층으로 세팅
    // Close door
    door.write("0", 1) ;
    
    redLight = 1 ;
    
    while( 1 ) {
        char c ;
        float distance = srf05.read() ; 
        
        if (distance < 2.4) { // up
            motor_direction = 2;  
            sprintf(motor_buf, "%d\r", 2) ;
            motor.write(motor_buf, strlen(motor_buf)) ;
        }
        else if (distance > 3.2) { // down
            motor_direction = 0;  
            sprintf(motor_buf, "%d\r", 0) ;
            motor.write(motor_buf, strlen(motor_buf)) ;
            
            printf("TEST\r\n"); 
        }
        else { // stop
            motor_direction = 1;  
            sprintf(motor_buf, "%d\r", 1) ;
            motor.write(motor_buf, strlen(motor_buf)) ;
            break ;
        }
    }
    
    redLight = 0 ; 
    sprintf(buffer, "End init.\r\n\r\n") ;
    pc.write(buffer, strlen(buffer)) ;
    
    pc.attach(rx_handler, SerialBase::RxIrq) ;
    
    while(1) { 
        
        float distance = srf05.read() ; 
        
        gOLED.setTextCursor(8 * 6, 4 * 8) ; 
        gOLED.setTextSize(3); 
        
        char c ; 
        
        scale.setScale(calibration_factor); //Adjust to this calibration factor
        float weight = -1 ;  // = -1 * scale.getGram();
        
        ThisThread::sleep_for(20ms) ; 
        
//        sprintf(buffer, "Weight: %.2f\r\n", weight) ; 
//        pc.write(buffer, strlen(buffer)) ; 
        
        sprintf(buffer, "curr distance: %.2f\r\n", distance); 
        pc.write(buffer, strlen(buffer)) ; 
        
        if ( distance < 26 ) {
            
            if ( open_door && next_floor == '1' && distance >= 2.4 && distance <= 3.2) { // 딱 1층일 때 open
                curr_floor = '1' ; 
                
                gOLED.printf("%cF", curr_floor); 
                gOLED.display() ;
                
                motor_direction = 1;  
                // motor stop 
                sprintf(motor_buf, "%d\r", 1) ;
                motor.write(motor_buf, strlen(motor_buf)) ;
                
                // wait 
                ThisThread::sleep_for(1000ms) ; // 1 sec 
                
                // First floor door open 
                door.write("1", 1) ;
                
                ThisThread::sleep_for(2000ms) ; // 2 sec 
                
                // Open door by button 
                while ( open_button == 1 ) {} 
                
                weight = -1 * scale.getGram();
                sprintf(buffer, "The value %.2f\r\n", weight);
                pc.write(buffer, strlen(buffer));
                
                while (weight > MAX_WEIGHT){
                    redLight = 1;
                    
                    sprintf(buffer, "MAX WEIGHT REACHED: (CURRENT)%.2f\r\n", weight);
                    pc.write(buffer, strlen(buffer));
                    ThisThread::sleep_for(2000ms) ; // 2 sec 
                    
                    weight = -1 * scale.getGram();
                } 
                
                redLight = 0;
                
                // FIrst floor door close
                door.write("4", 1) ;
                
                open_door = false ; 
            }
        } else if ( open_door && next_floor == '2' &&  distance >= 26 && distance < 45.6 ) {
            if ( distance <= 26.8 ) { // 딱 2층일 때 open
                curr_floor = '2' ; 
                
                gOLED.printf("%cF", curr_floor); 
                gOLED.display() ;
                // motor stop 
//                c = '1' ; 
//                motor.write(&c, 1) ;
                motor_direction = 1;  
                sprintf(motor_buf, "%d\r", 1) ;
                motor.write(motor_buf, strlen(motor_buf)) ;
                
                // wait 
                ThisThread::sleep_for(1000ms) ; // 1 sec 
                
                // Second floor door open 
                door.write("2", 1) ;
                
                ThisThread::sleep_for(2000ms) ; // 2 sec 
                
                // Open door by button 
                while ( open_button == 1 ) {} 
                
                weight = -1 * scale.getGram();
                sprintf(buffer, "The value %.2f\r\n", weight);
                pc.write(buffer, strlen(buffer));
                
                while (weight > MAX_WEIGHT){
                    redLight = 1;
                    
                    sprintf(buffer, "MAX WEIGHT REACHED: (CURRENT)%.2f\r\n", weight);
                    pc.write(buffer, strlen(buffer));
                    ThisThread::sleep_for(2000ms) ; // 2 sec 
                    
                    weight = -1 * scale.getGram();
                } 
                
                redLight = 0;
                
                
                // Second floor door close
                door.write("5", 1) ;
                
                open_door = false ;
            }
        } else {
            
            if ( open_door && next_floor == '3' && distance >= 47 && distance <= 47.8 ) { // 딱 3층일 때 open
                
                curr_floor = '3'; 
                gOLED.printf("%cF", curr_floor); 
                gOLED.display() ;
                // motor stop 
//                c = '1' ; 
//                motor.write(&c, 1) ;
                motor_direction = 1;  
                sprintf(motor_buf, "%d\r", 1) ;
                motor.write(motor_buf, strlen(motor_buf)) ;
                
                // wait 
                ThisThread::sleep_for(1000ms) ; // 1 sec 
                
                // Third floor door open
                door.write("3", 1) ;
                
                ThisThread::sleep_for(2000ms) ; // 2 sec 
                
                // Open door by button 
                while ( open_button == 1 ) {} 
                
                weight = -1 * scale.getGram();
                sprintf(buffer, "The value %.2f\r\n", weight);
                pc.write(buffer, strlen(buffer));
                
                while (weight > MAX_WEIGHT){
                    redLight = 1;
                    
                    sprintf(buffer, "MAX WEIGHT REACHED: (CURRENT)%.2f\r\n", weight);
                    pc.write(buffer, strlen(buffer));
                    ThisThread::sleep_for(2000ms) ; // 2 sec 
                    
                    weight = -1 * scale.getGram();
                } 
                
                redLight = 0;
                
                // Third floor door close
                door.write("6", 1) ;
                
                open_door = false ;
            }
        }   
        
        ThisThread::sleep_for(100ms) ; // 1 sec 
    }  
    
    return 0 ;
}
