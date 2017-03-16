/*
	This program will create a red, green or blue bitmap image file.
	Pass in width, height and color as command line arguments.

	e.g. ./colorbmp 640 480 red

	TODO:

	 - Add the ability to pass in RGB values from the command line.
	 - Name the output file according to its color.

	 for reference: https://jonasjacek.github.io/colors/

*/


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define RED 1
#define GREEN 2
#define BLUE 3

typedef int8_t int8;

struct arguments
{
	int width;
	int height;
	int color;
};

struct TwoByte
{
	int8 a;
	int8 b;
};


struct Pixel
{
	int8 b;
	int8 g;
	int8 r;
};

struct BitmapHeader
{
	int FileSize;
	int Reserved;
	int PixelOffset;
};

struct DIBHeader
{

	int DIBHeaderSize;
	int ImageWidth;
	int ImageHeight;
	int PlanesAndBPP;
	int Compression;
	int ImageSize;
	int Xppm;
	int Yppm;
	int ColorsInColorTable;
	int ColorCount;

};

// Global variables
static int width;
static int height;



// A function to process the command line arguments so we
// don't have to deal with that in the main function.
struct arguments* ProcessArgs(int argc, char* argv[])
{
	struct arguments* args = malloc(sizeof(struct arguments));
	args->width = 0;
	args->height = 0;
	args->color = 0;

	if (argc != 4)
	{
		printf("USAGE: ./colorbmp WIDTH HEIGHT COLOR(\"red\", \"green\", or \"blue\")\n");
		return NULL;
	}

	args->width = atoi(argv[1]);
	args->height = atoi(argv[2]);

	if (args->width < 10 || args->width > 1000)
	{
		printf("Choose width between 10 and 1000.\n");
		return NULL;
	}

	if (args->height < 10 || args->height > 1000)
	{
		printf("Choose height between 10 and 1000.\n");
		return NULL;
	}

	if (argv[3][0] == 'r')
		args->color = RED;
	if (argv[3][0] == 'g')
		args->color = GREEN;
	if (argv[3][0] == 'b')
		args->color = BLUE;

	switch(args->color)
	{
		case RED:
		case GREEN:
		case BLUE:
			break;
		default:
		{
			printf("Just type \"red\", \"green\", or \"blue\" for the color. \n");
			return NULL;
		}
	}


	return args;
};


// Color a single pixel at (x,y) with an RGB value.
void WritePixel(void* Array, int x, int y, int8 r, int8 g, int8 b)
{
	struct Pixel p;
	p.r = r;
	p.g = g;
	p.b = b;

	struct Pixel* ptr = (struct Pixel*)(Array);


	// get ptr to the right place
	ptr += (width*y);
	ptr += x;


	*ptr = p;

}





int main(int argc, char* argv[])
{
	
	struct arguments* argos = ProcessArgs(argc, argv);

	if(!argos)
	{
		// printf("Invalid arguments.\n");
		return -1;
	}

	width = argos->width;
	height = argos->height;
	int color = argos->color;

	free(argos);	

	/*
		Info we need to write into the file:

		-- File Signature --
		1. File signature: 2 bytes
		-- File Signature --

		-- Bitmap File Header --
		1. File size: 4 bytes (one int)
		2. Reserved (zeroes): 2 bytes
		3. Reserved (zeroes): 2 bytes
		4. File offset to PixelArray: 4 bytes
		-- Bitmap File Header --

		-- DIB Header --
		1. DIB Header Size: 4 bytes
		2. Image Width: 4 bytes
		3. Image Height: 4 bytes
		4. Planes (1): 2 bytes
		5. Bits per pixel: 2 bytes
		6. Compression (zeroes): 4 bytes
		7. Image Size: 4 bytes
		8. X pixels per meter: 4 bytes
		9. Y pixels per meter: 4 bytes
		10. Colors in Color Table (zeroes): 4 bytes
		11. Important Color Count (zeroes): 4 bytes
		-- DIB Header --

		-- Pixel Array --
		Pixels
		Padding
		-- Pixel Array --


	*/

	// Create the pixel array and fill it monochromatically

	int numPixels = width*height;
	void* PixelArray = malloc(numPixels*sizeof(struct Pixel));
	struct Pixel* pixptr = (struct Pixel*)(PixelArray);

	struct Pixel p;
	p.r = (color == RED? 0xff : 0);
	p.g = (color == GREEN? 0xff : 0);
	p.b = (color == BLUE? 0xff : 0);

	int i = 0;
	for (i = 0; i < numPixels; ++i)
	{
		*pixptr++ = p;
	}


	// add padding to the end of the pixel array.
	// NOTE: Padding does not seem to be necessary.


	// Create and fill in the info for the headers.

	struct TwoByte* FileSig = malloc(sizeof(struct TwoByte));
	FileSig->a = 'B';
	FileSig->b = 'M';

	struct BitmapHeader* Header = malloc(sizeof(struct BitmapHeader));
	Header->FileSize = 0; // calculate this after image size in the struct below.
	Header->Reserved = 0;
	Header->PixelOffset = 0;

	struct DIBHeader* DHeader = malloc(sizeof(struct DIBHeader));
	DHeader->DIBHeaderSize = sizeof(struct DIBHeader);
	DHeader->ImageWidth = width;
	DHeader->ImageHeight = height;
	DHeader->PlanesAndBPP = 0x1 | 0x180000; // probably have to | two numbers together?
	DHeader->Compression = 0;
	DHeader->ImageSize = width*height*sizeof(struct Pixel); // number of pixels * bits per pixel (plus padding?)
	DHeader->Xppm = 0xec4; // how to determine these?
	DHeader->Yppm = 0xec4; // copied from other files.
	DHeader->ColorsInColorTable = 0;
	DHeader->ColorCount = 0;

	// Now calculate file size
	Header->FileSize = sizeof(struct TwoByte) + sizeof(struct BitmapHeader)
   			+ sizeof(struct DIBHeader) + DHeader->ImageSize;
   	Header->PixelOffset = sizeof(struct TwoByte) + sizeof(struct BitmapHeader)
   			+ sizeof(struct DIBHeader);


	// Write all of the data to the file in the correct order

	char* filename = "colorimg.bmp";
	FILE* file = fopen(filename, "w");

	fwrite(FileSig, sizeof(struct TwoByte), 1, file);
	fwrite(Header, sizeof(struct BitmapHeader), 1, file);
	fwrite(DHeader, sizeof(struct DIBHeader), 1, file);
	fwrite(PixelArray, sizeof(struct Pixel), numPixels, file);


	// free all the memory reserved by malloc

	free(PixelArray);
	free(Header);
	free(DHeader);
	free(FileSig);
	fclose(file);




	return 0;
}