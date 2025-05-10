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
//起始信号为SCL是高电平时，SDA由高转低
void I2C_Start(void)
{
	I2C_W_SDA (1);
	I2C_W_SCL (1);
	I2C_W_SDA (0);
	I2C_W_SCL (0);
}
//停止信号为SCL是高电平时，SDA由低转高
void I2C_Stop(void)
{
	I2C_W_SDA (0);
	I2C_W_SCL (1);
	I2C_W_SDA (1);
}
//收到起始信号时，SCL拉低表示开始发送，此时就可以通过SDA的高低电平来读取数据
//每次发送完时，都要重置一次SCL，以便发送下一位数据
void I2C_SendByte(uint8_t Byte)
{
	I2C_W_SCL (0);
	for(int i=0;i<8;i++)
	{
	I2C_W_SDA (Byte&(0x80>>i));
	I2C_W_SCL (1);
	I2C_W_SCL (0);
	}
	I2C_W_SCL (1);//额外时钟信号，表示不处理应答信号
	I2C_W_SCL (0);
}
//接收函数，通过SDA的电平状态判断当前信号，组合成一串二进制数据传给主机
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
//写命令函数
void OLED_WriteCommand(uint8_t Cmd)
{
	I2C_Start ();
	I2C_SendByte(0x78);//寻址
	I2C_SendByte (0x00);//告诉屏幕，我发的是命令，不是数据
	I2C_SendByte (Cmd);//指定的命令
	I2C_Stop ();
}
//写数据函数
void OLED_WriteData(uint8_t Data)
{
	I2C_Start ();
	I2C_SendByte(OLED_ADRESS);//寻址
	I2C_SendByte (0x40);//告诉屏幕，我发的是数据，不是命令
	I2C_SendByte (Data);//指定的命令
	I2C_Stop ();
}
//设置光标函数
void OLED_SetCursor(uint8_t Y,uint8_t X)
{
	OLED_WriteCommand(0xB0 | Y);					//设置Y位置
	OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));	//设置X位置高4位
	OLED_WriteCommand(0x00 | (X & 0x0F));			//设置X位置低4位
}
//画点函数
void OLED_DrawUnit(uint8_t x,uint8_t y)
{
	uint8_t page,line;
	
	page = y/8;//计算页
	line = y%8;//计算列
	
	OLED_Buffer[page][x]|= 0x01<<(line);	
}
//清屏
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
//刷新显存
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
    uint8_t page_start = BY / 8;  // 计算起始页
    uint8_t page_end = EY / 8;    // 计算结束页
    uint8_t page;
    for (page = page_start; page <= page_end; page++)
    {
        OLED_SetCursor(page, BX);   // 设置光标到当前页的起始列
        for (i = BX; i <= EX; i++)  // 遍历指定的列范围
        {
            OLED_WriteData(0x00);   // 写入清屏数据（关闭像素）
            OLED_Buffer[page][i] = 0x00; // 更新显存缓冲区对应位置的数据
        }
    }
	OLED_UpDate() ;
}
void OLED_ShowChar(uint8_t X, uint8_t Y, char Char)
{
    uint8_t i;
    uint8_t page = Y / 8;  // 计算字符所在的页
    uint8_t col = X;       // 字符的起始列

    // 写入上半部分字符数据到显存缓冲区
    for (i = 0; i < 8; i++)
    {
        OLED_Buffer[page][col + i] = OLED_F8x16[Char - ' '][i];
    }

    // 写入下半部分字符数据到显存缓冲区
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

    // 处理整数部分
    if (intPart == 0)
    {
        OLED_ShowChar(X, Y++, '0');
    }
    else
    {
        // 存储整数部分的每一位数字对应的字符
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

    // 显示小数点
    OLED_ShowChar(X, Y++, '.');

    // 处理小数部分
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
		/*遍历左右Y坐标，画矩形左右两条线*/
		for (i = BY; i < BY + Height; i ++)
		{
			OLED_DrawUnit(i,BY);
			OLED_DrawUnit(i,BY+Width);
		}
}

//OLED初始化
void OLED_Init(void)
{
	uint32_t i, j;
	
	for (i = 0; i < 1000; i++)			//上电延时
	{
		for (j = 0; j < 1000; j++);
	}
	
	I2C_GPIO_Init();			//端口初始化
	
	OLED_WriteCommand(0xAE);	//关闭显示
	
	OLED_WriteCommand(0xD5);	//设置显示时钟分频比/振荡器频率
	OLED_WriteCommand(0x80);
	
	OLED_WriteCommand(0xA8);	//设置多路复用率
	OLED_WriteCommand(0x3F);
	
	OLED_WriteCommand(0xD3);	//设置显示偏移
	OLED_WriteCommand(0x00);
	
	OLED_WriteCommand(0x40);	//设置显示开始行
	
	OLED_WriteCommand(0xA1);	//设置左右方向，0xA1正常 0xA0左右反置
	
	OLED_WriteCommand(0xC8);	//设置上下方向，0xC8正常 0xC0上下反置

	OLED_WriteCommand(0xDA);	//设置COM引脚硬件配置
	OLED_WriteCommand(0x12);
	
	OLED_WriteCommand(0x81);	//设置对比度控制
	OLED_WriteCommand(0xCF);

	OLED_WriteCommand(0xD9);	//设置预充电周期
	OLED_WriteCommand(0xF1);

	OLED_WriteCommand(0xDB);	//设置VCOMH取消选择级别
	OLED_WriteCommand(0x30);

	OLED_WriteCommand(0xA4);	//设置整个显示打开/关闭

	OLED_WriteCommand(0xA6);	//设置正常/倒转显示

	OLED_WriteCommand(0x8D);	//设置充电泵
	OLED_WriteCommand(0x14);

	OLED_WriteCommand(0xAF);	//开启显示
	
	OLED_Clear();
		
}
