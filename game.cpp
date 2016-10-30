internal int
RoundReal32ToInt(real32 Value)
{
	int Result;

	if((Value - (int)Value) >= 0.5)
	{
		Result = (int)Value + 1;
	}
	else 
	{
		Result = (int)Value;
	}
	return Result;
}

internal void
DrawRectangle(game_offscreen_buffer *Buffer, 
			  int MinX, int MinY, int MaxX, int MaxY,
			  int R, int G, int B)
{
	uint8 *Row = (uint8 *)Buffer->Memory + (MinX * Buffer->BytesPerPixel)  + (MinY * Buffer->Pitch);

	//TODO: test if this will go up en entirity of the square of if condition should be <=
	for (int Y = MinY; 
		 Y < MaxY;
		 Y++)
	{
		uint32 *Pixel = (uint32 *)Row;
		
		for (int X = MinX;
			 X < MaxX;
			 X++)
		{	 
			//Layout in memory is XX RR GG BB
			*Pixel++ = (R << 16) | (G << 8)| (B);
		}

		Row += Buffer->Pitch;
	}
}

internal bool32
CheckAABBCollision(real32 X1, real32 Y1, real32 Width1, real32 Height1,
				   real32 X2, real32 Y2, real32 Width2, real32 Height2)
{
	bool32 Collided;

	//AABB Collision
	if((Y1 + Height1) < Y2)
	{
		Collided = false;
	}
	else if(Y1 > (Y2 + Height2))
	{
		Collided = false;
	}
	else if (X1 > (X2 + Width2)) 
	{
		Collided = false;
	}
	else if ((X1 + Width1) < X2)
	{
		Collided = false;
	}
	else 
	{
		Collided = true;
	}

	return Collided;
}

internal void
SetupGame(game_state *GameState)
{
	//setup player 1
	player *Player1 = &GameState->Player1;

	Player1->X = 100;
	Player1->Y = 100;
	
	Player1->Width = 20;
	Player1->Height = 100;

	Player1->NextY = Player1->Y;

	//setup player 2
	player *Player2 = &GameState->Player2;

	Player2->X = 1160;
	Player2->Y = 100;
	
	Player2->Width = 20;
	Player2->Height = 100;

	Player2->NextY = Player2->Y;

	//setup ball
	ball *Ball = &GameState->Ball;

	Ball->X = 400;
	Ball->Y = 400;

	Ball->Width = 10;
	Ball->Height = 10;

	Ball->NextX = Ball->X;
	Ball->NextY = Ball->Y;

	Ball->Velocity.X = 1;
	Ball->Velocity.Y = 1;

	GameState->Reset = false;

}

internal void 
GameUpdateAndRender(game_memory *Memory, game_input *Input, game_offscreen_buffer *Buffer)
{

	game_state *GameState = (game_state *)Memory->PermanentStorage;

	if(!Memory->IsInitialised)
	{
		SetupGame(GameState);

		Memory->IsInitialised = true;
	}

	if(GameState->Reset)
	{
		SetupGame(GameState);
	}

	player *Player1 = &GameState->Player1;
	player *Player2 = &GameState->Player2;
	ball *Ball = &GameState->Ball;
	
	//ball physics
	Ball->NextX += Ball->Velocity.X;
	Ball->NextY -= Ball->Velocity.Y; 


	//deal with input
	if(Input->Keyboard.Up1.IsDown)
	{
		Player1->NextY -= 1;
	}

	if(Input->Keyboard.Down1.IsDown)
	{
		Player1->NextY += 1;
	}

	if(Input->Keyboard.Up2.IsDown)
	{
		Player2->NextY -= 1;
	}

	if(Input->Keyboard.Down2.IsDown)
	{
		Player2->NextY += 1;
	}

	//player collisions
	if(Player1->NextY < 0)
	{
		Player1->NextY = 0;
	}
	else if((Player1->NextY + Player1->Height) > Buffer->Height)
	{
		Player1->NextY = (real32)Buffer->Height - Player1->Height;
	}

	if(Player2->NextY < 0)
	{
		Player2->NextY = 0;
	}
	else if((Player2->NextY + Player2->Height) > Buffer->Height)
	{
		Player2->NextY = (real32)Buffer->Height - Player2->Height;
	}

	//ball collisions with borders
	//Right wall
	if((Ball->NextX + Ball->Width) > Buffer->Width)
	{
		Ball->NextX = Buffer->Width - Ball->Width;
		Ball->Velocity.X *= -1;
	}
	//Left Wall
	else if(Ball->NextX < 0)
	{
		Ball->NextX = 0;

		Ball->Velocity.X *= -1;
	}

	//Bottom
	if((Ball->NextY + Ball->Height) > Buffer->Height)
	{
		Ball->NextY = Buffer->Height - Ball->Height;

		Ball->Velocity.Y *= -1;
	}
	//Top
	else if(Ball->NextY < 0)
	{
		Ball->NextY = 0;

		Ball->Velocity.Y *= -1;
	}

	//ball colisions with player
	if(CheckAABBCollision(Player1->X, Player1->NextY, Player1->Width, Player1->Height,
							Ball->NextX, Ball->NextY, Ball->Width, Ball->Height))
	{
		Ball->Velocity.X *= -1;
	}

	if(CheckAABBCollision(Player2->X, Player2->NextY, Player2->Width, Player2->Height,
							Ball->NextX, Ball->NextY, Ball->Width, Ball->Height))
	{
		Ball->Velocity.X *= -1;
	}

	//Update player position
	Player1->Y = Player1->NextY;
	Player2->Y = Player2->NextY;

	//Update ball position
	Ball->X = Ball->NextX;
	Ball->Y = Ball->NextY;

	//check whether point has been scored
	if(Ball->X < Player1->X)
	{
		GameState->Reset = true;
	}

	if(Ball->X > (Player2->X + Player2->Width))
	{
		GameState->Reset = true;
	}

	//Make background black
	DrawRectangle(Buffer, 0, 0, Buffer->Width, Buffer->Height, 0, 0, 0);

	//Draw players
	DrawRectangle(Buffer, 
				  RoundReal32ToInt(Player1->X), RoundReal32ToInt(Player1->Y), 
				  RoundReal32ToInt((Player1->X + Player1->Width)),
				  RoundReal32ToInt((Player1->Y + Player1->Height)),
				  255, 255, 255);

	DrawRectangle(Buffer, 
			  RoundReal32ToInt(Player2->X), RoundReal32ToInt(Player2->Y), 
			  RoundReal32ToInt((Player2->X + Player2->Width)),
			  RoundReal32ToInt((Player2->Y + Player2->Height)),
			  255, 255, 255);
	
	//Draw ball
	DrawRectangle(Buffer, 
				  RoundReal32ToInt(Ball->X), RoundReal32ToInt(Ball->Y), 
				  RoundReal32ToInt(Ball->X + Ball->Width), RoundReal32ToInt(Ball->Y + Ball->Height), 
				  255, 255, 255);
}	