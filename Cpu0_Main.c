#include "main.h"
#include "Drivers/asclin.h"
#include "my_stdio.h"
#include "Bluetooth.h"
#include "IO/GPIO.h"
#include "Motor.h"
#include "gpt12.h"
#include "IO/ToF.h"
#include "Ultrasonic.h"
#include "Buzzer.h"

#define DIR_B &MODULE_P10,2
#define PWM_B &MODULE_P10,3
#define BREAK_B &MODULE_P02,6

IfxCpu_syncEvent g_cpuSyncEvent = 0;

int sumArr(int *arr, int len) {
    volatile int sum = 0;
    volatile int i;
    for (i = 0; i < len; i++) {
        sum += arr[i];
    }
    return sum;
}

void delay() {
    volatile int i = 0;
    while(i++ < 1000000);
}

int core0_main (void)
{
    IfxCpu_enableInterrupts();

    /* !!WATCHDOG0 AND SAFETY WATCHDOG ARE DISABLED HERE!!
     * Enable the watchdogs and service them periodically if it is required
     */
    IfxScuWdt_disableCpuWatchdog(IfxScuWdt_getCpuWatchdogPassword());
    IfxScuWdt_disableSafetyWatchdog(IfxScuWdt_getSafetyWatchdogPassword());

    /* Wait for CPU sync event */
    IfxCpu_emitEvent(&g_cpuSyncEvent);
    IfxCpu_waitEvent(&g_cpuSyncEvent, 1);

    /* Module Initialize */
    /* Start Code */
    _init_uart3();
    _init_uart1();
    init_gpt2();
    Init_Mystdio();
    Init_Bluetooth();
    Init_Ultrasonics();
    Init_GPIO();
    Init_DCMotors();
    Init_Buzzer();

    int duty = 75;
    char dx, flag;
    volatile int j=0;
    unsigned int timer_end;
    float rear_duration, distance_rear;
    int distance_lazer;


    char save, ch, prev;
    while(1){

        save = getBluetoothByte_nonBlocked();
        //res = _poll_uart3(&ch);

        //lazer control

        distance_lazer = getTofDistance();
        if( save >0 && save >= 'a' && save <= 'z') ch = save;

        if (ch == 'w' && distance_lazer <= 250 && distance_lazer > 0){
                stopChA();
                stopChB();
                ch = ' ';
            }

        /* ULTRASONIC SENSOR DISTANSE & Buzzer control !!!!!!!!!!!!!!!!!!!!!!!!! */

        distance_rear = ReadRearUltrasonic_noFilt();
        if(distance_rear > 40.0f){
            setBeepCycle(0);
        }
        else if (distance_rear > 35.0f) {
            setBeepCycle(500);
        //    delay_ms(7000);
        }
        else if( distance_rear>25.f){
            setBeepCycle(100);
        //    delay_ms(2000);
        }
        else if(ch == 's' && distance_rear <15.f){
            setBeepCycle(1);
            stopChA();
            stopChB();
            ch = ' ';
        }



        delay_ms(10);

    //    bl_printf("Distance_rear: %d\n", (int)distance_rear);



        //motor control

        if(save > 0 ){
            if( save >= 'a' && save <= 'z') ch = save;

            if (ch == 'w' || ch == 'W'){flag = 'w'; dx = 'z';} //forward
            else if(ch == 's' || ch =='S'){flag = 's'; dx = 'z';} //backward
            else if (ch == 'q' || ch == 'Q'){ duty += 10; }//speed up
            else if (ch == 'e' || ch == 'E'){ duty -= 10; }  // speed down
            else if(ch == 'a' || ch == 'A'){    //left
                delay_ms(100);
                dx = 'x';
            }
            else if(ch == 'd' || ch == 'D'){ //right
                delay_ms(100);
                dx = 'y';
            }
            else if(ch == 'c'){flag = 'c';}

            if (duty > 100){duty = 100;}
            else if (duty < 0){duty = 0;}

            if(flag == 'w'){
                if(dx == 'x'){
                    movChA_PWM(0, 1);
                    movChB_PWM(duty, 1);
                    ch = 'w';
                    continue;
                }
                else if(dx == 'y'){
                    movChA_PWM(duty, 1);
                    movChB_PWM(0, 1);
                    ch = 'w';
                    continue;
                }

                movChA_PWM(duty, 1);
                movChB_PWM(duty, 1);
            }

            else if(flag == 's'){
                if(dx == 'x'){
                    movChA_PWM(0, 0);
                    movChB_PWM(duty, 0);
                    ch = 's';
                    continue;
                }
                else if(dx == 'y'){
                    movChA_PWM(duty, 0);
                    movChB_PWM(0, 0);
                    ch = 's';
                    continue;
                }
                movChA_PWM(duty, 0);
                movChB_PWM(duty, 0);
            }
            else if(flag == 'c'){
                stopChA();
                stopChB();

            }
        }

    }


    return 0;

}


