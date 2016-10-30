struct win32_window_dimension
{
	int Width;
	int Height;
};

struct win32_offscreen_buffer
{
	BITMAPINFO Info;
	void *Memory;
	int BytesPerPixel;
	int Width;
	int Height;
	int Pitch;
};
