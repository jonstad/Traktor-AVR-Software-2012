void USART_Init( unsigned int baudrate );
void USART_Transmit( unsigned char data );
void Term_Send_Value_as_Digits(unsigned char value);
unsigned char USART_Getchar(void);
void Term_Send_Buffer(unsigned char str[],char length);