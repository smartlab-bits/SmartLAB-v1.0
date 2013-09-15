#include<reg51.h>
sbit p32=P3^2;  //output of outer sensor for entry detect
sbit p33=P3^3;  //output of inner sensor for exit detect

sbit p12=P1^2; //to check state of 32
sbit p13=P1^3; //to check state of 33


sbit p36=P3^6; //entry o/p to bbxm
sbit p37=P3^7;  //exit o/p to bbxm

sbit p15=P1^5; //alignement check

void delay(long int m);
long int countDelay=0;
//int c1=0,c2=0;
int count=0;

void main()
{
	P2=0x00;
	
	p15=1;
	p36=1;
	p37=1;
	
	outer : while(1)
	{
		countDelay = 0;
		
		if(p32==1)
		{
			while(p32==1)
			{
				delay(5);
				p15=0;
				delay(5);
				p15=1;
			}
			
			while(p33==0)
			{
				delay(1);
				if(countDelay>30000) goto outer;
			}
			while(p33==1)
			{ delay(5);
				p15=0;
				delay(5);
				p15=1;
			}
			//while(p31==1)
			count++;
			p36=~p36;
			countDelay++;
			delay(500);
			
		}
		else if(p33==1)
		{
			
			while(p33==1)
			{ delay(5);
				p15=0;
				delay(5);
				p15=1;
			}
			while(p32==0)
			{
				delay(1);
				if(countDelay>30000) goto outer;
			}
			
			while(p32==1)
			{ delay(5);
				p15=0;
				delay(5);
				p15=1;
			}
			//while(p31==1)
			
			count--;
			p37=~p37;
			countDelay++;\
			delay(500);
		}
		
		P2=count;
		
		if(p32==1 || p33==1)	p15=1;
		else	p15=0;
		
		p12 = p32;
		p13 = p33;
}
 	
}
 void delay(long int m)
 {
    long i;
    int j;
    for(i=0;i<m;i++)
    for(j=0;j<100;j++)
		{
				j++;
				j--;
		}
 }
