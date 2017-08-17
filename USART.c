#include <avr\io.h>
#include <avr/interrupt.h>
#include "USART.h"


void USART_Init( unsigned int baud )
{
	/* Set baud rate */
	UBRRH = (unsigned char)(baud>>8);
	UBRRL = (unsigned char)baud;
	/* Enable Receiver and Transmitter */
	UCSRB = (1<<RXEN)|(1<<TXEN | 1<< RXCIE);
	/* Set frame format: 8data, 1stop bit */
	//UCSRC = (1<<URSEL)|(3<<UCSZ0);
}

void USART_Init_Getchar(unsigned int baud)
{
	/* Set baud rate */
	UBRRH = (unsigned char)(baud>>8);
	UBRRL = (unsigned char)baud;
	/* Enable Receiver and Transmitter */
	UCSRB = (1<<RXEN)|(1<<TXEN | 0<< RXCIE);	//Turn of RXComplete interupt
}

void Term_Send_Buffer(unsigned char str[],char length){
	int ii;
	for(ii=0;ii<length;ii++){
		USART_Transmit(str[ii]);
	}

}
void Term_Send_Value_as_Digits(unsigned char value){
	unsigned char digit;
	digit='0';
	while(value>= 100)
	{
		digit++;
		value-=100;	
	}
	USART_Transmit(digit);
	digit ='0';
	while(value>= 10)
	{
		digit++;
		value-=10;	
	}
	USART_Transmit(digit);
	USART_Transmit('0'+value);	
}
void USART_Transmit( unsigned char data )
{
/* Wait for empty transmit buffer */
while ( !( UCSRA & (1<<UDRE)) );
/* Put data into buffer, sends the data */
UDR = data;
}

void USART_TransmitString ( const char *str ) { 

     while (*str) { 
         USART_Transmit(*str); 
         str++; 
     }
		USART_Transmit(0xa);//Line feed
		USART_Transmit(0xd);//Carriage return 
 }
unsigned char USART_Getchar(void){
		/* Wait for data to be received */
		while ( !(UCSRA & (1<<RXC)) );
		/* Get and return received data from buffer */
		return UDR;
}


