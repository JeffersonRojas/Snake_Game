/* ---------------------------------------------------------------------------
**
** Snake_Game.c
**
** Copyright 2015 Jefferson Rojas
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
**
** -------------------------------------------------------------------------*/

#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "grlib/grlib.h"
#include "drivers/buttons.h"
#include "drivers/formike128x128x16.h"

#ifdef DEBUG
void
__error__(char *pcFilename, unsigned long ulLine)
{}
#endif

tContext Graphics_Context;
tRectangle Rectangle;
int Orientation = 1;
int Screen_Cache[16][14];
int Snake_Body[384][2];
int Size_Snake = 15;
int Snake_Velocity = 8;
int Collision = 0;

void ARM_Config()
{
  ROM_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_8MHZ);
  Formike128x128x16Init();
  ButtonsInit();
  Formike128x128x16BacklightOn();
  GrContextInit(&Graphics_Context, &g_sFormike128x128x16);
  tRectangle Rect;
  Rect.sXMin = 0;
  Rect.sYMin = 0;
  Rect.sXMax = GrContextDpyWidthGet(&Graphics_Context) - 1;
  Rect.sYMax = 16;
  GrContextForegroundSet(&Graphics_Context, ClrRed);
  GrRectFill(&Graphics_Context, &Rect);
  GrContextForegroundSet(&Graphics_Context, ClrWhite);
  GrContextFontSet(&Graphics_Context, &g_sFontFixed6x8);
  GrStringDrawCentered(&Graphics_Context, "Snake 5.0", -1, GrContextDpyWidthGet(&Graphics_Context) / 2, 7, 0);
  ROM_SysTickPeriodSet(ROM_SysCtlClockGet() / 200);
  ROM_SysTickEnable();
  ROM_SysTickIntEnable();
}

void SysTickIntHandler(void)
{
  unsigned char ucButtons, ucDelta, ucRepeat;
  ucButtons = ButtonsPoll(&ucDelta, &ucRepeat);
  if(BUTTON_PRESSED(UP_BUTTON, ucButtons, ucDelta))
  {
    if(Orientation != 4) Orientation = 3;
  }
  if(BUTTON_PRESSED(DOWN_BUTTON, ucButtons, ucDelta))
  {
    if(Orientation != 3) Orientation = 4;
  }
  if(BUTTON_PRESSED(LEFT_BUTTON, ucButtons, ucDelta))
  {
    if(Orientation != 1) Orientation = 2;
  }
  if(BUTTON_PRESSED(RIGHT_BUTTON, ucButtons, ucDelta))
  {
    if(Orientation != 2) Orientation = 1;
  }
  if(BUTTON_PRESSED(SELECT_BUTTON, ucButtons, ucDelta))
  {
    Size_Snake++;
    Snake_Velocity++;
  }
}

void Update_Screen()
{
  for (int i = 0; i < 16; i++)
  {
    for (int j = 0; j < 14; j++)
    {
      Rectangle.sXMin = (i*8);
      Rectangle.sYMin = (j*8)+16;
      Rectangle.sXMax = (i*8)+8;
      Rectangle.sYMax = (j*8)+24;
      if (Screen_Cache[i][j] == 0)
      {
        GrContextForegroundSet(&Graphics_Context, ClrBlack);
        GrRectFill(&Graphics_Context, &Rectangle);
      }
      if (Screen_Cache[i][j] == 1)
      {
        GrContextForegroundSet(&Graphics_Context, ClrLimeGreen);
        GrRectFill(&Graphics_Context, &Rectangle);
        GrContextForegroundSet(&Graphics_Context, ClrWhite);
        GrRectDraw(&Graphics_Context, &Rectangle);
      }
      if (Screen_Cache[i][j] == 2)
      {
        GrContextForegroundSet(&Graphics_Context, ClrDarkGreen);
        GrRectFill(&Graphics_Context, &Rectangle);
      }
    }
  }
  //GrFlush(&Graphics_Context);
}

void Clean_Screen_Cache()
{
  for (int i = 0; i < 16; i++)
  {
    for (int j = 0; j < 14; j++)
    {
      Screen_Cache[i][j] = 0;
    }
  }
}

void Move_Snake()
{
  for (int i = 0; i < Size_Snake; i++)
  {
    if (i == (Size_Snake-1))
    {
      switch (Orientation)
      {
        case 1:
          Snake_Body[i][0] = Snake_Body[i][0]+1;
          break;
        case 2:
          Snake_Body[i][0] = Snake_Body[i][0]-1;
          break;
        case 3:
          Snake_Body[i][1] = Snake_Body[i][1]-1;
          break;
        case 4:
          Snake_Body[i][1] = Snake_Body[i][1]+1;
          break;
      }
      for (int x = 0; x < Size_Snake-1; x++)
      {
        if(Snake_Body[i][0] == Snake_Body[x][0] && Snake_Body[i][1] == Snake_Body[x][1]) Collision = 1;
      }
      if (Snake_Body[i][0] > 15) Snake_Body[i][0] = 0;
      if (Snake_Body[i][0] < 0)  Snake_Body[i][0] = 15;
      if (Snake_Body[i][1] < 0)  Snake_Body[i][1] = 13;
      if (Snake_Body[i][1] > 13) Snake_Body[i][1] = 0;
      Screen_Cache[Snake_Body[i][0]][Snake_Body[i][1]] = 2;
    }
    else
    {
      Snake_Body[i][0] = Snake_Body[i+1][0];
      Snake_Body[i][1] = Snake_Body[i+1][1];
      Screen_Cache[Snake_Body[i][0]][Snake_Body[i][1]] = 1;
    }
  }
}

void Snake_Game()
{
  Move_Snake();
  int c = 0;
  while (c < SysCtlClockGet()/(100*Snake_Velocity))
  {
    if(Collision)
    {
      c = 1;
    }
    else
    {
      c++;
    }
  }
  //SysCtlDelay(SysCtlClockGet()/8);
  Update_Screen();
  Clean_Screen_Cache();
}

int main(void)
{
  ARM_Config();
  while(1)
  {
    Snake_Game();
  }
}
