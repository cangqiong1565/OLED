#include "main.h"
#include "OLED_Font.h"
#include <stdio.h>
#include "stdbool.h"

#define I2C_W_SCL(x)         HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,(GPIO_PinState)(x))
#define I2C_W_SDA(x)         HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,(GPIO_PinState)(x))
#define OLED_ADRESS          0x78 

#define OLED_WIDTH 128
#define OLED_HEIGHT 64

uint8_t OLED_Buffer[8][128];


void I2C_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	I2C_W_SCL (1);
	I2C_W_SDA (1);
}
//��ʼ�ź�ΪSCL�Ǹߵ�ƽʱ��SDA�ɸ�ת��
void I2C_Start(void)
{
	I2C_W_SDA (1);
	I2C_W_SCL (1);
	I2C_W_SDA (0);
	I2C_W_SCL (0);
}
//ֹͣ�ź�ΪSCL�Ǹߵ�ƽʱ��SDA�ɵ�ת��
void I2C_Stop(void)
{
	I2C_W_SDA (0);
	I2C_W_SCL (1);
	I2C_W_SDA (1);
}
//�յ���ʼ�ź�ʱ��SCL���ͱ�ʾ��ʼ���ͣ���ʱ�Ϳ���ͨ��SDA�ĸߵ͵�ƽ����ȡ����
//ÿ�η�����ʱ����Ҫ����һ��SCL���Ա㷢����һλ����
void I2C_SendByte(uint8_t Byte)
{
	I2C_W_SCL (0);
	for(int i=0;i<8;i++)
	{
	I2C_W_SDA (Byte&(0x80>>i));
	I2C_W_SCL (1);
	I2C_W_SCL (0);
	}
	I2C_W_SCL (1);//����ʱ���źţ���ʾ������Ӧ���ź�
	I2C_W_SCL (0);
}
//���պ�����ͨ��SDA�ĵ�ƽ״̬�жϵ�ǰ�źţ���ϳ�һ�����������ݴ�������
uint8_t I2C_ReceiveByte(void)
{
	uint8_t Byte=0x00;
	I2C_W_SDA (1);
	for(int i=0;i<8;i++)
	{
	I2C_W_SCL (1);
	if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_13)==1){Byte|=0x80>>i;}
	I2C_W_SCL (0);
	}
	return Byte;
}
//д�����
void OLED_WriteCommand(uint8_t Cmd)
{
	I2C_Start ();
	I2C_SendByte(0x78);//Ѱַ
	I2C_SendByte (0x00);//������Ļ���ҷ����������������
	I2C_SendByte (Cmd);//ָ��������
	I2C_Stop ();
}
//д���ݺ���
void OLED_WriteData(uint8_t Data)
{
	I2C_Start ();
	I2C_SendByte(OLED_ADRESS);//Ѱַ
	I2C_SendByte (0x40);//������Ļ���ҷ��������ݣ���������
	I2C_SendByte (Data);//ָ��������
	I2C_Stop ();
}
//���ù�꺯��
void OLED_SetCursor(uint8_t Y,uint8_t X)
{
	OLED_WriteCommand(0xB0 | Y);					//����Yλ��
	OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));	//����Xλ�ø�4λ
	OLED_WriteCommand(0x00 | (X & 0x0F));			//����Xλ�õ�4λ
}
//���㺯��
void OLED_DrawUnit(uint8_t x,uint8_t y)
{
	uint8_t page,line;
	
	page = y/8;//����ҳ
	line = y%8;//������
	
	OLED_Buffer[page][x]|= 0x01<<(line);	
}
//����
void OLED_Clear(void)
{  
	uint8_t i, j;
	for (j = 0; j < 8; j++)
	{
		OLED_SetCursor(j, 0);
		for(i = 0; i < 128; i++)
		{
			OLED_WriteData(0x00);
		}
	}
}
//ˢ���Դ�
void OLED_UpDate(void)
{
	int i,j;
	for(i=0;i<8;i++)
	{
		OLED_SetCursor(i,0);
	for(j=0;j<128;j++)
		{
		OLED_WriteData(OLED_Buffer [i][j]);   
		}
	
	}
}

void OLED_ClearArea(uint8_t BX, uint8_t BY, uint8_t EX, uint8_t EY)
{
    uint8_t i, j;
    uint8_t page_start = BY / 8;  // ������ʼҳ
    uint8_t page_end = EY / 8;    // �������ҳ
    uint8_t page;
    for (page = page_start; page <= page_end; page++)
    {
        OLED_SetCursor(page, BX);   // ���ù�굽��ǰҳ����ʼ��
        for (i = BX; i <= EX; i++)  // ����ָ�����з�Χ
        {
            OLED_WriteData(0x00);   // д���������ݣ��ر����أ�
            OLED_Buffer[page][i] = 0x00; // �����Դ滺������Ӧλ�õ�����
        }
    }
	OLED_UpDate() ;
}
void OLED_ShowChar(uint8_t X, uint8_t Y, char Char)
{
    uint8_t i;
    uint8_t page = Y / 8;  // �����ַ����ڵ�ҳ
    uint8_t col = X;       // �ַ�����ʼ��

    // д���ϰ벿���ַ����ݵ��Դ滺����
    for (i = 0; i < 8; i++)
    {
        OLED_Buffer[page][col + i] = OLED_F8x16[Char - ' '][i];
    }

    // д���°벿���ַ����ݵ��Դ滺����
    page++;
    for (i = 0; i < 8; i++)
    {
        OLED_Buffer[page][col + i] = OLED_F8x16[Char - ' '][i + 8];
    }
}
	
void OLED_ShowString(uint8_t X,uint8_t Y,char *Str)
{
	bool Flag=true;
	while(*Str!='\0')
	{
		OLED_ShowChar(X,Y,*Str);
		X+=8;
		Str++;
		if(X>=128&&Flag==true)
		{
			X=0;
			Y+=16;
			Flag=false;
		}
	}
}

void OLED_ShowNum(uint8_t X,uint8_t Y,uint8_t Num)
{
	char Str[100];
	sprintf(Str,"%d",Num);
	OLED_ShowString(X,Y,Str);
}
void OLED_ShowfloatNum(uint8_t X,uint8_t Y,float Num,uint8_t length)
{
    int intPart = (int)Num;
    float decPart = Num - intPart;

    // ������������
    if (intPart == 0)
    {
        OLED_ShowChar(X, Y++, '0');
    }
    else
    {
        // �洢�������ֵ�ÿһλ���ֶ�Ӧ���ַ�
        char intStr[10];
        int i = 0;
        while (intPart > 0)
        {
            intStr[i++] = intPart % 10 + '0';
            intPart /= 10;
        }
        for (int j = i - 1; j >= 0; j--)
        {
            OLED_ShowChar(X, Y++, intStr[j]);
        }
    }

    // ��ʾС����
    OLED_ShowChar(X, Y++, '.');

    // ����С������
    for (uint8_t i = 0; i < length; i++)
    {
        decPart *= 10;
        int digit = (int)decPart;
        OLED_ShowChar(X, Y++, digit + '0');
        decPart -= digit;
    }
}

void OLED_DrawRectangle(uint8_t BX, uint8_t BY, uint8_t Width, uint8_t Height)
{
		uint8_t i, j;
		for (i = BX; i < BX + Width; i ++)
		{
			OLED_DrawUnit(BX,i);
			OLED_DrawUnit(BX+Height,i);
		}
		/*��������Y���꣬����������������*/
		for (i = BY; i < BY + Height; i ++)
		{
			OLED_DrawUnit(i,BY);
			OLED_DrawUnit(i,BY+Width);
		}
}

//OLED��ʼ��
void OLED_Init(void)
{
	uint32_t i, j;
	
	for (i = 0; i < 1000; i++)			//�ϵ���ʱ
	{
		for (j = 0; j < 1000; j++);
	}
	
	I2C_GPIO_Init();			//�˿ڳ�ʼ��
	
	OLED_WriteCommand(0xAE);	//�ر���ʾ
	
	OLED_WriteCommand(0xD5);	//������ʾʱ�ӷ�Ƶ��/����Ƶ��
	OLED_WriteCommand(0x80);
	
	OLED_WriteCommand(0xA8);	//���ö�·������
	OLED_WriteCommand(0x3F);
	
	OLED_WriteCommand(0xD3);	//������ʾƫ��
	OLED_WriteCommand(0x00);
	
	OLED_WriteCommand(0x40);	//������ʾ��ʼ��
	
	OLED_WriteCommand(0xA1);	//�������ҷ���0xA1���� 0xA0���ҷ���
	
	OLED_WriteCommand(0xC8);	//�������·���0xC8���� 0xC0���·���

	OLED_WriteCommand(0xDA);	//����COM����Ӳ������
	OLED_WriteCommand(0x12);
	
	OLED_WriteCommand(0x81);	//���öԱȶȿ���
	OLED_WriteCommand(0xCF);

	OLED_WriteCommand(0xD9);	//����Ԥ�������
	OLED_WriteCommand(0xF1);

	OLED_WriteCommand(0xDB);	//����VCOMHȡ��ѡ�񼶱�
	OLED_WriteCommand(0x30);

	OLED_WriteCommand(0xA4);	//����������ʾ��/�ر�

	OLED_WriteCommand(0xA6);	//��������/��ת��ʾ

	OLED_WriteCommand(0x8D);	//���ó���
	OLED_WriteCommand(0x14);

	OLED_WriteCommand(0xAF);	//������ʾ
	
	OLED_Clear();
		
}
