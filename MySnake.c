
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiRuntimeLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/SimplePointer.h>
#include <Protocol/AbsolutePointer.h>
#include <Protocol/SimpleTextInEx.h>
#define height 17
#define width 28
#define JR_RANDOM_NUM   100
static UINT32 RandomPool[JR_RANDOM_NUM];
static UINT32 JR_index = JR_RANDOM_NUM;
static UINT32 RandomResult = 0;
UINT32 IsFailed = 0;
UINT32 ispaused = 0;
typedef struct _SNAKEDIRECTION{
    INT32 x;
    INT32 y;
}SNAKEDIRECTION;
SNAKEDIRECTION direction;
typedef enum _BOARDSTATUS{
    snake, food, space
}BOARDSTATUS;
BOARDSTATUS board[height][width];
typedef struct _NODE{
    UINT32 x;
    UINT32 y;
}NODE;
NODE myfood ;
UINT32 SnakeLen = 3;
UINT32 score = 0;
NODE MySnake[height*width];
VOID Init();
VOID CreateSnake();
VOID CreateFood();
VOID PushSnakeFront(NODE pos);
NODE RemoveSnakeBack();
UINT32 isoverstep(NODE pos);
VOID gameover(EFI_EVENT Event);
UINT32 isSpace(NODE pos);
UINT32 isSnake(NODE pos);
UINT32 isFood(NODE pos);
static VOID JR_InitRandom();
UINT32 JR_randomInt(UINT32 max);
VOID Run(EFI_EVENT Event,VOID *Context);
EFI_STATUS SetTimer();

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
    //EFI_STATUS Status;
    //EFI_SIMPLE_TEXT_OUTPUT_MODE SavedConsoleMode;
    //CopyMem(&SavedConsoleMode, gST->ConOut->Mode, sizeof(EFI_SIMPLE_TEXT_OUTPUT_MODE));
    /*
    Status = gST->ConOut->EnableCursor(gST->ConOut, FALSE);
    Status = gST->ConOut->ClearScreen(gST->ConOut);
    Status = gST->ConOut->SetAttribute(gST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK));
    Status = gST->ConOut->SetCursorPosition(gST->ConOut, 0, 0);
    */
    SetTimer();
    /*
    gST->ConOut->EnableCursor(gST->ConOut, SavedConsoleMode.CursorVisible);
    gST->ConOut->SetCursorPosition(gST->ConOut, SavedConsoleMode.CursorColumn, SavedConsoleMode.CursorRow);
    gST->ConOut->SetAttribute(gST->ConOut, SavedConsoleMode.Attribute);
    gST->ConOut->ClearScreen(gST->ConOut);
    */
    return 0;
}


VOID Run(EFI_EVENT Event,VOID *Context)
{
    INT32 headerX, headerY;
    headerX = MySnake[0].x + direction.x;
    headerY = MySnake[0].y + direction.y;
    NODE snakeHeader;
    snakeHeader.x=headerX;
    snakeHeader.y=headerY;
    if(isoverstep(snakeHeader)||isSnake(snakeHeader))
        gameover(Event);
    if(isFood(snakeHeader))
    {
        score ++;
        CreateFood();
    }
    else
    {
        NODE snakeFooter;
        snakeFooter = RemoveSnakeBack();
    }
    PushSnakeFront(snakeHeader);
    UINT32 i;
    for(i=0;i<SnakeLen;i++)
    {
        Print(L"[%d,%d] ",MySnake[i].x,MySnake[i].y);
    }
    Print(L"  Direction:%d,%d    ",direction.x,direction.y);
    Print(L"Food:[%d,%d]\n",myfood.x,myfood.y);
}
EFI_STATUS SetTimer()
{
    Init();
    CreateSnake();
    CreateFood();
	EFI_STATUS  Status;
	EFI_EVENT myEvent;
	Print(L"Begin\n");
	Status = gBS->CreateEvent(EVT_TIMER | EVT_NOTIFY_SIGNAL, TPL_CALLBACK, (EFI_EVENT_NOTIFY)Run, (VOID*)L"Hello! Time Out!", &myEvent);
	Status = gBS->SetTimer(myEvent, TimerPeriodic, 2 * 1000 * 1000);
    while (1)
    {
        if(IsFailed)
        {
            Print(L"GameOver\n");
            break;
        }
        EFI_INPUT_KEY Key;
        Status = gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);
        if(Key.ScanCode == SCAN_ESC)
            break;
        if(!ispaused)
        {
            if(Key.ScanCode == SCAN_UP&&direction.x!=1  )
            {
                direction.x=-1;
                direction.y=0;
            }
            else if(Key.ScanCode == SCAN_RIGHT&&direction.y!=-1   )
            {
                direction.x=0;
                direction.y=1;
            }
            else if(Key.ScanCode == SCAN_DOWN&&direction.x!=-1   )
            {
                direction.x=1;
                direction.y=0;
            }
            else if(Key.ScanCode == SCAN_LEFT&&direction.y!=1   )
            {
                direction.x=0;
                direction.y=-1;
            }
        }
        if(Key.UnicodeChar == ' ')
        {
            if(ispaused)
            {
                Status = gBS->CreateEvent(EVT_TIMER | EVT_NOTIFY_SIGNAL, TPL_CALLBACK, (EFI_EVENT_NOTIFY)Run, (VOID*)L"Hello! Time Out!", &myEvent);
	            Status = gBS->SetTimer(myEvent, TimerPeriodic, 2 * 1000 * 1000);
                ispaused=0;
            }
            else
            {
                gBS->CloseEvent(myEvent);
                ispaused=1;
            }
        }
    }
	Status = gBS->CloseEvent(myEvent);
	return EFI_SUCCESS;
}
VOID Init()
{
    direction.x = 0;
    direction.y = 1;
    UINT32 i, j ;
    for(i=0;i<height;i++)
        for(j=0;j<width;j++)
        {
            board[i][j] = space;
        }
}
VOID CreateSnake()
{
    MySnake[0].x = 8;
    MySnake[0].y = 15;
    MySnake[1].x = 8;
    MySnake[1].y = 14;
    MySnake[2].x = 8;
    MySnake[2].y = 13;
    UINT32 i;
    for(i=0;i<SnakeLen;i++)
        board[MySnake[i].x][MySnake[i].y]=snake;
}
VOID CreateFood()
{
    UINT32 randomX, randomY;
    randomX = JR_randomInt(height);
    randomY = JR_randomInt(width);
    if(board[randomX][randomY] != space)
        CreateFood();
    else
    {
        Print(L"Food:x=%d y=%d\n",randomX,randomY);
        myfood.x=randomX;
        myfood.y=randomY;
        board[randomX][randomY] = food;
    }
}
UINT32 isoverstep(NODE pos)
{
    if(pos.x<0||pos.x>16||pos.y<0||pos.y>27)
		return 1;			
	else
		return 0;
}
static VOID JR_InitRandom(){
    UINT32 i = 0;
    EFI_TIME  Time;
    if(JR_index != JR_RANDOM_NUM) return ;
    JR_index = 0;
    for(i=0; i<JR_RANDOM_NUM; i++){
        if(RandomResult ==0)
        {
            gRT->GetTime(&Time, NULL);
            RandomResult  = Time.Second;
        }
        RandomResult = (RandomResult<<1) | (((RandomResult&0x80)>>7)^((RandomResult&0x40)>>6));
        RandomPool[i] = RandomResult;
    }
}
UINT32 JR_randomInt(UINT32 max){
    JR_InitRandom();
    return (RandomPool[JR_index++] % (max));
}
VOID gameover(EFI_EVENT Event)
{
    EFI_STATUS Status;
    Status = gBS->CloseEvent(Event);
    IsFailed=1;
}
UINT32 isSpace(NODE pos)
{
    if(board[pos.x][pos.y] == space )
        return 1;
    else
        return 0;
}
UINT32 isSnake(NODE pos)
{
    if(board[pos.x][pos.y] == snake )
        return 1;
    else
        return 0;
}
UINT32 isFood(NODE pos)
{
    if(board[pos.x][pos.y] == food )
        return 1;
    else
        return 0;
}
VOID PushSnakeFront(NODE pos)
{
    UINT32 i;
    for(i=SnakeLen;i>0;i--)
    {
        MySnake[i]=MySnake[i-1];
    }
    board[pos.x][pos.y]=snake;
    MySnake[0]=pos;
    SnakeLen++ ;
}
NODE RemoveSnakeBack()
{
    SnakeLen-- ;
    board[MySnake[SnakeLen].x][MySnake[SnakeLen].y]=space;
    return MySnake[SnakeLen];
}