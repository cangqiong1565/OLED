#ifndef __OLED_H
#define __OLED_H
void OLED_DrawUnit(uint8_t x, uint8_t y);
void OLED_Clear(void);
void OLED_UpDate(void);
void OLED_ClearArea(uint8_t BX, uint8_t BY, uint8_t EX, uint8_t EY);
void OLED_ShowChar(uint8_t X, uint8_t Y, char Char);
void OLED_ShowString(uint8_t X, uint8_t Y, char *Str);
void OLED_ShowNum(uint8_t X, uint8_t Y, uint8_t Num);
void OLED_ShowfloatNum(uint8_t X, uint8_t Y, float Num, uint8_t length);
void OLED_DrawRectangle(uint8_t BX, uint8_t BY, uint8_t Width, uint8_t Height);
void OLED_Init(void);
#endif