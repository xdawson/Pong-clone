#define global static
#define internal static
#define local_persist static

#define PI32 3.14159265359f

#include <stdint.h>

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float real32;
typedef double real64;

global bool GlobalRunning;

#include <math.h>

#include <windows.h>

#include "game.h"
#include "game.cpp"

#include <dsound.h>
#include <xinput.h>
#include <stdio.h>

#include "win32_game.h"

global win32_offscreen_buffer GlobalBuffer;

internal win32_window_dimension
Win32GetWindowDimension(HWND Window)
{
	win32_window_dimension Result;

	RECT ClientRect;
	GetClientRect(Window, &ClientRect);

	Result.Width = ClientRect.right - ClientRect.left;
	Result.Height = ClientRect.bottom - ClientRect.top;

	return Result;
}

internal void 
Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height)
{
	if (Buffer->Memory)
	{
		VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
	}
	
	Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
	Buffer->Info.bmiHeader.biWidth = Width;
	Buffer->Info.bmiHeader.biHeight = -Height;
	Buffer->Info.bmiHeader.biPlanes = 1;
	Buffer->Info.bmiHeader.biBitCount = 32;
	Buffer->Info.bmiHeader.biCompression = BI_RGB;

	Buffer->Width = Width;
	Buffer->Height = Height;
	Buffer->BytesPerPixel = 4;
	Buffer->Pitch = Width * Buffer->BytesPerPixel;
	int BufferSize = (Width*Height) * Buffer->BytesPerPixel;

	Buffer->Memory = VirtualAlloc(0, BufferSize, MEM_COMMIT, PAGE_READWRITE);
}

internal void
Win32DisplayBufferInWindow(win32_offscreen_buffer Buffer, win32_window_dimension Dimension, HDC DeviceContext)
{
	StretchDIBits(
		DeviceContext,
		0, 0, Dimension.Width, Dimension.Height,
		0, 0, Buffer.Width, Buffer.Height,
		Buffer.Memory,
		&Buffer.Info,
		DIB_RGB_COLORS,
		SRCCOPY
		);
}

internal LRESULT CALLBACK 
WindowProc(
	HWND   Window,
	UINT   Message,
	WPARAM WParam,
	LPARAM LParam
	)
{
	LRESULT Result = 0;
	
	switch (Message) 
	{
		case WM_DESTROY:
		{
			//could be an error? maybe try recreating the window?
			GlobalRunning = false;
		} break;

		case WM_CLOSE:
		{
			//a user has tried to close the window, could post "are you sure" message to user
			GlobalRunning = false;
		} break;

		case WM_ACTIVATEAPP:
		{
			OutputDebugStringA("WM_ACTIVATEAPP\n");
		} break;

		case WM_PAINT:
		{
			PAINTSTRUCT Paint;
			HDC DeviceContext = BeginPaint(Window, &Paint);
			
			int X = Paint.rcPaint.left;
			int Y = Paint.rcPaint.top;
			int Width = Paint.rcPaint.right - Paint.rcPaint.left;
			int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;

			win32_window_dimension Dimension = Win32GetWindowDimension(Window);

			Win32DisplayBufferInWindow(GlobalBuffer, Dimension, DeviceContext);
			
			EndPaint(Window, &Paint);
		} break;

		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_KEYUP:
		{
			Assert(!"Keyboard input came in through a non-dispatch message!")
		}

		default:
		{
			Result = DefWindowProcA(Window, Message, WParam, LParam);
		} break;
	}

	return(Result);
}

internal void
Win32ProcessKeyboardDigitalButton(game_button_state *State, bool32 IsDown)
{
	State->IsDown = IsDown;
}
internal void
Win32ProcessPendingMessages(HWND Window, game_keyboard_input *Keyboard)
{
	MSG Message;
	while(PeekMessageA(&Message, Window, 0, 0, PM_REMOVE))
	{
		if (Message.message == WM_QUIT)
		{
			GlobalRunning = false;
		}

		switch(Message.message)
		{
			case WM_QUIT:
			{
				GlobalRunning = false;
			} break;

			case WM_SYSKEYDOWN:
			case WM_SYSKEYUP:
			case WM_KEYDOWN:
			case WM_KEYUP:
			{
				uint32 VKCode = (uint32)Message.wParam;
				bool32 WasDown = ((Message.lParam & 1 << 30) != 0);
				bool32 IsDown = ((Message.lParam & 1 << 31) == 0);
		
				if(WasDown != IsDown)
				{
					if(VKCode == 'W')
					{
						Win32ProcessKeyboardDigitalButton(&Keyboard->Up1, IsDown);
					}
					
					else if(VKCode == 'S')
					{
						Win32ProcessKeyboardDigitalButton(&Keyboard->Down1, IsDown);
					}

					else if(VKCode == VK_UP)
					{
						Win32ProcessKeyboardDigitalButton(&Keyboard->Up2, IsDown);
					}

					else if(VKCode == VK_DOWN)
					{
						Win32ProcessKeyboardDigitalButton(&Keyboard->Down2, IsDown);
					}
				}

				if(VKCode == VK_ESCAPE)
				{
					GlobalRunning = false;
				}
			} break;

			default:
			{
				TranslateMessage(&Message);
				DispatchMessageA(&Message);
			}
		}
	}
}

int CALLBACK 
WinMain(HINSTANCE Instance,
		HINSTANCE PrevInstance,
		LPSTR     lpCmdLine,
		int       nCmdShow) 
{

	WNDCLASSA WindowClass = {};
	
	Win32ResizeDIBSection(&GlobalBuffer, 1280, 720);
	
	WindowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
	WindowClass.lpfnWndProc = WindowProc;
	WindowClass.hInstance = Instance;
	//WindowClass.hIcon = ;
	WindowClass.lpszClassName = "My Window Class";
 
	if (RegisterClassA(&WindowClass)) {

		HWND Window = CreateWindowExA(
			0,
			WindowClass.lpszClassName,
			"Game",
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			0,
			0,
			Instance,
			0);

		if (Window) 
		{
			//Since I specified CS_OWNDC I can just get one device context and use it forever
			//because I'm not sharing it with anyone
			//no release required
			HDC DeviceContext = GetDC(Window);

		
			GlobalRunning = true;

			//Sould I do the thing like in handmade hero where he uses a different base address for VirtualAlloc
			//depending of whether GAME_INTERNAL is 1?
			game_memory GameMemory = {};
			GameMemory.PermanentStorageSize = Megabytes(64);
			GameMemory.PermanentStorage = VirtualAlloc(0, GameMemory.PermanentStorageSize, 
										  MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

			GameMemory.TransientStorageSize = Gigabytes(4);
			GameMemory.TransientStorage = VirtualAlloc(0, GameMemory.TransientStorageSize, 
										  MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

			if(GameMemory.PermanentStorage)
			{

				game_input Input = {};


				while(GlobalRunning) 
				{
					Win32ProcessPendingMessages(Window, &Input.Keyboard);	
					
					//Graphics Code
					game_offscreen_buffer Buffer = {};
					Buffer.Memory = GlobalBuffer.Memory;
					Buffer.BytesPerPixel = GlobalBuffer.BytesPerPixel;
					Buffer.Width = GlobalBuffer.Width;
					Buffer.Height = GlobalBuffer.Height;
					Buffer.Pitch = GlobalBuffer.Pitch;
					
					GameUpdateAndRender(&GameMemory ,&Input, &Buffer);

					win32_window_dimension Dimension = Win32GetWindowDimension(Window);
					Win32DisplayBufferInWindow(GlobalBuffer, Dimension, DeviceContext);
				}
			}
		}
	}
	return 0;
}