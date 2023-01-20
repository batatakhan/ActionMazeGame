/*
	CPSC 359 - Assignment 4
	  RPi Mario Kart Game

 Done by:
 	
	Name: Areeb Khan
	UCID: 30130100

	Name: Arhum Ladak
	UCID: 30116116
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wiringPi.h>
#include <pthread.h>
#include <time.h>
#include <sys/mman.h>
#include "initGPIO.h"
#include "framebuffer.h"
#include "startMenu.h"
#include "quitMenu.h"
#include "wall.h"
#include "player.h"
#include "path.h"
#include "goomba.h"
#include "ghost.h"
#include "up.h"
#include "down.h"
#include "right.h"
#include "left.h"
#include "finish.h"
#include "restart.h"
#include "quit.h"
#include "heart.h"
#include "timemultiply.h"
#include "scoremultiply.h"
#include "greenkey.h"
#include "bluekey.h"
#include "yellowkey.h"
#include "greengate.h"
#include "bluegate.h"
#include "yellowgate.h"
#include "gameWon.h"
#include "gameLost.h"
#include "liv.h"
#include "es.h"
#include "ti.h"
#include "me.h"
#include "sco.h"
#include "re.h"
#include "zero.h"
#include "one.h"
#include "two.h"
#include "three.h"
#include "four.h"
#include "five.h"
#include "six.h"
#include "seven.h"
#include "eight.h"
#include "nine.h"

// Definitions
typedef struct {
	int color;
	int x, y;
} Pixel;

// Method Declarations
struct fbs framebufferstruct;
void drawPixel(Pixel *pixel);
void *timer(void *param);
void determineLivesDisplay();
void determineScoreDisplay();
void determineTimeDisplay();
void drawEndGameScreen (int menuToChoose);
void *placeValuePacks(void *param);
void drawInitialScreen(int menuToChoose);
void drawMainMenuScreen();
void drawPauseMenuScreen();
void drawPauseInterruptScreen(int menuToChoose);
void determineValuePackPos();
void setMapPtr(char gameMap[9][16]);
void drawLevel1();
void drawLevel2();
void drawLevel3();
void drawLevel4();
void movement();
void drawMap();
void determineAdjacentBlocks();
void drawObject(int x_cell, int y_cell, char tile_type);
void Init_SNES();
void Init_GPIO(int pin_number, int function_mode);
void Write_Latch(int function_mode);
void Write_Clock(int function_mode);
int Read_Data();
void Wait(int wait_time);
int Read_SNES();

// Global Variables
Pixel *pixel;
short int *startMenuPtr = (short int *) startMenu.pixel_data;
short int *quitMenuPtr = (short int *) quitMenu.pixel_data;
short int *marioPtr = (short int *) player.pixel_data;
short int *pathPtr = (short int *) path.pixel_data;
short int *wallPtr = (short int *) wall.pixel_data;
short int *upPtr = (short int *) up.pixel_data;
short int *downPtr = (short int *) down.pixel_data;
short int *rightPtr = (short int *) right.pixel_data;
short int *leftPtr = (short int *) left.pixel_data;
short int *goombaPtr = (short int *) goomba.pixel_data;
short int *ghostPtr = (short int *) ghost.pixel_data;
short int *finishPtr = (short int *) finish.pixel_data;
short int *restartPtr = (short int *) restart.pixel_data;
short int *quitPtr = (short int *) quit.pixel_data;
short int *scorePtr = (short int *) scoremultiply.pixel_data;
short int *timePtr = (short int *) timemultiply.pixel_data;
short int *heartPtr = (short int *) heart.pixel_data;
short int *yellowkeyPtr = (short int *) yellowkey.pixel_data;
short int *greenkeyPtr = (short int *) greenkey.pixel_data;
short int *bluekeyPtr = (short int *) bluekey.pixel_data;
short int *greengatePtr = (short int *) greengate.pixel_data;
short int *yellowgatePtr = (short int *) yellowgate.pixel_data;
short int *bluegatePtr = (short int *) bluegate.pixel_data;
short int *gameWonPtr = (short int *) gameWon.pixel_data;
short int *gameLostPtr = (short int *) gameLost.pixel_data;
short int *livPtr = (short int *) liv.pixel_data;
short int *esPtr = (short int *) es.pixel_data;
short int *tiPtr = (short int *) ti.pixel_data;
short int *mePtr = (short int *) me.pixel_data;
short int *scoPtr = (short int *) sco.pixel_data;
short int *rePtr = (short int *) re.pixel_data;
short int *zeroPtr = (short int *) zero.pixel_data;
short int *onePtr = (short int *) one.pixel_data;
short int *twoPtr = (short int *) two.pixel_data;
short int *threePtr = (short int *) three.pixel_data;
short int *fourPtr = (short int *) four.pixel_data;
short int *fivePtr = (short int *) five.pixel_data;
short int *sixPtr = (short int *) six.pixel_data;
short int *sevenPtr = (short int *) seven.pixel_data;
short int *eightPtr = (short int *) eight.pixel_data;
short int *ninePtr = (short int *) nine.pixel_data;

int pauseMenuDisplayed = FALSE;
int quitMenuFlag = FALSE;
int winFlag = FALSE;
int loseFlag = FALSE;
int mainMenuFlag = FALSE;

unsigned int *gpioPtr;
char mapPtr[9][16];

#define GPSET0 7 // 28/4
#define GPCLR0 10 // 40/4
#define GPLEV0 13 // 52/4

// Structs
struct character {
	int lives;
	int score;
	int timeRemaining;
};

struct vehicle {
	int x;
	int y;
};

struct adjacentPaths {
	int up_x;
	int up_y;
	
	int down_x;
	int down_y;
	
	int right_x;
	int right_y;
	
	int left_x;
	int left_y;
};

struct valuePack {
	int displayed;
	int timeToPlace;
	
	int heart_x;
	int heart_y;
	
	int scoreMultiplier_x;
	int scoreMultiplier_y;
	
	int timeMultiplier_x;
	int timeMultiplier_y;
};

struct character mario;
struct vehicle kart;
struct adjacentPaths nextMove;
struct valuePack valuePacks;

// Main function
// This is the function that runs the game
int main(){
	
	/* initialize + get FBS */
	framebufferstruct = initFbInfo();
	
	/* initialize SNES lines */
	gpioPtr = getGPIOPtr(); // get gpio pointer
	Init_SNES();

	pixel = malloc(sizeof(Pixel)); // initialize a pixel
	
	pthread_t tid1; // initialize a pthread
	
	srand(time(NULL)); // sets the seed as time for the random number generator algorithm used by the function rand()
	
	mario.timeRemaining = 200;
	drawMainMenuScreen(); // display main menu
	
	while (!quitMenuFlag) {

		mario.lives = 4;
		mario.score = 0;
		
		pthread_create(&tid1, NULL, timer, NULL); // create a pthread
		drawLevel1(); // start Level 1/4 of the game
		pthread_cancel(tid1); // end the pthread
		
		mario.timeRemaining = 200;
		
		// check if game won or lost
		if (winFlag) {
			drawEndGameScreen(TRUE);
			winFlag = FALSE;
			Read_SNES();
		} else if (loseFlag) {
			determineScoreDisplay();

			drawEndGameScreen(FALSE);
			loseFlag = FALSE;
			Read_SNES();
		} else mainMenuFlag = FALSE;
			
		Wait(250000);
		
		valuePacks.displayed = FALSE;
		pauseMenuDisplayed = FALSE;

		drawMainMenuScreen();
	}
	
	
	// free pixel's allocated memory
	free(pixel);
	pixel = NULL;
	munmap(framebufferstruct.fptr, framebufferstruct.screenSize);
	
	return 0;
}

// Draw a pixel
void drawPixel(Pixel *pixel){
	long int location =(pixel->x +framebufferstruct.xOff) * (framebufferstruct.bits/8) +
                       (pixel->y+framebufferstruct.yOff) * framebufferstruct.lineLength;
	*((unsigned short int*)(framebufferstruct.fptr + location)) = pixel->color;
}

// thread executed function
// This function is used to keep track of the time and display it every second.
void *timer(void *param) {
	
	while(mario.timeRemaining >= 0) {
		
		determineTimeDisplay();

		// Make sure that the value packs are displayed after 30 seconds
		if (!valuePacks.displayed && mario.timeRemaining == valuePacks.timeToPlace) {
			valuePacks.displayed = TRUE;
			
			mapPtr[valuePacks.heart_y][valuePacks.heart_x] = 'L';
			mapPtr[valuePacks.scoreMultiplier_y][valuePacks.scoreMultiplier_x] = '$';
			mapPtr[valuePacks.timeMultiplier_y][valuePacks.timeMultiplier_x] = 'T';
			
			drawMap();
		}
		
		if (mario.timeRemaining > 0) {
			mario.timeRemaining--;
			Wait(1000000);
		}
		
		while(pauseMenuDisplayed);
		drawMap();
	}
	
	
	loseFlag = TRUE;
	pthread_exit(NULL); // exit pthread
}

// This function is used to check how many lives are left. The number of lives are stored at the location (in the game map) where it will be displayed.
void determineLivesDisplay() {

	if (mario.lives == 8)
		mapPtr[8][2] = '8';
	else if (mario.lives == 7)
		mapPtr[8][2] = '7';
	else if (mario.lives == 6)
		mapPtr[8][2] = '6';
	else if (mario.lives == 5)
		mapPtr[8][2] = '5';
	else if (mario.lives == 4)
		mapPtr[8][2] = '4';
	else if (mario.lives == 3)
		mapPtr[8][2] = '3';
	else if (mario.lives == 2)
		mapPtr[8][2] = '2';
	else if (mario.lives == 1)
		mapPtr[8][2] = '1';	
	else mapPtr[8][2] = '0';
}

// // This function is used to check and store the Total Score at the location where it will be displayed in the game map.
void determineScoreDisplay() {
	
	if (mario.timeRemaining <= 0)
		mario.score = 0;
		
	if (mario.score >= 9000)
		mapPtr[8][12] = '9';
	else if (mario.score >= 8000)
		mapPtr[8][12] = '8';	
	else if (mario.score >= 7000)
		mapPtr[8][12] = '7';	
	else if (mario.score >= 6000)
		mapPtr[8][12] = '6';	
	else if (mario.score >= 5000)
		mapPtr[8][12] = '5';	
	else if (mario.score >= 4000)
		mapPtr[8][12] = '4';	
	else if (mario.score >= 3000)
		mapPtr[8][12] = '3';
	else if (mario.score >= 2000)
		mapPtr[8][12] = '2';	
	else if (mario.score >= 1000)
		mapPtr[8][12] = '1';
	else mapPtr[8][12] = '0';
	
	if ((mario.score % 1000) >= 900)
		mapPtr[8][13] = '9';
	else if ((mario.score % 1000) >= 800)
		mapPtr[8][13] = '8';	
	else if ((mario.score % 1000) >= 700)
		mapPtr[8][13] = '7';	
	else if ((mario.score % 1000) >= 600)
		mapPtr[8][13] = '6';	
	else if ((mario.score % 1000) >= 500)
		mapPtr[8][13] = '5';	
	else if ((mario.score % 1000) >= 400)
		mapPtr[8][13] = '4';	
	else if ((mario.score % 1000) >= 300)
		mapPtr[8][13] = '3';
	else if ((mario.score % 1000) >= 200)
		mapPtr[8][13] = '2';	
	else if ((mario.score % 1000) >= 100)
		mapPtr[8][13] = '1';
	else mapPtr[8][13] = '0';
	
	if ((mario.score % 100) >= 90)
		mapPtr[8][14] = '9';
	else if ((mario.score % 100) >= 80)
		mapPtr[8][14] = '8';	
	else if ((mario.score % 100) >= 70)
		mapPtr[8][14] = '7';	
	else if ((mario.score % 100) >= 60)
		mapPtr[8][14] = '6';	
	else if ((mario.score % 100) >= 50)
		mapPtr[8][14] = '5';	
	else if ((mario.score % 100) >= 40)
		mapPtr[8][14] = '4';	
	else if ((mario.score % 100) >= 30)
		mapPtr[8][14] = '3';
	else if ((mario.score % 100) >= 20)
		mapPtr[8][14] = '2';	
	else if ((mario.score % 100) >= 10)
		mapPtr[8][14] = '1';
	else mapPtr[8][14] = '0';
	
	if ((mario.score % 10) == 9)
		mapPtr[8][15] = '9';
	else if ((mario.score % 10) == 8)
		mapPtr[8][15] = '8';
	else if ((mario.score % 10) == 7)
		mapPtr[8][15] = '7';
	else if ((mario.score % 10) == 6)
		mapPtr[8][15] = '6';
	else if ((mario.score % 10) == 5)
		mapPtr[8][15] = '5';
	else if ((mario.score % 10) == 4)
		mapPtr[8][15] = '4';
	else if ((mario.score % 10) == 3)
		mapPtr[8][15] = '3';
	else if ((mario.score % 10) == 2)
		mapPtr[8][15] = '2';
	else if ((mario.score % 10) == 1)
		mapPtr[8][15] = '1';	
	else mapPtr[8][15] = '0';
}

// // This function is used to check the time left before game ends. The time is then stored at the location where it will be displayed in the game map.
void determineTimeDisplay() {

	if (mario.timeRemaining == 200)
		mapPtr[8][6] = '2';
	else if (mario.timeRemaining >= 100)
		mapPtr[8][6] = '1';
	else mapPtr[8][6] = '0';
	
	if ((mario.timeRemaining % 100) >= 90)
		mapPtr[8][7] = '9';
	else if ((mario.timeRemaining % 100) >= 80)
		mapPtr[8][7] = '8';	
	else if ((mario.timeRemaining % 100) >= 70)
		mapPtr[8][7] = '7';	
	else if ((mario.timeRemaining % 100) >= 60)
		mapPtr[8][7] = '6';	
	else if ((mario.timeRemaining % 100) >= 50)
		mapPtr[8][7] = '5';	
	else if ((mario.timeRemaining % 100) >= 40)
		mapPtr[8][7] = '4';	
	else if ((mario.timeRemaining % 100) >= 30)
		mapPtr[8][7] = '3';
	else if ((mario.timeRemaining % 100) >= 20)
		mapPtr[8][7] = '2';	
	else if ((mario.timeRemaining % 100) >= 10)
		mapPtr[8][7] = '1';
	else mapPtr[8][7] = '0';
	
	if ((mario.timeRemaining % 10) == 9)
		mapPtr[8][8] = '9';
	else if ((mario.timeRemaining % 10) == 8)
		mapPtr[8][8] = '8';
	else if ((mario.timeRemaining % 10) == 7)
		mapPtr[8][8] = '7';
	else if ((mario.timeRemaining % 10) == 6)
		mapPtr[8][8] = '6';
	else if ((mario.timeRemaining % 10) == 5)
		mapPtr[8][8] = '5';
	else if ((mario.timeRemaining % 10) == 4)
		mapPtr[8][8] = '4';
	else if ((mario.timeRemaining % 10) == 3)
		mapPtr[8][8] = '3';
	else if ((mario.timeRemaining % 10) == 2)
		mapPtr[8][8] = '2';
	else if ((mario.timeRemaining % 10) == 1)
		mapPtr[8][8] = '1';	
	else mapPtr[8][8] = '0';
}


// function to draw the screen which will be displayed when the game is "won" or "lost"
// it takes an integer as a parameter which is used to determine if the game is won or lost.
void drawEndGameScreen (int menuToChoose) {
	
	// borders for the display screen
	for (int y = 120; y < 600; y++) {
		for (int x = 120; x < 1160; x++) {	
				pixel->color = 0x0000; // black pixel
				pixel->x = x;
				pixel->y = y;
	
				drawPixel(pixel);
		}
	}
	
	short int *pointerToUse;
	if (menuToChoose) // if menuToChoose == 1, then gameWon screen else gameLost screen
		pointerToUse = gameWonPtr;
	else pointerToUse = gameLostPtr;
	
	// draw the 'end game' screen
	int i = 0;
	for (int y = 160; y < 560; y++) {
		for (int x = 160; x < 1120; x++) {
				
				pixel->color = pointerToUse[i];
				pixel->x = x;
				pixel->y = y;
	
				drawPixel(pixel);
				i++;	
		}
	}
	
	// display the score on the 'end game' screen
	mapPtr[6][8] = mapPtr[8][10];
	mapPtr[6][9] = mapPtr[8][11];
	mapPtr[6][10] = mapPtr[8][12];
	mapPtr[6][11] = mapPtr[8][13];
	mapPtr[6][12] = mapPtr[8][14];
	mapPtr[6][13] = mapPtr[8][15];
	
	for (int i = 8; i < 14; i++) {
		drawObject(i, 6, mapPtr[6][i]);
	}
}

// This function is used to display the main menu screen. It takes a parameter which is used to determine if the pointer is on 'start game' or 'quit game'
void drawInitialScreen(int menuToChoose) {
	
	short int *pointerToUse;
	if (menuToChoose)
		pointerToUse = startMenuPtr;
	else pointerToUse = quitMenuPtr;
	
	int i = 0;
	for (int y = 0; y < 720; y++) {
		for (int x = 0; x < 1280; x++) {
				
				pixel->color = pointerToUse[i];
				pixel->x = x;
				pixel->y = y;
	
				drawPixel(pixel);
				i++;	
		}
	}
}

// This function is used to determine if the pointer is on 'start game' or 'quit game'. It then calls another function to display the Main Menu screen.
void drawMainMenuScreen() {
	drawInitialScreen(TRUE);
	drawInitialScreen(TRUE);
	int button_pressed = Read_SNES();
	
	while (button_pressed != 9) {
		if (button_pressed == 5) {
			drawInitialScreen(TRUE);
			quitMenuFlag = FALSE;
		} else if (button_pressed == 6) {
			drawInitialScreen(FALSE);
			quitMenuFlag = TRUE;
		}
		
		button_pressed = Read_SNES();
	}
}

// This function is called when the user hits the pause (START) button during the game. It determines if the pointer is on 'restart game' or 'quit game'.
// It then calls another function to display the Pause Menu screen.
void drawPauseMenuScreen() {
	int restartMenu = TRUE;
	pauseMenuDisplayed = TRUE;
	
	drawPauseInterruptScreen(TRUE);
	int button_pressed = Read_SNES();
	
	while (button_pressed != 4 && button_pressed != 9) {
		
		if (button_pressed == 5) {
				drawPauseInterruptScreen(TRUE);
				restartMenu = TRUE;
		} else if (button_pressed == 6) {
				drawPauseInterruptScreen(FALSE);
				restartMenu = FALSE;
		} else if (button_pressed == 0) {
				restartMenu = FALSE;
				break;
		}
		
		Wait(250000);
		button_pressed = Read_SNES();
	}
	
	if (button_pressed == 4)
		pauseMenuDisplayed = FALSE;
	else if (restartMenu) {
		mario.lives = 4;
		mario.score = 0;
		mario.timeRemaining = 200;
		
		valuePacks.displayed = FALSE;
		pauseMenuDisplayed = FALSE;
		
		drawLevel1(); // restart from level 1
		
	} else {
		mainMenuFlag = TRUE;
		mario.timeRemaining = -1;
	}
	
}

// // This function is used to display the pause menu screen. It takes a parameter which is used to determine if the pointer is on 'restart game' or 'quit game'
void drawPauseInterruptScreen(int menuToChoose) {
	
	short int *pointerToUse;
	if (menuToChoose)
		pointerToUse = restartPtr;
	else pointerToUse = quitPtr;
	
	int i = 0;
	for (int y = 80; y < 640; y++) {
		for (int x = 400; x < 880; x++) {
				
				pixel->color = pointerToUse[i];
				pixel->x = x;
				pixel->y = y;
	
				drawPixel(pixel);
				i++;	
		}
	}
}


// Finds random positions for the value packs to spawn. It makes sure that the randomly chosen position is empty
void determineValuePackPos() {
	int xCoord;
	int yCoord;
	
	do {
		xCoord = (rand() % 16);
		yCoord = (rand() % 9);
	} while (mapPtr[yCoord][xCoord] != 'P');
	
	valuePacks.heart_x = xCoord;
	valuePacks.heart_y = yCoord;
	
	do {
		xCoord = (rand() % 16);
		yCoord = (rand() % 9);
	} while (mapPtr[yCoord][xCoord] != 'P');
	
	valuePacks.scoreMultiplier_x = xCoord;
	valuePacks.scoreMultiplier_y = yCoord;
	
	do {
		xCoord = (rand() % 16);
		yCoord = (rand() % 9);
	} while (mapPtr[yCoord][xCoord] != 'P');
	
	valuePacks.timeMultiplier_x = xCoord;
	valuePacks.timeMultiplier_y = yCoord;
}

void setMapPtr(char gameMap[9][16]) {
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 16; j++) {
			mapPtr[i][j] = gameMap[i][j];
		}
	}
}

// This function is called to start Level 1/4 of the game.
void drawLevel1() {
	// Initial position of the game map.
	char gameMap[9][16] = {	{'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W'},
							{'W', 'P', 'P', 'W', 'P', 'P', 'P', 'G', 'W', 'P', 'P', 'G', 'P', 'P', 'F', 'W'},
							{'W', 'P', 'W', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'W', 'P', 'U', 'P', 'P', 'W'},
							{'W', 'P', 'W', 'P', 'P', 'P', 'P', 'U', 'U', 'P', 'P', 'P', 'W', 'P', 'W', 'W'},
							{'W', 'P', 'P', 'W', 'G', 'P', 'W', 'W', 'W', 'P', 'U', 'P', 'P', 'P', 'G', 'W'},
							{'W', 'P', 'G', 'W', 'P', 'P', 'W', 'P', 'P', 'P', 'W', 'P', 'U', 'P', 'P', 'W'},
							{'W', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'W', 'W', 'P', 'W', 'P', 'U', 'W'},
							{'W', 'M', 'U', 'U', 'P', 'P', 'P', 'P', 'P', 'P', 'D', 'P', 'P', 'P', 'W', 'W'},
							{'V', 'E', 'W', 'W', 'I', 'N', 'W', 'W', 'W', 'W', 'O', 'R', 'W', 'W', 'W', 'W'} };
	
	// updating the game map with displays values for score and lives left
	determineLivesDisplay();
	determineScoreDisplay();
	
	setMapPtr(gameMap); // game map updated
	
	determineValuePackPos(); // spawn value packs
	valuePacks.timeToPlace = mario.timeRemaining - 30;
	
	drawMap(); // display the game map
	
	kart.x = 1;
	kart.y = 7;
	
	movement();
	
	if (winFlag) { // check if the level is successfully completed
		mario.score += (mario.lives + mario.timeRemaining)*7;
		
		valuePacks.displayed = FALSE;
		winFlag = FALSE;
		drawLevel2(); // go to level 2/4
	}
}

// This function is called to start Level 2/4 of the game.
void drawLevel2() {
	// Initial position of the game map.
	char gameMap[9][16] = {	{'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W'},
							{'W', 'P', 'P', 'W', 'W', 'D', 'D', 'P', 'P', 'P', 'D', 'P', 'P', 'P', 'F', 'W'},
							{'W', 'P', 'P', 'W', 'P', 'P', 'P', 'P', 'G', 'P', 'P', 'P', 'W', 'H', 'P', 'W'},
							{'W', 'P', 'P', 'P', 'P', 'P', 'G', 'P', 'P', 'P', 'H', 'P', 'W', 'P', 'P', 'W'},
							{'W', 'H', 'W', 'P', 'G', 'P', 'P', 'P', 'W', 'W', 'W', 'P', 'W', 'P', 'P', 'W'},
							{'W', 'P', 'P', 'P', 'P', 'H', 'P', 'W', 'P', 'W', 'P', 'P', 'P', 'P', 'P', 'W'},
							{'W', 'P', 'W', 'G', 'W', 'W', 'P', 'P', 'P', 'P', 'P', 'G', 'W', 'P', 'P', 'W'},
							{'W', 'M', 'W', 'P', 'P', 'P', 'P', 'U', 'G', 'P', 'P', 'P', 'W', 'U', 'U', 'W'},
							{'V', 'E', 'W', 'W', 'I', 'N', 'W', 'W', 'W', 'W', 'O', 'R', 'W', 'W', 'W', 'W'} };
		
	setMapPtr(gameMap);
	
	determineLivesDisplay();
	determineScoreDisplay();
	
	determineValuePackPos();
	valuePacks.timeToPlace = mario.timeRemaining - 30;
	
	drawMap();
	
	kart.x = 1;
	kart.y = 7;
	
	movement();
	
	if (winFlag) {
		mario.score += (mario.lives + mario.timeRemaining)*7;
		
		valuePacks.displayed = FALSE;
		winFlag = FALSE;
		drawLevel3(); // go to level 3/4
	}
}

// This function is called to start Level 3/4 of the game.
void drawLevel3() {
	// Initial position of the game map.
	char gameMap[9][16] = {	{'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W'},
							{'W', 'W', 'W', 'W', 'P', 'W', 'W', 'P', 'P', 'P', 'P', 'P', 'G', 'W', 'F', 'W'},
							{'W', 'W', 'D', 'W', 'P', 'W', 'W', 'P', 'P', 'G', 'P', 'P', 'P', 'W', 'P', 'W'},
							{'W', 'W', 'P', 'P', 'P', 'P', 'P', 'P', 'W', 'P', 'P', 'H', 'P', 'W', 'P', 'W'},
							{'W', 'P', 'U', 'H', 'P', 'P', 'P', 'H', 'W', 'P', 'P', 'G', 'P', 'A', 'P', 'W'},
							{'W', 'H', 'W', 'P', 'P', 'G', 'P', 'P', 'P', 'W', 'P', 'W', 'P', 'A', 'P', 'W'},
							{'W', 'P', 'P', 'P', 'H', 'P', 'P', 'P', 'P', 'P', 'P', 'H', 'P', 'W', 'P', 'W'},
							{'W', 'M', 'U', 'P', 'P', 'P', 'G', 'S', 'W', 'W', 'X', 'W', 'P', 'W', 'H', 'W'},
							{'V', 'E', 'W', 'W', 'I', 'N', 'W', 'W', 'W', 'W', 'O', 'R', 'W', 'W', 'W', 'W'} };
		
	setMapPtr(gameMap);
	
	determineLivesDisplay();
	determineScoreDisplay();
	
	determineValuePackPos();
	valuePacks.timeToPlace = mario.timeRemaining - 30;
	
	drawMap();
	
	kart.x = 1;
	kart.y = 7;
	
	movement();
	
	if (winFlag) {
		mario.score += (mario.lives + mario.timeRemaining)*7;
		
		valuePacks.displayed = FALSE;
		winFlag = FALSE;
		drawLevel4(); // go to level 4/4
	}
}

// This function is called to start Level 4/4 of the game.
void drawLevel4() {
	// Initial position of the game map.
	char gameMap[9][16] = {	{'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W', 'W'},
							{'W', 'W', 'W', 'W', 'P', 'Z', 'W', 'P', 'P', 'P', 'P', 'P', 'G', 'W', 'F', 'W'},
							{'W', 'J', 'D', 'W', 'B', 'W', 'W', 'P', 'P', 'G', 'P', 'H', 'P', 'W', 'P', 'W'},
							{'W', 'J', 'P', 'P', 'P', 'P', 'P', 'P', 'W', 'H', 'P', 'P', 'P', 'W', 'P', 'W'},
							{'W', 'P', 'P', 'W', 'P', 'P', 'P', 'H', 'W', 'J', 'P', 'G', 'P', 'A', 'P', 'W'},
							{'W', 'P', 'W', 'W', 'P', 'G', 'P', 'P', 'W', 'W', 'C', 'W', 'P', 'A', 'P', 'W'},
							{'W', 'P', 'W', 'W', 'H', 'P', 'P', 'S', 'W', 'W', 'P', 'W', 'P', 'W', 'P', 'W'},
							{'W', 'M', 'W', 'W', 'J', 'U', 'Y', 'S', 'W', 'W', 'X', 'W', 'P', 'W', 'H', 'W'},
							{'V', 'E', 'W', 'W', 'I', 'N', 'W', 'W', 'W', 'W', 'O', 'R', 'W', 'W', 'W', 'W'} };
		
	setMapPtr(gameMap);
	
	determineLivesDisplay();
	determineScoreDisplay();
	
	determineValuePackPos();
	valuePacks.timeToPlace = mario.timeRemaining - 30;
	
	drawMap();
	
	kart.x = 1;
	kart.y = 7;
	
	movement();
	// calculating the final score. Adding remaining time and remaining lives, and multiplying the sum with a constant (7).
	mario.score += (mario.lives + mario.timeRemaining)*7;
	determineScoreDisplay();
}


// This the main method which is used to move the mario. This method determines if mario hits and obstacel, picks up the value pack, reaches finsih line, etc.
void movement() {
	int button_pressed;
	char obstacleToReplace;
	
	int isHit = FALSE;
	int heartTaken = FALSE;
	int scoreTaken = FALSE;
	int timeTaken = FALSE;
	
	determineAdjacentBlocks(); // find and store the adjacent blocks from the current location
	
	while (!winFlag && !loseFlag && !mainMenuFlag) {
		button_pressed = Read_SNES();

		switch (button_pressed) {
				case 4: // If 'START' is pressed, go to pause menu screen
					Wait(150000);
					drawPauseMenuScreen();
					break;
					
				case 5: // 'JOY PAD UP ARROW'
					// Only allowed to move if the block if empty or contains a value pack
					if (mapPtr[nextMove.up_y][nextMove.up_x] == 'P' || mapPtr[nextMove.up_y][nextMove.up_x] == 'X' || mapPtr[nextMove.up_y][nextMove.up_x] == 'Y' || mapPtr[nextMove.up_y][nextMove.up_x] == 'Z' || mapPtr[nextMove.up_y][nextMove.up_x] == 'U' || mapPtr[nextMove.up_y][nextMove.up_x] == 'D' || mapPtr[nextMove.up_y][nextMove.up_x] == 'S' || mapPtr[nextMove.up_y][nextMove.up_x] == 'J' || mapPtr[nextMove.up_y][nextMove.up_x] == 'G' || mapPtr[nextMove.up_y][nextMove.up_x] == 'H' || mapPtr[nextMove.up_y][nextMove.up_x] == 'L' || mapPtr[nextMove.up_y][nextMove.up_x] == '$' || mapPtr[nextMove.up_y][nextMove.up_x] == 'T' || mapPtr[nextMove.up_y][nextMove.up_x] == 'F') {
						
						// When an obstacle is hit. lives = lives - 1;
						if (mapPtr[nextMove.up_y][nextMove.up_x] == 'U' || mapPtr[nextMove.up_y][nextMove.up_x] == 'D' || mapPtr[nextMove.up_y][nextMove.up_x] == 'S' || mapPtr[nextMove.up_y][nextMove.up_x] == 'J' || mapPtr[nextMove.up_y][nextMove.up_x] == 'G' || mapPtr[nextMove.up_y][nextMove.up_x] == 'H') {
							obstacleToReplace = mapPtr[nextMove.up_y][nextMove.up_x];
							isHit = TRUE;
						}
						
						mapPtr[nextMove.up_y][nextMove.up_x] = 'M';
						mapPtr[kart.y][kart.x] = 'P';
						kart.x = nextMove.up_x;
						kart.y = nextMove.up_y;
						
						determineAdjacentBlocks();
						
						if (isHit) {
							drawMap();
							mario.lives--;
						
							mapPtr[nextMove.down_y][nextMove.down_x] = 'M';
							mapPtr[kart.y][kart.x] = obstacleToReplace;
							kart.x = nextMove.down_x;
							kart.y = nextMove.down_y;
							
							determineAdjacentBlocks();
							isHit = FALSE;
						}
						
					}
					break;
					
				case 6: // 'JOY PAD DOWN ARROW' 
					// Only allowed to move if the block if empty or contains a value pack
					if (mapPtr[nextMove.down_y][nextMove.down_x] == 'P' || mapPtr[nextMove.down_y][nextMove.down_x] == 'X' || mapPtr[nextMove.down_y][nextMove.down_x] == 'Y' || mapPtr[nextMove.down_y][nextMove.down_x] == 'Z' || mapPtr[nextMove.down_y][nextMove.down_x] == 'U' || mapPtr[nextMove.down_y][nextMove.down_x] == 'D' || mapPtr[nextMove.down_y][nextMove.down_x] == 'S' || mapPtr[nextMove.down_y][nextMove.down_x] == 'J' || mapPtr[nextMove.down_y][nextMove.down_x] == 'G' || mapPtr[nextMove.down_y][nextMove.down_x] == 'H' || mapPtr[nextMove.down_y][nextMove.down_x] == 'L' || mapPtr[nextMove.down_y][nextMove.down_x] == '$' || mapPtr[nextMove.down_y][nextMove.down_x] == 'T' || mapPtr[nextMove.down_y][nextMove.down_x] == 'F') {
						
						// When an obstacle is hit. lives = lives - 1;
						if (mapPtr[nextMove.down_y][nextMove.down_x] == 'U' || mapPtr[nextMove.down_y][nextMove.down_x] == 'D' || mapPtr[nextMove.down_y][nextMove.down_x] == 'S' || mapPtr[nextMove.down_y][nextMove.down_x] == 'J' || mapPtr[nextMove.down_y][nextMove.down_x] == 'G' || mapPtr[nextMove.down_y][nextMove.down_x] == 'H') {
							obstacleToReplace = mapPtr[nextMove.down_y][nextMove.down_x];
							isHit = TRUE;
						}
						
						mapPtr[nextMove.down_y][nextMove.down_x] = 'M';
						mapPtr[kart.y][kart.x] = 'P';
						kart.x = nextMove.down_x;
						kart.y = nextMove.down_y;
						
						determineAdjacentBlocks();
						
						if (isHit) {
							drawMap();
							mario.lives--;
							
							mapPtr[nextMove.up_y][nextMove.up_x] = 'M';
							mapPtr[kart.y][kart.x] = obstacleToReplace;
							kart.x = nextMove.up_x;
							kart.y = nextMove.up_y;
							
							determineAdjacentBlocks();
							isHit = FALSE;
						}
					}
					break;
					
				case 7: // 'JOY PAD LEFT ARROW'
					// Only allowed to move if the block if empty or contains a value pack
					if (mapPtr[nextMove.left_y][nextMove.left_x] == 'P' || mapPtr[nextMove.left_y][nextMove.left_x] == 'X' || mapPtr[nextMove.left_y][nextMove.left_x] == 'Y' || mapPtr[nextMove.left_y][nextMove.left_x] == 'Z' || mapPtr[nextMove.left_y][nextMove.left_x] == 'U' || mapPtr[nextMove.left_y][nextMove.left_x] == 'D' || mapPtr[nextMove.left_y][nextMove.left_x] == 'S' || mapPtr[nextMove.left_y][nextMove.left_x] == 'J' || mapPtr[nextMove.left_y][nextMove.left_x] == 'G' || mapPtr[nextMove.left_y][nextMove.left_x]== 'H' || mapPtr[nextMove.left_y][nextMove.left_x] == 'L' || mapPtr[nextMove.left_y][nextMove.left_x] == '$' || mapPtr[nextMove.left_y][nextMove.left_x] == 'T' || mapPtr[nextMove.left_y][nextMove.left_x] == 'F') {
						
						// When an obstacle is hit. lives = lives - 1;
						if (mapPtr[nextMove.left_y][nextMove.left_x] == 'U' || mapPtr[nextMove.left_y][nextMove.left_x] == 'D' || mapPtr[nextMove.left_y][nextMove.left_x] == 'S' || mapPtr[nextMove.left_y][nextMove.left_x] == 'J' || mapPtr[nextMove.left_y][nextMove.left_x] == 'G' || mapPtr[nextMove.left_y][nextMove.left_x]== 'H') {
							obstacleToReplace = mapPtr[nextMove.left_y][nextMove.left_x];
							isHit = TRUE;
						}
						
						mapPtr[nextMove.left_y][nextMove.left_x] = 'M';
						mapPtr[kart.y][kart.x] = 'P';
						kart.x = nextMove.left_x;
						kart.y = nextMove.left_y;
						
						determineAdjacentBlocks();
						
						if (isHit) {
							drawMap();
							mario.lives--;
						
							mapPtr[nextMove.right_y][nextMove.right_x] = 'M';
							mapPtr[kart.y][kart.x] = obstacleToReplace;
							kart.x = nextMove.right_x;
							kart.y = nextMove.right_y;
							
							determineAdjacentBlocks();
							isHit = FALSE;
						}
					}
					break;
					
				case 8: // 'JOY PAD RIGHT ARROW'
					// Only allowed to move if the block if empty or contains a value pack
					if (mapPtr[nextMove.right_y][nextMove.right_x] == 'P' || mapPtr[nextMove.right_y][nextMove.right_x] == 'X' || mapPtr[nextMove.right_y][nextMove.right_x] == 'Y' || mapPtr[nextMove.right_y][nextMove.right_x] == 'Z' || mapPtr[nextMove.right_y][nextMove.right_x] == 'U' || mapPtr[nextMove.right_y][nextMove.right_x] == 'D' || mapPtr[nextMove.right_y][nextMove.right_x] == 'S' || mapPtr[nextMove.right_y][nextMove.right_x] == 'J' || mapPtr[nextMove.right_y][nextMove.right_x] == 'G' || mapPtr[nextMove.right_y][nextMove.right_x] == 'H' || mapPtr[nextMove.right_y][nextMove.right_x] == 'L' || mapPtr[nextMove.right_y][nextMove.right_x] == '$' || mapPtr[nextMove.right_y][nextMove.right_x] == 'T' || mapPtr[nextMove.right_y][nextMove.right_x] == 'F') {
						
						// When an obstacle is hit. lives = lives - 1;
						if (mapPtr[nextMove.right_y][nextMove.right_x] == 'U' || mapPtr[nextMove.right_y][nextMove.right_x] == 'D' || mapPtr[nextMove.right_y][nextMove.right_x] == 'S' || mapPtr[nextMove.right_y][nextMove.right_x] == 'J' || mapPtr[nextMove.right_y][nextMove.right_x] == 'G' || mapPtr[nextMove.right_y][nextMove.right_x] == 'H') {
							obstacleToReplace = mapPtr[nextMove.right_y][nextMove.right_x];
							isHit = TRUE;
						}
						
						mapPtr[nextMove.right_y][nextMove.right_x] = 'M';
						mapPtr[kart.y][kart.x] = 'P';
						kart.x = nextMove.right_x;
						kart.y = nextMove.right_y;
						
						determineAdjacentBlocks();
						
						if (isHit) {
							drawMap();
							mario.lives--;
							
							mapPtr[nextMove.left_y][nextMove.left_x] = 'M';
							mapPtr[kart.y][kart.x] = obstacleToReplace;
							kart.x = nextMove.left_x;
							kart.y = nextMove.left_y;
							
							determineAdjacentBlocks();
							isHit = FALSE;
						}
					}
					break;
		}
		
		// When keys are picked, the doors open. Or if value packs picked, do actions according to the value pack. If mario reaches finsh line, update win flag.
		if (kart.x == 5 && kart.y == 1)
			mapPtr[5][10] = 'P';
		else if (kart.x == 6 && kart.y == 7)
			mapPtr[2][4] = 'P';
		else if (kart.x == 10 && kart.y == 7) {
			mapPtr[4][13] = 'P';
			mapPtr[5][13] = 'P';
		}
		else if (valuePacks.displayed && !heartTaken && kart.x == valuePacks.heart_x && kart.y == valuePacks.heart_y) {
			heartTaken = TRUE;
			mario.lives++;
		}
		else if (valuePacks.displayed && !scoreTaken && kart.x == valuePacks.scoreMultiplier_x && kart.y == valuePacks.scoreMultiplier_y) {
			scoreTaken = TRUE;
			mario.score += 1000;
		}
		else if (valuePacks.displayed && !timeTaken && kart.x == valuePacks.timeMultiplier_x && kart.y == valuePacks.timeMultiplier_y) {
			timeTaken = TRUE;
			mario.timeRemaining += 10;
		}
		else if (kart.x == 14 && kart.y == 1)
			winFlag = TRUE;
		else if (!mario.lives || !mario.timeRemaining) {
			Wait(1000000);
			loseFlag = TRUE;
		}
		
		determineLivesDisplay();
		determineScoreDisplay();
		
		drawMap(mapPtr);
		Wait(100000);
	}
}

// Display the game map
void drawMap() {
	for (int j = 0; j < 9; j++) {
		for (int i = 0; i < 16; i++) {
			drawObject(i, j, mapPtr[j][i]);
		}
	}
}

// find and store the adjacent blocks from the current location of mario
void determineAdjacentBlocks() {
	nextMove.up_x = kart.x;
	nextMove.up_y = kart.y - 1;
	
	nextMove.down_x = kart.x;
	nextMove.down_y = kart.y + 1;
	
	nextMove.right_x = kart.x + 1;
	nextMove.right_y = kart.y;
	
	nextMove.left_x = kart.x - 1;
	nextMove.left_y = kart.y;
}

// Draw the objects, which are 80x80 blocks. It takes 3 parameters. The first two are the x and y cordinate of the location where the object is to be drawn.
// The third parameter is used to determine what kind of object needs to be drawn.
// the objects are the images turned into char using gimp.
void drawObject(int x_cell, int y_cell, char tile_type) {
	int x_start = x_cell * 80;
	int y_start = y_cell * 80;
	int x_end = x_start + 80;
	int y_end = y_start + 80;
	
	short int *pointerToUse;
	if (tile_type == 'M')
		pointerToUse = marioPtr;
	else if (tile_type == 'U')
		pointerToUse = upPtr;
	else if (tile_type == 'D')
		pointerToUse = downPtr;
	else if (tile_type == 'S')
		pointerToUse = leftPtr;
	else if (tile_type == 'J')
		pointerToUse = rightPtr;
	else if (tile_type == 'W')
		pointerToUse = wallPtr;
	else if (tile_type == 'P')
		pointerToUse = pathPtr;
	else if (tile_type == 'G')
		pointerToUse = goombaPtr;
	else if (tile_type == 'F')
		pointerToUse = finishPtr;
	else if (tile_type == 'A')
		pointerToUse = yellowgatePtr;
	else if (tile_type == 'X')
		pointerToUse = yellowkeyPtr;
	else if (tile_type == 'B')
		pointerToUse = bluegatePtr;
	else if (tile_type == 'Y')
		pointerToUse = bluekeyPtr;
	else if (tile_type == 'C')
		pointerToUse = greengatePtr;
	else if (tile_type == 'Z')
		pointerToUse = greenkeyPtr;
	else if (tile_type == 'H')
		pointerToUse = ghostPtr;
	else if (tile_type == 'L')
		pointerToUse = heartPtr;
	else if (tile_type == 'T')
		pointerToUse = timePtr;
	else if (tile_type == '$')
		pointerToUse = scorePtr;
	else if (tile_type == 'K')
		pointerToUse = gameWonPtr;
	else if (tile_type == 'Q')
		pointerToUse = gameLostPtr;
	else if (tile_type == 'V')
		pointerToUse = livPtr;
	else if (tile_type == 'E')
		pointerToUse = esPtr;
	else if (tile_type == 'I')
		pointerToUse = tiPtr;
	else if (tile_type == 'N')
		pointerToUse = mePtr;
	else if (tile_type == 'O')
		pointerToUse = scoPtr;
	else if (tile_type == 'R')
		pointerToUse = rePtr;
	else if (tile_type == '0')
		pointerToUse = zeroPtr;
	else if (tile_type == '1')
		pointerToUse = onePtr;
	else if (tile_type == '2')
		pointerToUse = twoPtr;
	else if (tile_type == '3')
		pointerToUse = threePtr;
	else if (tile_type == '4')
		pointerToUse = fourPtr;
	else if (tile_type == '5')
		pointerToUse = fivePtr;
	else if (tile_type == '6')
		pointerToUse = sixPtr;
	else if (tile_type == '7')
		pointerToUse = sevenPtr;
	else if (tile_type == '8')
		pointerToUse = eightPtr;
	else if (tile_type == '9')
		pointerToUse = ninePtr;
	
	int i = 0;
	for (int y = y_start; y < y_end; y++) {
		for (int x = x_start; x < x_end; x++) {
				
				pixel->color = pointerToUse[i];
				pixel->x = x;
				pixel->y = y;
	
				drawPixel(pixel);
				i++;
		}
	}
}

// the SNES initialization function
void Init_SNES() {
	Init_GPIO(9, 1); // initializing pin 9 (LATCH) to output
	Init_GPIO(11, 1); // initializing pin 11 (CLK) to output
	Init_GPIO(10, 0); // initializing pin 10 (DATA) to input
}

// this function is used to initialize the pins (9, 10, and 11). It takes the pointer, pin number and the function code as parameters
void Init_GPIO(int pin_number, int function_mode) {
	
	*(gpioPtr + ((pin_number)/10)) &= ~(7 << (((pin_number)%10)*3));
	
	if (function_mode)
		*(gpioPtr + ((pin_number)/10)) |= (1 << (((pin_number)%10)*3));
		
}

// This function is used to change the function code of pin 9 (LATCH). It takes the pointer and the function code as parameters.
void Write_Latch(int function_mode) {
	
	if (function_mode)
		gpioPtr[GPSET0] = 1 << 9; // writing 1
	else gpioPtr[GPCLR0] = 1 << 9; // writing 0
}

// This function is used to change the function code of pin 11 (CLK). It takes the pointer and the function code as parameters.
void Write_Clock(int function_mode) {
	
	if (function_mode)
		gpioPtr[GPSET0] = 1 << 11; // writing 1
	else gpioPtr[GPCLR0] = 1 << 11; // writing 0
}

// This function is used to read pin 10 (DATA). It takes the pointer as a parameter and return the integer value of the data
int Read_Data() {
	return (gpioPtr[GPLEV0] >> 10) & 1;
}

// function to wait/delay
void Wait(int wait_time) {
	delayMicroseconds(wait_time);
}

// The main function that reads input (buttons pressed) from a SNES controller. Returns the code of the pressed button.
int Read_SNES() {
	while (TRUE) {
		
		if (loseFlag || !mario.timeRemaining)
			return 0;
		
		Write_Clock(1); // set the function code of CLOCK to 1
		Write_Latch(1); // set the function code of LATCH to 1
		Wait(12); // wait for 12 microseconds
		Write_Latch(0); // set the function code of LATCH to 0
		
		int button_pressed;
		for (int i = 1; i <= 16; i++) {
			Wait(6); // wait for 6 microseconds
			Write_Clock(0); // set the function code of CLOCK to 0
			Wait(6); // wait for 6 microseconds
			button_pressed = Read_Data(); // store the return value of the function 'Read_Data' to a variable
			Write_Clock(1); // set the function code of CLOCK to 1
			
			if (button_pressed == 0) // when any button is pressed, then return the value of i
				return i;
		}
	}
}
