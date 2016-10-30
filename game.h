/*
	Note:

	GAME_INTERNAL :
	0 - Build for public release
	1 - Build for developer only

	GAME_SLOW :
	0 - No slow code allowed!
	1 - Slow code welcome						
*/

#if GAME_SLOW
#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#else
#define Assert(Expression)
#endif


#define Kilobytes(Value) ((Value) * 1024LL)
#define Megabytes(Value) ((Kilobytes(Value)) * 1024LL)
#define Gigabytes(Value) ((Megabytes(Value)) * 1024LL)
#define Terabytes(Value) ((Gigabytes(Value)) * 1024LL)

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

inline uint32
SafeTruncateUInt64(uint64 Value)
{
	Assert(Value <= 0xFFFFFFFF);
	uint32 Result = (uint32)Value;
	return Result;
}

//Services that the game provides to the platform layer
struct game_offscreen_buffer
{
	void* Memory;
	int BytesPerPixel;
	int Width;
	int Height;
	int Pitch;
};

struct game_sound_output_buffer
{
	int16 *Samples;
	int SamplesPerSecond;
	int SampleCount;
};

struct game_button_state
{
	bool32 IsDown;
};


struct game_keyboard_input
{
	game_button_state Up1;
	game_button_state Down1;
	
	game_button_state Up2;
	game_button_state Down2;
};

struct game_input
{
	game_keyboard_input Keyboard;
};


struct game_memory
{
	bool32 IsInitialised;

	uint64 PermanentStorageSize;
	void *PermanentStorage; //REQUIRED to be cleared to 0

	uint64 TransientStorageSize;
	void *TransientStorage;
};

internal void 
GameUpdateAndRender(game_memory *Memory, game_input *Input, game_offscreen_buffer *Buffer);

struct v2
{
	real32 X;
	real32 Y;
};

struct player
{
	real32 X;
	real32 Y;
	real32 Width;
	real32 Height;

	real32 NextY;
};

struct ball
{
	real32 X;
	real32 Y;
	real32 Width;
	real32 Height;

	real32 NextX;
	real32 NextY;

	v2 Velocity;
};

struct game_state
{
	player Player1;
	player Player2;
	ball Ball;
	bool32 Reset;
};