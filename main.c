#ifndef F_CPU
#define F_CPU	16000000ul
#endif
#include <avr/io.h>
#include <util/delay.h>
#include "servo.h"
#include "h_bridge.h"
#include <avr/interrupt.h>
/*
servo 1 in op -10
servo 1 uit op -65
servo 2 in op -80
servo 2 uit op -20
*/
//      Tilhulp
//  Buttons             PIN
#define opbouwen        PF3
#define afbouwen        PF6
#define noodstop        PD1

//  Output
#define Buzzer          PF0
#define Limit_top       PK7
#define Limit_bottom    PK6
// Servo_1         PH4
//Servo_2         PH3
// DC_motor_in     PG5
// DC_motor_out    PE3
//  Leds
#define D1     PF1 //Led_Knop_opbouwen
#define D2     PF2 //Led_knop_afbouwen
#define D3     PK1 //Led_Working
#define D4     PK2 //Led_noodstop
#define D5     PK3 //led_in_noodstop

//Button functies
static int button_opbouwen(void)
    {
        if (PINF & (1<<opbouwen))
        {
            _delay_ms (5);
            if (PINF & (1<<opbouwen))
            {
                return 0;
            }
        }
        return 1;
    }
static int button_afbouwen(void)
    {
        if (PINF & (1<<afbouwen))
        {
            _delay_ms (5);
            if (PINF & (1<<afbouwen))
            {
                return 0;
            }
        }
        return 1;
    }

static int button_LimitTop(void)
    {
        if (PINK & (1<<Limit_top))
        {
            _delay_ms (5);
            if (PINK & (1<<Limit_top))
            {
                return 0;
            }
        }
        return 1;
    }
    static int button_LimitBottom(void)
    {
        if (PINK & (1<<Limit_bottom))
        {
            _delay_ms (5);
            if (PINK & (1<<Limit_bottom))
            {
                return 0;
            }
        }
        return 1;
    }

static int button_noodstop(void)
    {
        if (PIND & (1<<noodstop))
        {
            _delay_ms (5);
            if (PIND & (1<<noodstop))
            {
                return 0;
            }
        }
        return 1;
    }





//led functies
static void Led_Knop_Op_off(void){PORTK &= ~(1<<D1);}
static void Led_Knop_Op_on(void){PORTK |= (1<<D1);}
static void Led_Knop_Af_off(void){PORTK &= ~(1<<D2);}
static void Led_Knop_Af_on(void){PORTK |= (1<<D2);}
static void Led_working_off(void){PORTF &= ~(1<<D3);}
static void Led_working_on(void){PORTF |= (1<<D3);}
static void Led_noodstop_off(void){PORTF &= ~(1<<D4);}
static void Led_noodstop_on(void){PORTF |= (1<<D4);}
static void Led_in_noodstop_on(void){PORTK |= (1<<D5);}



void init_timer(void)
{
    TCCR3A = 0;
    TCCR3B = (0<<CS32)|(1<<CS31)|(1<<CS30);
}
void delay(int time)
{
    int timer = 0;
    TCNT3=0;
    TIFR3= (1<<TOV3);
    while (timer< time)
    {
        if (TIFR3&(1<<TOV3))
        {
            TIFR3= (1<<TOV3);
            timer++;
        }
    }
}




static void init(void)
{
    DDRF |= (1<<D3)|(1<<D4);
    DDRK |= (1<<D1)|(1<<D2)|(1<<D5);

    PORTK |= (1<<PK3);//led in noodstop
    init_servo();
    init_h_bridge();
    init_timer();

}
int main(void)
{
    init();

    int stop = 0;
    int i = 1;
    int i2 = 1;
    Led_in_noodstop_on();
    //int State_Op = 0;
    while(1)
    {

    if (button_noodstop()){
        Led_noodstop_on();
        stop = 1;
    }
    else{
        Led_noodstop_off();
        stop = 0;
    }


        if (stop == 0)
        {
            if (button_opbouwen())
            {
                Led_Knop_Op_on();
                switch (i)
                {
                case 1: //steigerdeel oppakken
                    {
                    // Led_working aan
                    Led_working_on();
                    // Haakjes_uit
                    servo1_set_percentage(-65); // out
                    servo2_set_percentage(-20); //out
                    // Motor 1s naar boven
                    delay(10);
                    h_bridge_set_percentage(-100);
                    delay(2);
                    h_bridge_set_percentage(0);
                    //State_Op = 1;
                    i = 2;
                    stop = 1;
                    break;
                    }
                case 2: // steiger omhoog tillen
                    {
                        while(!button_LimitTop())
                        {
                            if (!button_noodstop()){
                                h_bridge_set_percentage(-100);
                                Led_noodstop_off();
                            }
                            else{
                                h_bridge_set_percentage(0);
                                Led_noodstop_on();
                            }
                        }
                        h_bridge_set_percentage(0);
                    i = 3;
                    stop = 1;
                    break;
                    }
                case 3: // Steiger plaatsen
                    {
                    //steiger plaatsen
                    h_bridge_set_percentage(100);
                    delay(4);
                    h_bridge_set_percentage(0);
                    i = 4;
                    stop = 1;
                    break;
                    }
                case 4: // back to Idle
                    {
                    servo1_set_percentage(-10); //in
                    servo2_set_percentage(-80); //in
                    delay(2);
                    while(!button_LimitBottom())
                    {
                        if(!button_noodstop()){
                            h_bridge_set_percentage(100);
                            Led_noodstop_off();
                        }
                        else{
                            h_bridge_set_percentage(0);
                            Led_noodstop_on();
                        }
                    }
                    h_bridge_set_percentage(0);
                    i = 1;
                    stop = 1;
                // State_Op = 0;
                    // Led_working uit & led knop op uit
                    //Led_Knop_Af_off();
                    Led_working_off();
                    Led_Knop_Op_off();
                    //Led_noodstop_on();
                    break;
                    }
                default: // knop_afbouwen indrukken??
                    {
                    // alles naar idle
                    //PORTB &= ~(1<<D3);
                    //PORTB &= ~(1<<D4);
                    i =  1;
                    break;
                    }
                }// end switch case
            }// end if opbouw

            if (button_afbouwen())
            {
                // led knop_Af aan
                Led_Knop_Af_on();
                switch (i2)
                {
                case 1: //steigerdeel 2 oppakken
                    {
                    // Led_working aan.
                    Led_working_on();
                    // Motor naar boven met delay
                    while (!button_LimitTop())
                        {
                        if (!button_noodstop()){
                            h_bridge_set_percentage(-100);
                            Led_noodstop_off();
                        }
                        else{
                            h_bridge_set_percentage(0);
                            Led_noodstop_on();
                        }
                    }
                    h_bridge_set_percentage(0);
                    delay(3);
                    h_bridge_set_percentage(100);
                    delay(4);
                    h_bridge_set_percentage(0);
                    // haakjes uit

                    i2 = 2;
                    stop = 1;
                    break;
                    }
                case 2: // 2de steiger optillen
                    {
                    // Motor omhoog in beetje om steiger los te maken
                    servo1_set_percentage(-65);
                    servo2_set_percentage(-20);
                    delay(6);
                    while (!button_LimitTop())
                        {
                        if (!button_noodstop()){
                            h_bridge_set_percentage(-100);
                            Led_noodstop_off();
                        }
                        else{
                            h_bridge_set_percentage(0);
                            Led_noodstop_on();
                        }

                    }
                    h_bridge_set_percentage(0);

                    i2 = 3;
                    stop = 1;
                    break;
                    }
                case 3: // Steiger naar beneden plaatsen & idle
                    {
                    // Motor naar beneden
                    while(!button_LimitBottom())
                    {
                        if(!button_noodstop()){
                            h_bridge_set_percentage(100);
                            Led_noodstop_off();
                        }
                        else{
                            h_bridge_set_percentage(0);
                            Led_noodstop_on();
                        }
                    }
                    h_bridge_set_percentage(0);
                    // haakjes_in
                    delay(10);
                    servo1_set_percentage(-10);
                    servo2_set_percentage(-80);
                    Led_Knop_Af_off();
                    Led_working_off();
                    i2 = 1;
                    stop = 1;
                    break;
                    }
                default: // knop_opbouwen indrukken
                    {
                    //PORTB &= ~(1<<D1);
                    //PORTB &= ~(1<<D2);
                    // alles naar idle
                    i2 =  1;
                    break;
                    }
                }// end switch case
            }//end if afbouw
            }// if stop functie
        else if ((!button_opbouwen())&&(!button_afbouwen()))
            {
                stop = 0;
            }



    }
    return 0;
}
