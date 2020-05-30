#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/boot.h>
#define F_CPU 12000000
#define BAUD_RATE 9600
#include <util/delay.h>
unsigned char address_byte_one,address_byte_two;
unsigned char data[64];
unsigned int Page_no=1;
void application (void) __attribute__ ((section (".app")));//this is default application
/////// /////////I do not want to burn boot reset fuse as I will be using same chip for a lot of other purposes////
void afterReset (void) __attribute__((section(".myinit")));
/////////////////so I written a code that let me jump to the main /////////////////////////////////////////////////
void eeprom_write(unsigned int address,unsigned char data);					      
void usart_init(void);
unsigned char usart_read(void);
void usart_write(unsigned char data);
 int main() {
	unsigned char ch;
	int i;
	usart_init();   
    PORTB|=(1<<PB2); 
	while((ch=usart_read())!='P')
	{
		
	}
	if(ch=='P'){// yes if host wants to program
		usart_write('o'); //ack
		ch=usart_read();
		if(ch=='d') // host sending data
		{ 	
			usart_write('o');  
			get_the_page:
			for(i=0;i<64;i++){
				PORTB|=(1<<PB2);
				data[i]=usart_read();//getting data
				PORTB&=~(1<<PB2);
				}
/////////////////////////////////////////////Writing the data/////////////////////////////////////////////		
			asm volatile (
		"myinit%=: \n\t"
		//setting lock bits
			"ldi r17,0xFF \n\t"
			"mov r0,r17 \n\t"
			"sts %0,(1<<3)|(1<<0) \n\t"
			"rcall do_spm%= \n\t"
		//initialising Z pointer
			"clr r30 \n\t"
			"clr r31 \n\t"
			"lds r25,Page_no \n\t"
		"initZ%=: \n\t"
			"adiw r30,63 \n\t"
			"adiw r30,1  \n\t"
			"dec r25\n\t"
			"brne initZ%=\n\t"
			"ldi r17,0x20 \n\t"
		//initialing Y pointer
			"ldi r29,hi8(data) \n\t"
			"ldi r28,lo8(data) \n\t"
		//erasing page pointed by Z	
			"ldi r16,(1<<1)|(1<<0) \n\t"
			"rcall do_spm%= \n\t"
		//reenabling RWW section	
			"ldi r16,(1<<4)|(1<<0) \n\t"
			"rcall do_spm%= \n\t"
	    "buffer_write_loop%=: \n\t"
		//writing to page buffer now set the data in r1:r0	
			"ld r0,Y+ \n\t"
			"ld r1,Y+ \n\t"
		//writing in buffer the word pointed by Z
			"ldi r16,(1<<0) \n\t"
			"rcall do_spm%= \n\t"
			"adiw r30,2 \n\t"
			"dec r17 \n\t"
			"brne buffer_write_loop%= \n\t"
		//writing the buffered page in flash restoring Z pointer to start of page	
			"sbiw r30,63 \n\t"
			"sbiw r30,1 \n\t"
			"ldi r16,(1<<2)|(1<<0) \n\t"
			"rcall do_spm%= \n\t"
		//reenabling RWW section	
			"ldi r16,(1<<4)|(1<<0) \n\t"
			"rcall do_spm%= \n\t"
			"rjmp write_done%= \n\t"
		"do_spm%=: \n\t"
		//wait for previous spm to complete 0x37=SPMCR
			"wait_spm%=: \n\t"
			"lds r18,%0 \n\t"
			"sbrc r18,0 \n\t"
			"rjmp wait_spm%= \n\t"
		//disable interrups and store status 0X3F=SREG	
		    "in r19,0x3F \n\t"
			"cli \n\t"
		//wait for eeprom write	0x1C=EECR  0X3F=SREG  0x37=SPMCR
			"wait_eeprom%=: \n\t"
			"sbic 0x1C,1 \n\t"
			"rjmp wait_eeprom%= \n\t"
			"sts %0,r16 \n\t"
			"spm \n\t"
			"nop \n\t" ////////Just for the sake of debugging////////
			"nop \n\t" // well i didn't know what he hell          //
			"out 0x3F,r19 \n\t" // was going wrong after spm                //
			"nop \n\t" // so i replaced the SREG restoring with nop//
			"ret \n\t" //////////////////////////////////////////////
			"write_done%=: \n\t"
			"clr __zero_reg__ \n\t"
			:"=m"(SPMCR)::"r0","r16","r17","r19","r24","r25","r28","r29","r30","r31");
///////////////////////////////////////////////////////////////////////////////////////////
			usart_write('m'); 
			ch=usart_read();
			if(ch=='y'){ // yes if more data
				usart_write('o');
				Page_no++;
				goto get_the_page;
			}
			else if(ch=='a'){ // jump to application
				usart_write('o');
				application();

			}
			else{  
				usart_write('e'); //nack
			}
			
		}
		else{
			usart_write('e');
		}
	}
	else
	{
		usart_write('e');
	}
		
/////////////This is just to know that nack was send//////////	
	while(1){
		PORTD|=(1<<PD7);
		_delay_ms(500);
		PORTD&=~(1<<PD7);
	    _delay_ms(500);
	}
	return 1;
}
void application(void){
	while(1){
		PORTB|=(1<<PB2);
		_delay_ms(500);
		PORTB&=~(1<<PB2);
		_delay_ms(500);
	}
}
void afterReset(void){
	DDRB&=~(1<<PB0);
	DDRD|=(1<<PD7);
	DDRB|=(1<<PB2);
	if(PINB&(1<<PB0))
	{
		asm("rjmp 0x0040");
	}
	else
	{
		asm("rjmp 0x1800");	
	}
}
void eeprom_write(unsigned int address,unsigned char data){
	while((EECR&(1<<EEWE))||SPMCR&(1<<SPMEN));
	EEARL=address;
	EEDR=data;
	cli();
	EECR|=(1<<EEMWE);
	EECR|=(1<<EEWE);	
    while((EECR)&(1<<EEWE));	
    sei(); 
}
void usart_init(void){
UBRRH= (((F_CPU/BAUD_RATE)/16)-1)>>8;
UBRRL= (((F_CPU/BAUD_RATE)/16)-1);
UCSRC|=(1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0);
UCSRB|=(1<<RXEN)|(1<<TXEN);

}
unsigned char usart_read(){
while(!(UCSRA&(1<<RXC)));
return UDR;
}
void usart_write(unsigned char data){
while(!(UCSRA&(1<<UDRE)));
UDR=data;
while(!(UCSRA&(1<<UDRE)));
}