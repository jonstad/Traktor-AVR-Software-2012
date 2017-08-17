#include <avr\io.h>
#include <avr\interrupt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include "main.h"
#include "ADC.h"
#include <math.h>

#define debug 0


/*************Variabler for DAC ***********************************/
unsigned int dacverdi=0;

/*************Variabler for Timer Overflow ***********************************/
// use volatile when variable is accessed from interrupts
volatile unsigned int Overflowteller=0;
volatile unsigned char Overflowed =0;
#define MaxOverflow	50

/*************Variabler for PWM ***********************************/
unsigned int  PWMvalue=	200; 	//setter TOP,dvs bestemmer pulsbredden dvs OCR1A
unsigned int  PWMfreq = 400;	//F=FCL/(8*PWMfreq) Setter MAX, dvs bestemmer frekvensen dvs ICR1
#define PWM_IncDec_value  (PWMfreq/255)
 unsigned char PWM_Status[2]={0,0}; //0:M1 , 1:M1
signed int Motor_Target[2]={0,0}; //0:M1 , 1:M1
signed int Motor_Value[2]={0,0}; //0:M1 , 1:M1

/*************Variabler for USART**********************************/
volatile unsigned char Buffersize=4;
volatile unsigned char UART_Counter=0;
volatile unsigned char UART_Rx_Buf[5];
volatile unsigned char UART_Rx_OK =0;
volatile unsigned char UART_Command_Received=0;
volatile unsigned char DAC_Buf[3];
volatile unsigned char PORT_Status[8]={0,0,0,0,0,0,0,0};

/*************Variabler for glatting av ADC****************/
unsigned char Unfiltered_values[8];

SIGNAL(SIG_OVERFLOW0){
	Overflowteller++;
	if(Overflowteller>=MaxOverflow)
		{
			Overflowteller=0;
			UART_Command_Received=0;
			UART_Counter=0;
			UART_Rx_OK=0;
			//Slå av tool dersom det ikke er kommunikasjon
			Overflowed =1;
		}
}

SIGNAL(SIG_UART_RECV)      /* signal handler for receive complete interrupt */
{
	char var=UDR;
	Overflowteller=0;
	if(!UART_Command_Received){
		if(var==TURN_ON_OFF_PIN){
			Buffersize=2;
			UART_Command_Received=1;
		}
		else if(
			var==RETURN_VALUES
			|| 	var == RETURN_VALUES_EXTENDED
			||	var==INC_PWM
			||	var==DEC_PWM
			||	var==TURN_OFF_PIN1
			||	var==TURN_ON_OFF_PIN1
			||	var==TURN_ON_OFF_PIN2
			||	var==TURN_ON_OFF_PIN3
			||	var==TURN_ON_OFF_PIN4
			||	var==TURN_ON_OFF_PIN5
			||	var==TURN_ON_OFF_PIN6
			||	var==GET_VERSION
			||	var==TURN_ON_OFF_PIN2_AND_NOTPIN3
			){
			Buffersize=1;
			UART_Command_Received=1;
		}

		//Set DAC value
		else if(var==DAC_COMMAND ){
			Buffersize=4;
			UART_Command_Received=1;
		}
		else if(var == PROG_ID){
			Buffersize=6;
			UART_Command_Received=1;
		}
		else if(var==GET_ADC_VALUE){
			Buffersize=2;
			UART_Command_Received=1;
		}
		else if(var==DAC_SET_VALUE){
			Buffersize=2;
			UART_Command_Received=1;
		}
	 }
	if(UART_Command_Received){
		UART_Rx_Buf[UART_Counter]=var;
		UART_Counter++;
		if(UART_Counter>=Buffersize){
			UART_Command_Received=0;
			UART_Counter=0;
			UART_Rx_OK=1;
		}
	}
	if(var=='x'){
			UART_Command_Received=0;
			UART_Counter=0;
			UART_Rx_OK=0;
	}
}
unsigned char EEPROM_read(unsigned int uiAddress)
{
		/* Wait for completion of previous write */
		while(EECR & (1<<EEWE));
		/* Set up address register */
		EEAR = uiAddress;
		/* Start eeprom read by writing EERE */
		EECR |= (1<<EERE);
		/* Return data from data register */
		return EEDR;
}

void EEPROM_write(unsigned int uiAddress, unsigned char ucData)
{
	/* Wait for completion of previous write */
	while(EECR & (1<<EEWE));
	/* Set up address and data registers */
	EEAR = uiAddress;
	EEDR = ucData;
	/* Write logical one to EEMWE */
	EECR |= (1<<EEMWE);
	/* Start eeprom write by setting EEWE */
	EECR |= (1<<EEWE);
}
void Pause(void){
	int ii;
	for(ii=0;ii<2000;ii++){
	}
	for(ii=0;ii<2000;ii++){
	}
}

void Received_Command(void){
	UART_Rx_OK=0;
	if(UART_Rx_Buf[0]==DAC_COMMAND){
		dacverdi=((UART_Rx_Buf[3]-48)+(UART_Rx_Buf[2]-48)*10+(UART_Rx_Buf[1]-48)*100);
		if(dacverdi>160){
			dacverdi=160;	//Limit max value to 160
		}
		DAC(DAC_A,dacverdi);

		DAC(DAC_B,dacverdi);
	}
	else if(UART_Rx_Buf[0]==PROG_ID){
		cli();
		EEPROM_write(2,UART_Rx_Buf[1] );
		EEPROM_write(3,UART_Rx_Buf[2] );
		EEPROM_write(4,UART_Rx_Buf[3] );
		EEPROM_write(5,UART_Rx_Buf[4] );
		EEPROM_write(6,UART_Rx_Buf[5] );
		sei();
	}
	else if(UART_Rx_Buf[0]==RETURN_VALUES){

	}
	else if(UART_Rx_Buf[0]==RETURN_VALUES_EXTENDED){

		USART_Transmit(64);		//0
		Term_Send_Buffer(PORT_Status,8);
		USART_Transmit(65);  	//10
		USART_Transmit(66);		//11
		USART_Transmit(67);		//12
		// USART_Transmit(68);
		// USART_Transmit(69);
		// USART_Transmit(70);
		/*	Transmit DAC value i.e. Wheel pressure on pipe wall*/
		//Les strømtrekk på kanal 5 i.e. ADC5
		unsigned long ADCvalue=128;

		ADCvalue=getADCch5();
		Term_Send_Value_as_Digits((char)ADCvalue);	//13 14 15

		Term_Send_Value_as_Digits(dacverdi);

		//Read ADC channel 0
		ADCvalue=getADCch3();
		Term_Send_Value_as_Digits((char)ADCvalue);
		//Pause();
		//Read ADC channel 1
		ADCvalue=0;
		ADCvalue=getADCch1();
		Term_Send_Value_as_Digits((char)ADCvalue);
		//Read ADC channel 2 Temperatur i traktor
		ADCvalue=0;
		ADCvalue=getADCch2();
		Term_Send_Value_as_Digits((char)ADCvalue);

		/*Read ADC channel 4 - Temperatur på PCB*/
		char ADCvalue2=50;
		ADCvalue2=getADCch4();
		Term_Send_Value_as_Digits((char)ADCvalue2);

		Pause();
	}

	else if(UART_Rx_Buf[0]==GET_VERSION){
		/*RETURN VERSION NUMBER AND COMMAND INFORMATION*/
		USART_TransmitString("Ver:");
		Term_Send_Value_as_Digits(0);
		Term_Send_Value_as_Digits(7);	//SET VERSION OF FIRMWARE
		USART_Transmit(LF);//Line feed
		USART_Transmit(CR);//Carriage return
		USART_TransmitString("DAC_COMMAND		'a'	a followed by 3numbers for the DAC");
		USART_TransmitString("TURN_ON_OFF_PIN 	'b'	UART_Rx_Buf[1]=ON/OFF UART_Rx_Buf[2]=PIN NUMBER");
		USART_TransmitString("RETURN_VALUES 	'c'	Return value stored in Buffer");
		USART_TransmitString("GET_ADC_VALUE 	'd'	Return ADC value on pin given");
		USART_TransmitString("TOGGELE PIN1 	'i'	PC0");
		USART_TransmitString("TOGGELE PIN2 	'j'	PD5");
		USART_TransmitString("TOGGELE PIN3 	'k'	PD6");
		USART_TransmitString("TOGGELE PIN4 	't'	PD3");
		USART_TransmitString("TOGGELE PIN5 	'u'	PD7");
		USART_TransmitString("GET_VERSION 		'v'	Return Version nr of firmware");
		USART_TransmitString("TOGGLE PIN2 AND TURN OF PIN3 'm'");
		USART_TransmitString("INC_DAC_VALUE2 	'h'");
		USART_TransmitString("DEC_DAC_VALUE2 	'l'");
		USART_TransmitString("INC_PWM 			'n' increase pulse width");
		USART_TransmitString("DEC_PWM 			'o' decrease pulse width");
		//unsigned char cEprom;
		USART_Transmit((char)EEPROM_read(2));
		USART_Transmit((char)EEPROM_read(3));
		USART_Transmit((char)EEPROM_read(4));
	    USART_Transmit((char)EEPROM_read(5));
		USART_Transmit((char)EEPROM_read(6));
		USART_TransmitString("*****");
	}

	else if(UART_Rx_Buf[0]==GET_ADC_VALUE){
		USART_Transmit((char)readADC(UART_Rx_Buf[1])-180);
	}
	else if(UART_Rx_Buf[0]==DAC_SET_VALUE){
		DAC(DAC_A,UART_Rx_Buf[1]);
		DAC(DAC_B,UART_Rx_Buf[1]);
	}
	/*TOGGLE PIN1  Hight/low Speed*/
	else if(UART_Rx_Buf[0]==TURN_ON_OFF_PIN1){
		if(PORT_Status[0]==0){
				sbi(PORTC,PIN1);	//High Speed
				PORT_Status[0]=1;
				}
			else if(PORT_Status[0]==1){
				cbi(PORTC,PIN1);	//Low Speed
				PORT_Status[0]=0;
				}
			}
	else if(UART_Rx_Buf[0]==TURN_OFF_PIN1){
			cbi(PORTC,PIN1);
			PORT_Status[0]=0;
	}

	/*TOGGLE PIN2 TURN OF PIN3  GO REVERSE*/
	else if(UART_Rx_Buf[0]==TURN_ON_OFF_PIN2){
			if(PORT_Status[1]==0){
				cbi(PORTC,PIN3);//Turn off PIN3
				sbi(PORTC,PIN2);//Turn on PIN2
				PORT_Status[1]=1;
				PORT_Status[2]=0;
				}
			else if(PORT_Status[1]==1){
				cbi(PORTC,PIN2);
				PORT_Status[1]=0;
				}

			}
	/*TOGGLE PIN3 TURN OF PIN2  GO FORWARD*/
	else if(UART_Rx_Buf[0]==TURN_ON_OFF_PIN3){
			if(PORT_Status[2]==0){
				cbi(PORTC,PIN2);//Turn off PIN2
				sbi(PORTC,PIN3);//Turn on PIN3
				PORT_Status[2]=1;
				PORT_Status[1]=0;
				}
			else if(PORT_Status[2]==1){
				cbi(PORTC,PIN3);
				PORT_Status[2]=0;
				}
			}
	/*TOGGLE PIN4 Colapse rear */
	else if(UART_Rx_Buf[0]==TURN_ON_OFF_PIN4){
		if(PORT_Status[3]==0){
				sbi(PORTC,PIN4);
				PORT_Status[3]=1;
				}
			else if(PORT_Status[3]==1){
				cbi(PORTC,PIN4);
				PORT_Status[3]=0;
				}
			}
	/*TOGGLE PIN5 Colapse front */
	else if(UART_Rx_Buf[0]==TURN_ON_OFF_PIN5){
		if(PORT_Status[4]==0){
				sbi(PORTC,PIN5);
				PORT_Status[4]=1;
				}
			else if(PORT_Status[4]==1){
				cbi(PORTC,PIN5);
				PORT_Status[4]=0;
				}
			}
	/*TOGGLE PIN6 Anti-Spin*/
	else if(UART_Rx_Buf[0]==TURN_ON_OFF_PIN6){
		if(PORT_Status[5]==0){
				sbi(PORTC,PIN6);
				PORT_Status[5]=1;
				}
			else if(PORT_Status[5]==1){
				cbi(PORTC,PIN6);
				PORT_Status[5]=0;
				}
			}
	/*TOGGLE PIN2 AND TOGGLE PIN3 INVERS*/
	else if(UART_Rx_Buf[0]==TURN_ON_OFF_PIN2_AND_NOTPIN3){
			if(PORT_Status[1]==0){
				cbi(PORTC,PIN3);//Turn off PIN3
				sbi(PORTC,PIN2);//Turn on PIN2
				PORT_Status[1]=1;
				PORT_Status[2]=0;
				}
			else if(PORT_Status[1]==1){
				cbi(PORTC,PIN2);//Turn off PIN2
				sbi(PORTC,PIN3);//Turn on PIN3
				PORT_Status[1]=0;
				PORT_Status[2]=1;
				}
			}


	else if(UART_Rx_Buf[0]==INC_PWM){
		// if(PWMvalue>=(PWMfreq-PWM_IncDec_value)){
			// PWMvalue=PWMfreq;
		// }
		// else	PWMvalue=PWMvalue+PWM_IncDec_value;

		//Fart allerede på max?
		Motor_Target[0]=Motor_Target[0]+1;
		if(Motor_Target[0]>500){
			Motor_Target[0]=500;
		}
	}
	else if(UART_Rx_Buf[0]==DEC_PWM){
		Motor_Target[0]=Motor_Target[0]-1;
		if(Motor_Target[0]<-500){
			Motor_Target[0]=-500;
		}
		// if(PWMvalue<=(PWM_IncDec_value+10)){
			// PWMvalue=0x0000;
		// }
		// else	PWMvalue=PWMvalue-PWM_IncDec_value;
	}
}
void DAC(char var1,char var2){
		//Var1 = Setup information i.e channel A or channel B
		//Var2 DAC value
		cbi(PORTB,SYNC);
		char ii;
		for(ii=0;ii<8;ii++){
			if(bit_is_set(var1,(7-ii))){
				sbi(PORTB,DIN);
			}
			else {
				cbi(PORTB,DIN);
			}
			cbi(PORTB,SCLK);
			Pause();
			sbi(PORTB,SCLK);
			Pause();
			}
		char jj;
		for(jj=0;jj<8;jj++){
			if(bit_is_set(var2,(7-jj))){
				sbi(PORTB,DIN);
				}
			else {
				cbi(PORTB,DIN);
				}
			cbi(PORTB,SCLK);
			Pause();
			sbi(PORTB,SCLK);
			Pause();
			}
			Pause();
			sbi(PORTB,SYNC);
}

void SPI_MasterInit(void)
{
	/* Set MOSI and SCK output, all others input */
	DDRB = (1<<DIN)|(1<<SCLK)|(1<<SYNC);
	/* Enable SPI, Master, set clock rate fck/16 */
	SPCR = (1<<SPE)|(1<<MSTR);
}
void SPI_MasterTransmit(char cData,char dacData)
{
	cbi(PORTB,SYNC);
	/* Start transmission */
	SPDR = cData;
	/* Wait for transmission complete */
	while(!(SPSR & (1<<SPIF)));
	/* Start transmission */
	SPDR = dacData;
	/* Wait for transmission complete */
	while(!(SPSR & (1<<SPIF)));
	sbi(PORTB,SYNC);
}
void SPI_Transmit(char cData)
{
	cbi(PORTB,SYNC);
	/* Start transmission */
	SPDR = cData;
	/* Wait for transmission complete */
	while(!(SPSR & (1<<SPIF)));
	sbi(PORTB,SYNC);
}





void initTimerOverflow(void){

/**************************************************************************************************************
		Timer 1
***************************************************************************************************************/
	TIMSK=(0<<TOIE1); //Enable/disable timer overflow
	TCNT1=0x0000;
	TCCR1B=0x04;
/**************************************************************************************************************
		Timer 0
***************************************************************************************************************/
	TIMSK=(1<<TOIE0); //Enable/disable timer overflow
	TCNT0=0x0000;
	TCCR0=0x05;
}
void initPWM(void)
{
    TCCR1B = 0x00;  //stop timer
    TCNT1H = 0;
    TCNT1L = 0;

    ICR1   = PWMfreq; // Setter MAX, dvs bestemmer frekvensen
    OCR1A  =0;// PWMvalue;  // setter TOP,dvs bestemmer pulsbredden
	OCR1B  =0;// PWMvalue;  // setter TOP,dvs bestemmer pulsbredden
    TCCR1A = 0x02; //1010 0010=A2  1000 0010=82 Setter aktive kanaler osv.
    TCCR1B = 0x19;  //start timer  0x1A=0001 1010	Tres siste bit bestemmer klokkedeling
}


int main( void )
{
	//Set PORT D as output /
	DDRD = (1<<PD4)|(1<<PD5)|(1<<PD6)|(1<<PD7)|(0<<PD2)|(1<<PD3);
	PORTD=0x00;

	DDRC=0xFF;
	PORTC=0x00;

	DDRB = (1<<SYNC)|(1<<DIN)|(1<<SCLK);
	PORTB=0xff;
	DDRA=0;
	char teller=0;
	USART_Init(25); /* Set the baudrate to x bps using a xMHz crystal */
	DAC(DAC_A,0);
	DAC(DAC_B,0);


	if(debug){
		ADC_init10bit();
	}
	else{
		ADC_init();
	}
	initTimerOverflow();
	initPWM();
	sei();


	while(1){
		Pause();

		if(debug){
		//	readADC10bit(7);
		//	USART_Transmit((char)readADC(7)-180);
			USART_Transmit(LF);
			USART_Transmit(CR);
			USART_Transmit(64);
			long jj;
			for(jj=0;jj<0xffffff;jj++){
			}
		}
		if(UART_Rx_OK==1){
				UART_Rx_OK=0;
				Received_Command();
			}

		//Anti spin funksjon kun tilgjengelig i low speed
		if(PORT_Status[0]==1){
			cbi(PORTC,PIN6);
			PORT_Status[5]=0;
		}

		//Slå av dersom overflowed =1 dvs dersom vi har mistet kommunikasjon
		if(Overflowed==1){
			Overflowed=0;

			cbi(PORTC,PC2);	// Turn of forward
			cbi(PORTC,PC3);	// Turn of reverse
			PORT_Status[1]=0;
			PORT_Status[2]=0;
			// PORT_Status[0]=0;
			// PORT_Status[1]=0;
			// PORT_Status[2]=0;
			// PORT_Status[3]=0;
			// PORT_Status[4]=0;
			// PORT_Status[5]=0;
			// PORT_Status[6]=0;
			// PORT_Status[7]=0;
		}
		//Kontroler PWM
		if(1){
// Motor_Target[0]=-100;

			if(Motor_Target[0]>1)//Forward
				{
					if(Motor_Value[0]<Motor_Target[0]){
						Motor_Value[0]=Motor_Value[0]+1;
					}
					else{
						Motor_Value[0]=Motor_Value[0]-1;
					}
					//kjør i positiv retning
					TCCR1A = 0x82; //1010 0010=A2  1000 0010=82 Setter aktive kanaler osv.
					OCR1A=Motor_Value[0];
					OCR1B=0;
				}
			else if(Motor_Target[0]<-1)//Reverse
				{

					// MotorV =-50 Motor Target -10

					if(Motor_Value[0]>Motor_Target[0]){
						Motor_Value[0]=Motor_Value[0]-1;
					}
					else{
						Motor_Value[0]=Motor_Value[0]+1;
					}
					TCCR1A = 0x22; //1010 0010=A2  1000 0010=82  Setter aktive kanaler osv. 0010 0010 =22
					OCR1B=-Motor_Value[0];
					OCR1A=0;
				}
			else{
				OCR1A=0;
				OCR1B=0;
				TCCR1A = 0x02; //0000 0010=2  0000 0001=1
			}


			}


	}








}
