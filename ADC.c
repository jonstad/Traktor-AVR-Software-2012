#include <avr/io.h>
#include <avr/interrupt.h>
#include "ADC.h"
#include "USART.h"
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif





void ADC_init(void)
{
	sbi(ADCSRA,ADEN); /*Enable ADC*/
	sbi(ADCSRA,ADSC); /*Start conversion*/
	cbi(ADCSRA,ADATE);/*ADC Auto trigger NOT enabled*/
	/*Set free runing mode*/
	cbi(SFIOR,ADTS2);
	cbi(SFIOR,ADTS1);
	cbi(SFIOR,ADTS0);

	cbi(ADCSRA,ADPS2); /*Prescaler*/
	cbi(ADCSRA,ADPS1);
	cbi(ADCSRA,ADPS0);

	sbi(ADMUX,6);//AVCC with external capacitor at AREF pin
	sbi(ADMUX,ADLAR);
	cbi(ADMUX,MUX4);
	cbi(ADMUX,MUX3);
	cbi(ADMUX,MUX2);
	cbi(ADMUX,MUX1);
	cbi(ADMUX,MUX0);
}
void ADC_init10bit(void)
{
	sbi(ADCSRA,ADEN); /*Enable ADC*/
	sbi(ADCSRA,ADSC); /*Start conversion*/
	sbi(ADCSRA,ADATE);/*ADC Auto trigger enable*/
	/*Set free runing mode*/
	cbi(SFIOR,ADTS2);
	cbi(SFIOR,ADTS1);
	cbi(SFIOR,ADTS0);

	cbi(ADCSRA,ADPS2); /*Prescaler*/
	cbi(ADCSRA,ADPS1);
	cbi(ADCSRA,ADPS0);

sbi(ADMUX,6);//AVCC with external capacitor at AREF pin
	cbi(ADMUX,ADLAR);
	cbi(ADMUX,MUX4);
	cbi(ADMUX,MUX3);
	cbi(ADMUX,MUX2);
	cbi(ADMUX,MUX1);
	cbi(ADMUX,MUX0);
}

char readADC(char nr){
	char retur=0;
	ADMUX=0;
	ADMUX=nr&0x07;
	sbi(ADMUX,ADLAR);
	sbi(ADCSRA, ADIF); /*ADIF is cleared by writing a logical one to the flag.*/
sbi(ADCSRA,ADSC); // Start conversion
	while (bit_is_clear(ADCSRA, ADIF)){}
	retur = ADCH;
	return retur;
}
char readADC10bit(char nr){
	long temp;
	char retur=0;
	ADMUX=0;
	ADMUX=nr&0x07;
	cbi(ADMUX,ADLAR);
	sbi(ADCSRA, ADIF); /*ADIF is cleared by writing a logical one to the flag.*/
	for (temp = 0; temp < 100; temp ++){}

	while (bit_is_clear(ADCSRA, ADIF)){}
	for (temp = 0; temp < 100; temp ++){}
	Term_Send_Value_as_Digits(ADCH);
	Term_Send_Value_as_Digits(ADCL);
	retur = ADCH;
	return retur;
}
char getADCch4(void){
	char retur=0;
	//ADMUX=0;
	while (bit_is_clear(ADCSRA, ADIF)){}
	sbi(ADMUX,6);//AVCC with external capacitor at AREF pin
	cbi(ADMUX,0);
	cbi(ADMUX,1);
	sbi(ADMUX,2);
	sbi(ADMUX,ADLAR);
	sbi(ADCSRA, ADIF); /*ADIF is cleared by writing a logical one to the flag.*/
	sbi(ADCSRA,ADSC);
	while (bit_is_clear(ADCSRA, ADIF)){}
	retur = ADCH;
	return retur;
}
char getADCch0(void){
	char retur=0;
	//ADMUX=0;
	while (bit_is_clear(ADCSRA, ADIF)){}
	sbi(ADMUX,6);//AVCC with external capacitor at AREF pin
	cbi(ADMUX,0);
	cbi(ADMUX,1);
	cbi(ADMUX,2);
	sbi(ADMUX,ADLAR);
	sbi(ADCSRA, ADIF); /*ADIF is cleared by writing a logical one to the flag.*/
	sbi(ADCSRA,ADSC); // Start conversion
	while (bit_is_clear(ADCSRA, ADIF)){}

	retur = ADCH;
	return retur;
}
char getADCch1(void){
	char retur=0;
	//ADMUX=0;
	while (bit_is_clear(ADCSRA, ADIF)){}
	sbi(ADMUX,6);//AVCC with external capacitor at AREF pin
	sbi(ADMUX,0);
	cbi(ADMUX,1);
	cbi(ADMUX,2);
	sbi(ADMUX,ADLAR);
	sbi(ADCSRA, ADIF); /*ADIF is cleared by writing a logical one to the flag.*/
	sbi(ADCSRA,ADSC); // Start conversion
	while (bit_is_clear(ADCSRA, ADIF)){}
	retur = ADCH;
	return retur;
}
char getADCch2(void){
	char retur=0;
	//ADMUX=0;
	while (bit_is_clear(ADCSRA, ADIF)){}
	sbi(ADMUX,6);//AVCC with external capacitor at AREF pin
	cbi(ADMUX,0);
	sbi(ADMUX,1);
	cbi(ADMUX,2);
	sbi(ADMUX,ADLAR);
	sbi(ADCSRA, ADIF); /*ADIF is cleared by writing a logical one to the flag.*/
	sbi(ADCSRA,ADSC); // Start conversion
	while (bit_is_clear(ADCSRA, ADIF)){}
	retur = ADCH;
	return retur;
}
char getADCch3(void){
	char retur=0;
//	ADMUX=0;
	sbi(ADMUX,6);//AVCC with external capacitor at AREF pin
	sbi(ADMUX,0);
	sbi(ADMUX,1);
	cbi(ADMUX,2);
	sbi(ADMUX,ADLAR);
	sbi(ADCSRA, ADIF); /*ADIF is cleared by writing a logical one to the flag.*/
	sbi(ADCSRA,ADSC); // Start conversion
	while (bit_is_clear(ADCSRA, ADIF)){}
	retur = ADCH;
	return retur;
}
char getADCch5(void){
	char retur=0;
//	ADMUX=0;
	sbi(ADMUX,6);//AVCC with external capacitor at AREF pin
	sbi(ADMUX,0);
	cbi(ADMUX,1);
	sbi(ADMUX,2);
	sbi(ADMUX,ADLAR);
	sbi(ADCSRA, ADIF); /*ADIF is cleared by writing a logical one to the flag.*/
	sbi(ADCSRA,ADSC); // Start conversion
	while (bit_is_clear(ADCSRA, ADIF)){}
	retur = ADCH;
	return retur;
}






