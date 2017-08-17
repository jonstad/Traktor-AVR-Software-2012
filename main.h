/************************************/
/************************************/
/*	8MHz clock gives baudrate 19200	*/
/************************************/
/************************************/

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#define SYNC PB4
#define DIN PB5
#define SCLK PB7

// #define PIN1 PD1
// #define PIN2 PD5
// #define PIN3 PD6
// #define PIN4 PD3
// #define PIN5 PD7
#define PIN1 PC1
#define PIN2 PC2
#define PIN3 PC3
#define PIN4 PC0
#define PIN5 PC4
#define PIN6 PC5

#define DAC_COMMAND 'a'		/*a followed by 3numbers for the DAC	*/
#define TURN_ON_OFF_PIN 'b'	/*UART_Rx_Buf[1]=ON/OFF UART_Rx_Buf[2]=PIN NUMBER	*/

#define RETURN_VALUES 'c'	/*Return value stored in Buffer*/
#define RETURN_VALUES_EXTENDED 'C'	/*Return value stored in Buffer extended*/
#define GET_ADC_VALUE 'd'	/*Return ADC value on pin given 	*/
#define DAC_SET_VALUE 'e'	/*Set the DAC value using one Byte*/
#define INC_DAC_VALUE10 'f'	/*Increment DAC Value by 10*/
#define DEC_DAC_VALUE10 'g'	/*Decrement DAC Value by 10*/
#define INC_DAC_VALUE5 'p'	/*Increment DAC Value by 5*/
#define DEC_DAC_VALUE5 'q'	/*Decrement DAC Value by 5*/
#define INC_DAC_VALUE1 'r'	/*Increment DAC Value by 1*/
#define DEC_DAC_VALUE1 's'	/*Decrement DAC Value by 1*/

#define TURN_ON_OFF_PIN1 'i'	/*	PC1					*/
#define TURN_OFF_PIN1 'I'		/*	PC1	High/Low speed	*/
#define TURN_ON_OFF_PIN2 'j'	/*	PC2	Reverse			*/
#define TURN_ON_OFF_PIN3 'k'	/*	PC3	Forward 		*/
#define TURN_ON_OFF_PIN4 't'	/*	PC0	Colapse rear	*/
#define TURN_ON_OFF_PIN5 'u'	/*	PC4	Colapse front 	*/
#define TURN_ON_OFF_PIN6 'w'	/*	PC5	ANTI-SPIN 		*/ 
#define GET_VERSION 'v'	/*Return Version nr of firmware	*/
#define PROG_ID 'V'		/* V followed by 3numbers for the ID	*/

#define TURN_ON_OFF_PIN2_AND_NOTPIN3 'm'	/*Ensure that PIN2 is inverse of PIN3	*/
#define INC_DAC_VALUE2 'h'
#define DEC_DAC_VALUE2 'l'
#define INC_PWM	'n'	//increase pulse width
#define DEC_PWM 'o'	//decrease pulse width


#define DAC_A	32
#define DAC_B	36


#define ON '1'
#define OFF '0'

char LF =0xa;	//Line Feed
char CR =0xd;	//Carriage Return



/***********************************************************************
		PROTOTYPER
***********************************************************************/
void DAC(char var1, char var2);
void Received_Command(void);
char readADC(char nr);
void initPWM(void);
void ADC_init10bit(void);
void EEPROM_write(unsigned int uiAddress, unsigned char ucData);
unsigned char EEPROM_read(unsigned int uiAddress);

