// Dylan Balata
// cs241 Maze Bitmap
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mazegen.h"
 
#define TILE_SIZE 192
#define TILE_WIDTH 8
#define TILE_HEIGHT 8
#define BYTES_PER_PIXEL 3
#define BITS_PER_PIXEL 24
#define HEADER_SIZE 54

unsigned char *bitmap;
unsigned char **imageStorage;
unsigned char **imageFlip;

int pixelWidth;
int pixelHeight;
int fileSize;
int rowSize;

int fileReadCount;

unsigned char *pipeImageList[32];

// Converts ints to byte format used in bmp
void copyIntToAddress(int n, unsigned char bytes[])
{
  bytes[0] = n & 0xFF;
  bytes[1] = (n >>  8) & 0xFF;
  bytes[2] = (n >> 16) & 0xFF;
  bytes[3] = (n >> 24) & 0xFF;
}

// Constucts standard bmp header, only input is rows and cols
// from base maze
void makeBitmapHeader()
{
  pixelWidth = TILE_WIDTH * cols; 
  pixelHeight =TILE_HEIGHT * rows;
  rowSize = pixelWidth * 3;
  int rowPadding = 0;
  rowPadding = (4 - (rowSize % 4)) % 4;
  rowSize += rowPadding;
  int pixelDataSize = rowSize*pixelHeight;
  fileSize = 54 + pixelDataSize;
  bitmap = calloc(fileSize, sizeof(char));
  imageStorage = calloc(pixelHeight, sizeof(*imageStorage));
  for(int i=0;i<rowSize;i++)
  {
    imageStorage[i] = calloc(rowSize, sizeof(char));
  }
  imageFlip = calloc(pixelHeight, sizeof(*imageFlip));
  for(int i=0;i<rowSize;i++)
  {
    imageFlip[i] = calloc(rowSize, sizeof(char));
  }

  unsigned char header[54] = 
  {
    'B','M',  // magic number
    0,0,0,0,  // size in bytes (set below)
    0,0,0,0,  // reserved
    54,0,0,0, // offset to start of pixel data
    40,0,0,0, // info hd size
    0,0,0,0,  // image width (set below)
    0,0,0,0,  // image heigth (set below)
    1,0,      // number color planes
    24,0,     // bits per pixel
    0,0,0,0,  // compression is none
    0,0,0,0,  // image byte size
    0x13,0x0B,0,0, // horz resoluition in pixel / m
    0x13,0x0B,0,0, // vert resolutions (0x03C3 = 96 dpi, 0x0B13 = 72 dpi)
    0,0,0,0,  // #colors in pallete
    0,0,0,0,  // #important colors
  };

  copyIntToAddress(fileSize, &header[2]);
  copyIntToAddress(pixelWidth, &header[18]);
  copyIntToAddress(pixelHeight, &header[22]);
  copyIntToAddress(pixelDataSize, &header[34]);

  for(int i=0;i<54;i++)
  {
    bitmap[i] = header[i];
  }
}

// Places 8x8 maze tile into storage array
void placeTile(int startCol, int startRow, unsigned char *image)
{
  int idx = 0;
  for(int i=startCol; i<startCol+8; i++)
  {
    for(int j=startRow; j<startRow+8*3; j++)
    {
      imageStorage[i][j] = image[idx];
      idx++;
    }
  }
}

void writeBitmap()
{
  if(solve)
  {
    FILE* myFile = fopen("mazeSolved.bmp", "wb");
    fwrite(bitmap, 1, fileSize, myFile);
    fclose(myFile);
  }
  else
  {
    FILE* myFile = fopen("maze.bmp", "wb");
    fwrite(bitmap, 1, fileSize, myFile);
    fclose(myFile);
  }
}

// Converts base maze to bitmap by creating mapping between
// chars in maze[][] and 8x8 tiles
void makeBitmapImage()
{
  for(int i=0;i<pixelHeight;i++)
  {
    for(int j=0;j<rowSize;j++)
    {
      imageStorage[i][j] = 0;
    }
  }
  
  for(int i = 0; i < rows; i++)
  {
    for(int j = 0; j < cols; j++)
    {
      if(((maze[i][j] & ALL_SPECIAL) & SPECIAL) && solve)
      {
	placeTile(i*8, j*8*3,
		  pipeImageList[(maze[i][j]&ALL_DIRECTIONS)+16]);
      }
      else
      {
      placeTile(i*8, j*8*3,
		pipeImageList[maze[i][j]&ALL_DIRECTIONS]);
      }
    }
  }

  // Have to flip image as bmp store rows in reverse order
  int flipRow = 0;
  int copyRow = pixelHeight;
  while(flipRow<pixelHeight)
  {
    for(int i=0;i<pixelWidth*3;i++)
    {
      imageFlip[flipRow][i] = imageStorage[copyRow][i];
    }
    flipRow++;
    copyRow--;
  }

  int idx = 54;
  for(int i=0;i<pixelHeight;i++)
  {
    for(int j=0;j<rowSize;j++)
    {
      bitmap[idx] = imageFlip[i][j];
      idx++;
    }
  }
  writeBitmap();
}

// Extract RGB data portion of 8x8 bmp tile
void extractImage(char fileName[])
{
  int const FILE_SIZE = HEADER_SIZE + TILE_SIZE;
  
  unsigned char storage[FILE_SIZE];
  
  FILE *inFile = fopen(fileName,"rb");  
  fread(storage, sizeof(char), FILE_SIZE, inFile);
  fclose(inFile);

  unsigned char *copy;
  copy = malloc(sizeof(char)*TILE_SIZE);
  copy = &storage[HEADER_SIZE];

  unsigned char  flip[TILE_SIZE];
  int flipRow = 0;
  int copyRow = 7;
  while(flipRow<8)
  {
    for(int i=0;i<24;i++)
    {
      flip[flipRow*24+i] = copy[copyRow*24+i];
    }
    flipRow++;
    copyRow--;
  }

  for(int i=0;i<TILE_SIZE;i++)
  {
   pipeImageList[fileReadCount][i]  = flip[i];
  }

  //free(copy); crashes program

  fileReadCount++;
}

// Load RGB data from 8x8 24bit bmp files created in GIMP
// *S is green solved version
void loadTiles()
{
  fileReadCount = 0;
  for(int i=0;i<32;i++)
  {
    pipeImageList[i] = malloc(sizeof(char)*8*8*3);
  }
  extractImage("blank.bmp");
  extractImage("north.bmp");
  extractImage("east.bmp");
  extractImage("northEast.bmp");
  extractImage("south.bmp");
  extractImage("northSouth.bmp");
  extractImage("eastSouth.bmp");
  extractImage("northEastSouth.bmp");
  extractImage("west.bmp");
  extractImage("northWest.bmp");
  extractImage("eastWest.bmp");
  extractImage("northEastWest.bmp");
  extractImage("southWest.bmp");
  extractImage("northSouthWest.bmp");
  extractImage("eastSouthWest.bmp");
  extractImage("all.bmp");
  extractImage("blank.bmp");
  extractImage("northS.bmp");
  extractImage("eastS.bmp");
  extractImage("northEastS.bmp");
  extractImage("southS.bmp");
  extractImage("northSouthS.bmp");
  extractImage("eastSouthS.bmp");
  extractImage("northEastSouthS.bmp");
  extractImage("westS.bmp");
  extractImage("northWestS.bmp");
  extractImage("eastWestS.bmp");
  extractImage("northEastWestS.bmp");
  extractImage("southWestS.bmp");
  extractImage("northSouthWestS.bmp");
  extractImage("eastSouthWestS.bmp");
  extractImage("allS.bmp");
}

void cleanup()
{
  for(int i=0;i<32;i++)
  {
    free(pipeImageList[i]);
  }
  for(int i=0;i<rowSize;i++)
  {
    //free(imageStorage[i]);
    //free(imageFlip[i]);
  }
  //free(imageFlip);
  //free(imageStorage);
}

void main()
{
  loadTiles();
  srand((unsigned)time(NULL));
  
  mazeGenerate(1000,1000,   500,500,0,  1.0,  0.0, TRUE);
  makeBitmapHeader();
  makeBitmapImage();
  mazeSolve();
  mazePrint();
  makeBitmapImage();
  cleanup();
}
  

  
  
  
