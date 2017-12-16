// Dylan Balata
// CS241 Project 2
// Maze Generation

#include <stdio.h>
#include <stdlib.h>
#include "mazegen.h"

// Maze structure
unsigned char **maze;

// Maze dimensions
int rows;
int cols;

// Point solution must pass through
int globalXWayPoint;
int globalYWayPoint;

// Used in determining whether to free
int mazeExists;

// Used to determine if top or bottom exits has been made
int foundEnd;

// Whether to show solution
int solve;

// Frees memory allocated for maze
void mazeFree();

// Used in choosing random direction when carving maze
int randList[] = {0, 1, 2, 3};


// Utillity funciton when randomizing randList
void randomSwap(int x, int y)
{
  unsigned char tmp = randList[x];
  randList[x] = randList[y];
  randList[y] = tmp;
}

// Randomize randList to aid in maze complexity
void randomizeList()
{
  for (int i = TOTAL_DIRECTIONS-1; i > 0; i--)
    {
        int j = rand() % (i+1);
        randomSwap(i, j);
    }
}

// Zeros maze after allocation
void zeroMaze()
{ 
  for(int i=0;i<rows;i++)
  {
    for(int j=0;j<cols;j++)
    {
      maze[i][j] = 0;
    }
  }
}

// Determines if coordinates are within bounds of maze
int isInBounds(int x, int y)
{
  return x >= 0 && x < rows && y >= 0 && y < cols;
}

// Used to make symmetric wall breaks
int flipDirection(int x)
{
  if (x==NORTH) return SOUTH;
  if (x==SOUTH) return NORTH;
  if (x==EAST) return WEST;
  if (x==WEST) return EAST;
}

// Recursively calls self to carve out maze by checking all four directions
// in random order. first pass finds path to top while remaining in top half,
// then begins recursion at start point (waypoints). Then repeats for bottom
// half. During first part of each call marks solution path.
int carveMaze(int x, int y, int pass)
{
  
  int newX, newY, dir, barrier, endDir, endCond;

  // Depending on pass set direction to be exiting maze
  if(!pass)
  {
    endDir = NORTH;
    endCond = 0;
  }
  else
  {
    endDir = SOUTH;
    endCond = rows-1;
  }
  
  int onPath = 0; // On solution path

  // Checking for top or bottom of maze to male entrance or exit
  // then resume recursion at start
  if(!foundEnd)
  {
    if(x == endCond)
    {
      foundEnd = 1;
      maze[x][y] |= endDir;
      maze[x][y] |= SPECIAL;
      if (pass) carveMaze(x, y, pass);
      else carveMaze(globalXWayPoint, globalYWayPoint, pass);
      return 1;
    }
  }
  
  randomizeList();

  // Check in all directions for empty space within bounds to
  // continue recursion
  for(int i=0;i<TOTAL_DIRECTIONS;i++)
  {
    dir = randList[i];
    
    newY = y;
    newX = x;
    
    newX += DIRECTION_DY[dir];
    newY += DIRECTION_DX[dir];

    if(!pass) barrier = newX <= globalXWayPoint;
    else barrier = newX >= globalXWayPoint;
    
    if(isInBounds(newX, newY) && maze[newX][newY] == 0 && barrier)
    {
      maze[x][y] |=  DIRECTION_LIST[dir];
      maze[newX][newY] |= flipDirection(DIRECTION_LIST[dir]);
      if(carveMaze(newX, newY, pass) == 1)
      {
	maze[x][y] |= SPECIAL;
	onPath = 1;
      }
    }
  }
  if (onPath) return 1;
  else return 0;
}

// Check for invalid starting parameters
int checkError(int x, int y, int xx, int yy)
{
  if (x < 3 || y < 3)
  {
    printf("Array is too small.\n");
    return 1;
  }
  if (xx > x || yy > y || xx == 0 || yy == 0)
  {
    printf("Waypoint is out of bounds.\n");
    return 1;

  }
  return 0;
}

// Check for errors, allocate memory, then begin carving maze
int mazeGenerate(int width, int height,
		 int wayPointX, int wayPointY,
		 int wayPointAlleyLength,
		 double wayPointDirectionPercent,
		 double straightProbability,
		 int printAlgorithmSteps)
{
  if(mazeExists) mazeFree(); // Avoid leaks
  
  if(checkError(width, height, wayPointX, wayPointY))
  {
    return TRUE;
  }

  
  solve = 0; // Solving maze must be specified

  
  cols = width;
  rows = height;

  // Maze is 0 indexed, waypoints are 1
  globalXWayPoint = wayPointY - 1;
  globalYWayPoint = wayPointX - 1;

  // Allocate memory for maze, then set to zero
  maze = malloc(rows * sizeof(*maze));
  for(int i=0;i<rows;i++)
  {
    maze[i] = malloc(cols * sizeof(unsigned char));
  }
  zeroMaze();

  // Carve top then bottom
  foundEnd = 0;
  carveMaze(globalXWayPoint, globalYWayPoint, 0);

  foundEnd = 0;
  carveMaze(globalXWayPoint, globalYWayPoint, 1);

  // Mark waypoint in maze
  maze[globalXWayPoint][globalYWayPoint] |= GOAL;

  mazeExists = 1;

  return FALSE;
}

// Determines whether to show solution when printing
void mazeSolve()
{
  solve = 1;
}

// Print maze using pipe characters, prints solution in green,
// waypoint in red, and rest in white, requires CP866 encoding
// in terminal
void mazePrint()
{
  for(int i=0;i<rows;i++)
  {
    for(int j=0;j<cols;j++)
    {
      if((maze[i][j] & ALL_SPECIAL) & GOAL)
      {
	textcolor(TEXTCOLOR_RED);
      }
      else if (((maze[i][j] & ALL_SPECIAL) & SPECIAL) && solve)
      {
        textcolor(TEXTCOLOR_GREEN);
      }
      else
      {
	textcolor(TEXTCOLOR_WHITE);
      }
      
      printf("%c", pipeList[maze[i][j] & ALL_DIRECTIONS]);
    }
    printf("\n");
  }
  printf("\n");
}


// Checks to see if maze exists, then frees allocated memory
void mazeFree()
{
  if (mazeExists){ 
    for(int i=0;i<rows;i++)
    {
      free(maze[i]);
    }
    free(maze);
    mazeExists = 0;
  }
}

