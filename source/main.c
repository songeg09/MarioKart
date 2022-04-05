// Author: Duong Tran (30113765) and Eungyo Song (30079379)
// CPSC 359 - Assignment 4 

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "framebuffer.h"

#include <unistd.h>
#include <wiringPi.h>
#include "initGPIO.h"

#include <pthread.h>

#include <time.h>
// the followings are the graphic image files in C code
#include "OpeningBackground.h"
#include "MainMenu.h"
#include "Arrow.h"

#include "Coin.h"
#include "Heart.h"
#include "Mario.h"

#include "TIME.h"
#include "SCORE.h"
#include "Finish.h"

#include "ZERO.h"
#include "ONE.h"
#include "TWO.h"
#include "THREE.h"
#include "FOUR.h"
#include "FIVE.h"
#include "SIX.h"
#include "SEVEN.h"
#include "EIGHT.h"
#include "NINE.h"

#include "Clock.h"
#include "Rocket.h"
#include "GreenShell.h"
#include "smallHeart.h"

#include "StageOne.h"
#include "StageTwo.h"
#include "StageThree.h"
#include "StageFour.h"

#include "Map1.h"
#include "Map2.h"
#include "Map3.h"
#include "Map4.h"

#include "LOST.h"
#include "WIN.h"
#include "FINALSCORE.h"

#include "PAUSE.h"


//////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                              //
//                                        SNESS CONTROLLER                                      //
//                                                                                              //
//////////////////////////////////////////////////////////////////////////////////////////////////

// Input (000)
#define INP_GPIO(p, gpioPtr) *(gpioPtr+((p)/10)) &= ~(7<<(((p)%10)*3))     
// Output (001)
#define OUT_GPIO(p, gpioPtr) *(gpioPtr+((p)/10)) |= (1<<(((p)%10)*3))    

#define CLK       11
#define LAT       9
#define DAT       10

#define GPSET0  7       // 28/4
#define GPCLR0  10      // 40/4
#define GPLEV0  13      // 52/4



// function to init the gpio pin according to its function
void Init_GPIO(int lineNumber, int functionCode, unsigned int *gpioPtr) {
    if (functionCode == 0) {                // set input
        INP_GPIO(lineNumber, gpioPtr);
    } else if (functionCode == 1) {         // set output
        INP_GPIO(lineNumber, gpioPtr);      // always call INP before OUT
        OUT_GPIO(lineNumber, gpioPtr);
    }else{
        printf("Function code must be 0 or 1.");
    }
}

// function to write on pin 9 (LATCH)
void Write_Latch(int n, unsigned int *gpioPtr) { 
    if (n == 1) {
        gpioPtr[GPSET0] = 1 << LAT;         // write 1
    } else {
        gpioPtr[GPCLR0] = 1 << LAT;         // clear
    }
}

// function to write on pin 11 (CLOCK)
void Write_Clock(int n, unsigned int *gpioPtr) { 
    if (n == 1) {
        gpioPtr[GPSET0] = 1 << CLK;         // write 1
    } else {
        gpioPtr[GPCLR0] = 1 << CLK;         // clear
    }
}


// function to Read Data in pin 10 (DAT)
unsigned int Read_Data(unsigned int *gpioPtr) {
    return (gpioPtr[GPLEV0] >> DAT) & 1;             // change pin 3 to 10. read DATA
}

// create the struct for the player with all attributes
struct player {
    int current_x;          // store current x coordinate 
    int current_y;          // store current y coordinate 

    int previous_x;         // store previous x coordinate 
    int previous_y;         // store previous y coordinate 

    int lives;              // count live of player has left

    int score;              // current score of when play
    int total_score;        // score when win / lost

    int timeleft;           // timeleft after finishing each stage

    int time;               // time countdown when enter each stage

    int distance;           // countdown to reach finish line

    int stage;              // check current stage

    int winflag;            
    int loseflag;

    int game_on;            // when still playing
    int gamepause;          // pause game flag
    int restart;            

    int finish_x;
};
struct player p;            // create struct player p

// function to print message declaring which button is pressed
void ApplyChange(int cycle, int *c_x, int *c_y, int *p_x, int *p_y) {
    *p_y = *c_y;    // previous x = current x
    *p_x = *c_x;    // previous y = current y
    switch (cycle)
    {
    case 1:
        break;
    case 2:
        break;
    case 3:
        break;
    case 4:                                     // START
        p.gamepause = 1;            // flag if someone pause during game
        break;                               
    case 5:                                    // UP
        if (*c_y - 64 >= 320){      // upper bound of screen for mario
		    *c_y = *c_y - 64;       // move 1 grid up at time (using 64x64 grid)
            break;
        }else{
            break;
        }
    case 6:                                    // DOWN
        if (*c_y + 128 <= 720){     // lower bound for mario movement
		    *c_y = *c_y + 64;       // move 1 grid down at time (using 64x64 grid)
            break;
        }else{
            break;
        }
    case 7:                                    // LEFT
        if (*c_x - 64 >= 0){        // left bound for mario movement
		    *c_x = *c_x - 64;       // move 1 grid left at time (using 64x64 grid)
            break;
        }else{
            break;
        }
    case 8:                                    // RIGHT
        if (*c_x + 128 <= 1280){    // right bound for mario movement
		    *c_x = *c_x + 64;       // move 1 grid right at time (using 64x64 grid)
            break;
        }else{
            break;
        }
    case 9:                                   // A
        break;
    case 10:
        break;
    case 11:
        break;
    case 12:
        break;    
    }
}

int buttons[16];        // array for 16 buttons of snes

// Read_SNES
void Read_SNES(unsigned int *gpioPtr) {
    // MOV button, #0   
    for(int i=0; i<16; i++) {       // reset buttons
        buttons[i] = 1;
    }
    Write_Latch(1, gpioPtr);        // set pin 9
    delayMicroseconds(12);          // wait 12ms
    Write_Latch(0, gpioPtr);        // clear pin 9
    int cycle = 1;                  // 1st button
    int pin_val;
    
    while (cycle <= 16) {               // pulse Loop
        
        delayMicroseconds(6);
        Write_Clock(0, gpioPtr);        // falling edge         clear gpio 11

        pin_val = Read_Data(gpioPtr);   // read value of gpio 10,   0 means pressed, 1 means not
        if (pin_val == 0 && cycle < 13) {
            buttons[cycle-1] = 0;
        } 
        delayMicroseconds(6);
        Write_Clock(1, gpioPtr);        // rising edge, new cycle       set gpio 11
        cycle++;                        // next button
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                              //
//                                        SNESS CONTROLLER                                      //
//                                                                                              //
//////////////////////////////////////////////////////////////////////////////////////////////////
void startGame();


int i;
int width = 1280;       // screen size
int height = 720;

/* Definitions */
typedef struct {        // struct contain pixel data to draw on screeen
	short int color;
	int x, y;
} Pixel;

struct fbs framebufferstruct;
void drawPixel(Pixel *pixel);


// OPENING SCREEN when boot up the program
void opening(unsigned int *gpioPtr, Pixel *pixel){

    short int *OpeningBackgroundPtr=(short int *) OpeningBackgroundImage.pixel_data;    // create array of pixel data for Opening screen
    i = 0;

     for (int y = 0; y < height; y++)                   // draw entire screen, so y = 0 to height-1
		for (int x = 0; x < width; x++)                 // draw entire screeen, x = 0 to width-1
		{	
			pixel->color = OpeningBackgroundPtr[i];     // set color for pixel
			pixel->x = x;                               // set x coordinate
			pixel->y = y;                               // set y coordinate
	
			drawPixel(pixel);                           // draw that pixel
            i++;
		}
	}

    do{
        Read_SNES(gpioPtr);                             // cont to read SNES input
    }while (buttons[3]==1);                             // while start is not pressed yet (wait for START to press)
}

// draw MAIN MENU screen and link to 2 options: play or quit
void mainMenu(unsigned int *gpioPtr, Pixel *pixel){
    short int *MainMenuPtr=(short int *) MainMenuImage.pixel_data;  // create array of pixel data for Main menu screen
    i = 0;

    for (int y = 0; y < height; y++)                                // draw entire screen, so y = 0 to height-1
	{
		for (int x = 0; x < width; x++)                             // draw entire screeen, x = 0 to width-1
		{	
			pixel->color = MainMenuPtr[i];                          // set color for pixel from the main menu pixel array
			pixel->x = x;                                           // set x coordinate
			pixel->y = y;                                           // set y coordinate
	
			drawPixel(pixel);                                       // draw that pixel
            i++;
		}
	}
    
    delayMicroseconds(750000);

    short int *ArrowPtr=(short int *) ArrowImage.pixel_data;        // create array of pixel data for the red arrow on Main menu screen
    int cursor = 0;                                                 // cursor is on play button by default

    while(1){
        i = 0;
        
        // when cursor is on play
        if (cursor == 0){
            // Fixing the background
            for (int y = 520; y < 584; y++){                        // fill the pixels of arrow point to Quit button by the background
			    for (int x = 460; x < 524; x++) {
                
                    pixel->color = MainMenuPtr[y*width+x];          // get color at that specific pixel in MainMenu image file
				    pixel->x = x;
				    pixel->y = y;
		
				    drawPixel(pixel);
                    i++;
			    }
		    }

            i = 0;
            // Draw the arrow
            for (int y = 370; y < 434; y++){                        // draw the arrow point to Play button
			    for (int x = 460; x < 524; x++) {
                
                    if (ArrowPtr[i] == 0x000){                      // for all black pixel in ArrowPtr, fill it with MainMenu color
                        pixel->color = MainMenuPtr[y*width+x];      // get color at that specific pixel in MainMenu image file
                    }else{
                        pixel->color = ArrowPtr[i];                 // get color of the arrow
                    } 
				    pixel->x = x;
				    pixel->y = y;
		
				    drawPixel(pixel);                               // draw arrow
                    i++;
			    }
		    }
         
        // when cursor is on quit button
        }else if (cursor == 1){
            // Fixing the background    
            for (int y = 370; y < 434; y++){                        // fix the arrow when arrow was point to Play
			    for (int x = 460; x < 524; x++) {   
                
                    pixel->color = MainMenuPtr[y*width+x];          // redraw that part with color from MainMenu
				    pixel->x = x;
				    pixel->y = y;
		
				    drawPixel(pixel);                               // draw the pixel
                    i++;
			    }
		
		    }

            i = 0;

            // Draw the arrow
            for (int y = 520; y < 584; y++){                        // select the area where arrow point to Quit button
			    for (int x = 460; x < 524; x++) {
                
                    if (ArrowPtr[i] == 0x000){                      // if pixel was black
                        pixel->color = MainMenuPtr[y*width+x];      // get color from main menu
                    }else{
                        pixel->color = ArrowPtr[i];                 // get arrow color
                    } 
				    pixel->x = x;
				    pixel->y = y;
		
				    drawPixel(pixel);                               // draw arrow
                    i++;
			    }
		
		    }
        }

        Read_SNES(gpioPtr);                             // read snes input
        if (cursor == 0){                               // if arrow is at Play        
            if (buttons[5] == 0){                       // if button Down is pressed
                cursor = 1;                             // arrow point to Quit button
            }else if (buttons[8] == 0){                 // if button A is pressed
                startGame();                            // start the game
                break;
            }
        
        } else if (cursor == 1){                         // if arrow is at Quit
            if (buttons[4] == 0){                       // if button UP is pressed
                cursor = 0;                             // arrow now points to Play
            } else if (buttons[8] == 0){                 // if A is pressed
                for (int y = 0; y < height; y++){       // draw the entire screen
			        for (int x = 0; x < width; x++) {
                
                        pixel->color = 0x000;           // mask every pixel with black, 
				        pixel->x = x;
				        pixel->y = y;
		
				        drawPixel(pixel);               // draw black screen
			        }
		        }
                exit(0);                                // Terminate the program
            }
        }
    }

}


// Draw Stage for each screen 
void DrawingStage(unsigned int *gpioPtr, Pixel *pixel){

    if (p.stage == 1){          // check current stage = 1
        short int *StageOnePtr=(short int *) StageOneImage.pixel_data;      // get pixel data of stage 1
        i = 0;

        for (int y = 0; y < height; y++)                     // select entire screen to draw
        {
            for (int x = 0; x < width; x++) 
            {	
                pixel->color = StageOnePtr[i];              // get color of that pixel in stage 1 
                pixel->x = x;
                pixel->y = y;
        
                drawPixel(pixel);                           // draw that pixel
                i++;                                        // increment pixel index
            }
        }
    }else if (p.stage == 2){                                // if current stage = 2
        short int *StageTwoPtr=(short int *) StageTwoImage.pixel_data;      // get pixel data of stage 2
        i = 0;

        for (int y = 0; y < height; y++)                    // select entire screen to draw
        {
            for (int x = 0; x < width; x++) 
            {	
                pixel->color = StageTwoPtr[i];              // get color of that pixel in stage 2
                pixel->x = x;
                pixel->y = y;
        
                drawPixel(pixel);                           // draw that pixel
                i++;                                        // increment pixel index
            }
        }
    }else if (p.stage == 3){                                // if current stage = 3
        short int *StageThreePtr=(short int *) StageThreeImage.pixel_data;      // get pixel data of stage 3
        i = 0;

        for (int y = 0; y < height; y++)                    // select entire screen to draw
        {
            for (int x = 0; x < width; x++) 
            {	
                pixel->color = StageThreePtr[i];            // get color of that pixel in stage 3
                pixel->x = x;
                pixel->y = y;
        
                drawPixel(pixel);                           // draw that pixel
                i++;                                        // increment pixel index
            }
        }
    }else{                                                  // if current stage = 3
        short int *StageFourPtr=(short int *) StageFourImage.pixel_data;        // get pixel data of stage 4
        i = 0;

        for (int y = 0; y < height; y++)                    // select entire screen to draw
        {
            for (int x = 0; x < width; x++) 
            {	
                pixel->color = StageFourPtr[i];             // get color of that pixel in stage 4
                pixel->x = x;
                pixel->y = y;
        
                drawPixel(pixel);                           // draw that pixel
                i++;                                        // increment pixel index
            }
        }
    }

    sleep(2);
}



// Draw Finish Line that end current stage and lead to next stage
void DrawingFinish(unsigned int *gpioPtr, Pixel *pixel){
    short int *MapPtr;
    if (p.stage == 1){
        MapPtr=(short int *) Map1Image.pixel_data;                  // get pixel data of map 1
    }else if(p.stage == 2){
        MapPtr=(short int *) Map2Image.pixel_data;                  // get pixel data of map 2
    }else if(p.stage ==3){
        MapPtr=(short int *) Map3Image.pixel_data;                  // get pixel data of map 3
    }else{
        MapPtr=(short int *) Map4Image.pixel_data;                  // get pixel data of map 4
    }

    short int *FinishPtr=(short int *) FinishImage.pixel_data;      // get pixel data of finish line

    p.finish_x = 21;    // Preventing printing black square when objects are coming out
    int w = 0;

    while (p.game_on == 1){

        if (p.distance == 0){
            p.finish_x = 19;
            /*	Fixing Background	*/
            while(p.game_on == 1){  
                while(p.gamepause == 1) {           // if pause
                    if (p.restart == 1){            // if restart
                        return;
                    }
                }
                for (int y = 320; y < 704; y++ )        // select the height for non-moving background
                {
                    for (int x = (p.finish_x+1)*64 ; x < (p.finish_x+1)*64 + 64; x++)  // select width for non-moving background
                    {
                        pixel->color = MapPtr[y*width+x];           // get color of that pixel on Map
                        pixel->x = x;
                        pixel->y = y;
                        drawPixel(pixel);                           // draw that pixel
                    }
                }
                
                w = 0;
                for (int y = 320; y < 704; y++ )                // finish line has the height = height of non-moving background 
                {
                    for (int x = (p.finish_x)*64 ; x < (p.finish_x)*64 + 64; x++)   // finish line width = non-moving background
                    {   
                        pixel->color = FinishPtr[w];                // get color of that pixel in FinishLine image
                        pixel->x = x;
                        pixel->y = y;
                        drawPixel(pixel);                           // draw that pixel
                        w++;                                        // increment finish line pixel index
                    }
                }

                if (p.finish_x == -1){                              // finish line not go over the screen
                    break;
                }

                if (p.current_x == p.finish_x*64){                  // if mario hit finish line
                    p.game_on = 0;                                  // end the stage
                    break;
                }
                
                if (p.stage == 1){
                    for(int check = 0; check < 170000; check ++){   // moving speed of uppper moving screen in stage 1
                        delayMicroseconds(1);
                        if (p.current_x == p.finish_x*64){          // if mario hit finish line
                            p.game_on = 0;                          // end the stage
                            break;
                        }
                    }
                    if (p.current_x == p.finish_x*64){              // if mario hit finish line
                        break;
                    }
                }else if (p.stage ==2 ){
                    for(int check = 0; check < 150000; check ++){   // moving speed of uppper moving screen in stage 2
                        delayMicroseconds(1);
                        if (p.current_x == p.finish_x*64){          // if mario hit finish line
                            p.game_on = 0;                          // end the stage
                            break;
                        }
                    }
                    if (p.current_x == p.finish_x*64){              // if mario hit finish line
                        break;
                    }
                }else if (p.stage == 3){
                    for(int check = 0; check < 130000; check ++){   // moving speed of uppper moving screen in stage 3
                        delayMicroseconds(1);
                        if (p.current_x == p.finish_x*64){          // if mario hit finish line
                            p.game_on = 0;                          // end the stage
                            break;
                        }
                    }
                    if (p.current_x == p.finish_x*64){              // if mario hit finish line
                        break;
                    }
                }else{
                    for(int check = 0; check < 100000; check ++){   // moving speed of uppper moving screen in stage 4
                        delayMicroseconds(1);
                        if (p.current_x == p.finish_x*64){          // if mario hit finish line
                            p.game_on = 0;                          // end the stage
                            break;
                        }
                    }
                    if (p.current_x == p.finish_x*64){              // if mario hit finish line
                        break;
                    }
                }
                
                p.finish_x = p.finish_x - 1;                        // finish line move left, decrease 1 grid at time
            }
        }
    }
}

// thread function running the finish line
void *runFinish(void *unused){
    
    unsigned int *gpioPtr = getGPIOPtr();
    Init_GPIO(CLK, 1, gpioPtr);         // init pin 11 to output
    Init_GPIO(LAT, 1, gpioPtr);         // init pin 9 to output
    Init_GPIO(DAT, 0, gpioPtr);

    /* initialize + get FBS */
    framebufferstruct = initFbInfo();
    
    /* initialize a pixel */
    Pixel *pixel;
    pixel = malloc(sizeof(Pixel));

    DrawingFinish(gpioPtr, pixel);  
    
    pthread_exit(0);                    // exit thread after finish the drawingFinish fct
}



// Draw the non moving Map (lower background)
void DrawingMap(unsigned int *gpioPtr, Pixel *pixel){
    
    short int *MapPtr;
    if (p.stage == 1){
        MapPtr=(short int *) Map1Image.pixel_data;      // get pixel data of map 1
    }else if(p.stage == 2){
        MapPtr=(short int *) Map2Image.pixel_data;      // get pixel data of map 2
    }else if(p.stage ==3){
        MapPtr=(short int *) Map3Image.pixel_data;      // get pixel data of map 3
    }else{
        MapPtr=(short int *) Map4Image.pixel_data;      // get pixel data of map 4
    }

    int w = 0;

    for (int y = 320; y < 720; y++) {                   // for the area of lower background
        for (int x = 0; x < 1280; x++){
            pixel->color = MapPtr[y*width+x];           // get color of the pixel in map
            pixel->x = x;                               // get coordinate
            pixel->y = y;

            drawPixel(pixel);                           // draw pixel
        }
    }

    while (p.game_on == 1){

        while(p.gamepause == 1) {                       // if pause
            if (p.restart == 1){                        // if resume
                return;
            }
        }
        /*	Back ground	*/
        for (int y = 64; y < 320; y++)                  // area of upper moving background
        {
            for (int j = 0; j < 20; j++) 
            {
                for (int x = 0; x < 64; x++){
                    pixel->color = MapPtr[y*width+(j*64)+x-(19*64)+(w*64)];     // get color of that pixel in moving background
                    pixel->x = x+(j*64);                                        // get coordinate
                    pixel->y = y;

                    drawPixel(pixel);                                           // draw pixel
                }
            }
        }
        if (w == 19){                           // use w to make background moving
            w = 0;                              // reset w when reach another side of the screen
        }else{
            w++;
        }

        
        if (p.stage == 1){
            delayMicroseconds(170000);          // set moving speed for upper background in stage 1
        }else if (p.stage ==2 ){
            delayMicroseconds(150000);          // set moving speed for upper background in stage 2
        }else if (p.stage == 3){
            delayMicroseconds(130000);          // set moving speed for upper background in stage 3
        }else{
            delayMicroseconds(100000);          // set moving speed for upper background in stage 4
        }
    }
    
}


// thread function to run Map
void *runMap(void *unused){
    
    unsigned int *gpioPtr = getGPIOPtr();
    Init_GPIO(CLK, 1, gpioPtr);         // init pin 11 to output
    Init_GPIO(LAT, 1, gpioPtr);         // init pin 9 to output
    Init_GPIO(DAT, 0, gpioPtr);

    /* initialize + get FBS */
    framebufferstruct = initFbInfo();
    
    /* initialize a pixel */
    Pixel *pixel;
    pixel = malloc(sizeof(Pixel));

    delayMicroseconds(30000);

    DrawingMap(gpioPtr, pixel);    
    
    pthread_exit(0);                // exit thread when finish
}


// draw resume screen
void DrawingPauseScreen(unsigned int *gpioPtr, Pixel *pixel){
    short int *MapPtr;
    if (p.stage == 1){
        MapPtr=(short int *) Map1Image.pixel_data;          // get pixel data of map 1
    }else if(p.stage == 2){
        MapPtr=(short int *) Map2Image.pixel_data;          // get pixel data of map 2
    }else if(p.stage ==3){
        MapPtr=(short int *) Map3Image.pixel_data;          // get pixel data of map 3
    }else{
        MapPtr=(short int *) Map4Image.pixel_data;          // get pixel data of map 4
    }

    short int *PAUSEPtr=(short int *) PAUSEImage.pixel_data;    // get pixel data of PAUSE screen

    while (p.game_on == 1){   

        if (p.gamepause == 1) {
            delayMicroseconds(100000);
            i = 0;

            for (int y = 0; y < height; y++)            // select entire screen to draw
            {
                for (int x = 0; x < width; x++) 
                {	
                    pixel->color = PAUSEPtr[i];         // get color of that pixel on PAUSE
                    pixel->x = x;                       // get coordinate
                    pixel->y = y;
            
                    drawPixel(pixel);                   // draw pixel
                    i++;                                // increment index of pixel in PAUSE
                }
            }
            short int *ArrowPtr=(short int *) ArrowImage.pixel_data;    // get pixel data of ARROW image
            int cursor = 0;                                             // curosr is on play by default

            while(p.gamepause == 1){
                i = 0;
                
                // when cursor is on play
                if (cursor == 0){
                    // Fixing the background
                    for (int y = 430; y < 494; y++){                    // select area when arrow point to QUIT
                        for (int x = 490; x < 554; x++) {
                        
                            pixel->color = PAUSEPtr[y*width+x];         // get color of pixel on PAUSE
                            pixel->x = x;                               // set coordinate
                            pixel->y = y;
                
                            drawPixel(pixel);                           // draw pixel
                            i++;                                        // increment index of pixel in PAUSE
                        }
                    }

                    i = 0;
                    // Draw the arrow
                    for (int y = 300; y < 364; y++){                    // select area when arrow point to PLAY
                        for (int x = 440; x < 504; x++) {
                        
                            if (ArrowPtr[i] == 0x000){                  // if pixel on ARROW is black
                                pixel->color = PAUSEPtr[y*width+x];     // get color in PAUSE image
                            }else{
                                pixel->color = ArrowPtr[i];             // get color of arrow
                            } 
                            pixel->x = x;                               // set coordinate  
                            pixel->y = y;
                
                            drawPixel(pixel);                           // draw pixel
                            i++;                                        // increment index of pixel in ARROW
                        }
                
                    }
                
                // when cursor is on quit button
                }else if (cursor == 1){
                    // Fixing the background
                    for (int y = 300; y < 364; y++){                // select area when arrow point to PLAY
                        for (int x = 440; x < 504; x++) {
                        
                            pixel->color = PAUSEPtr[y*width+x];     // get color of pixel on PAUSE
                            pixel->x = x;                           // set coordinate  
                            pixel->y = y;
                
                            drawPixel(pixel);                       // draw pixel
                            i++;
                        }
                
                    }

                    i = 0;

                    // Draw the arrow
                    for (int y = 430; y < 494; y++){                // select area when arrow point to QUUT
                        for (int x = 490; x < 554; x++) {
                        
                            if (ArrowPtr[i] == 0x000){              // if pixel on ARROW is black
                                pixel->color = PAUSEPtr[y*width+x];  // get color in PAUSE image
                            }else{
                                pixel->color = ArrowPtr[i];         // get color of arrow
                            } 
                            pixel->x = x;                           // set coordinate
                            pixel->y = y;
                
                            drawPixel(pixel);                       // draw pixel
                            i++;                                    // increment index of pixel in ARROW
                        }
                    }
                }

                Read_SNES(gpioPtr);                                 // read input
                if (buttons[3] == 0){                               // if start is pressed
                    sleep(1);
                    for (int y = 320; y < 720; y++) {               // select lower background area
                        for (int x = 0; x < 1280; x++){
                            pixel->color = MapPtr[y*width+x];       // get color of lower background
                            pixel->x = x;
                            pixel->y = y;                           // set coordinate

                            drawPixel(pixel);                       // draw pixel
                        }
                    }
                    p.gamepause = 0;
                    break;
                }
                if (cursor == 0){                                   // if arrow at PLAY
                    if (buttons[5] == 0){                           // if DOWN pressed
                        cursor = 1;                                 // arrow at QUIT
                    }else if (buttons[8] == 0){                     // if A is pressed
                        p.game_on = 0;                              // reset flag
                        p.restart = 1;
                        p.gamepause = 0;
                        return;
                    }
                
                }else if (cursor == 1){                             // if arrow at QUIT
                    if (buttons[4] == 0){                           // if UP is pressed
                        cursor = 0;                                 // arrow at PLAY
                    }else if (buttons[8] == 0){                     // if A is pressed
                        p.loseflag = 1;                             // set flag
                        p.gamepause = 0;
                        p.game_on = 0;                              // 
                        break;
                    }
                }

                delayMicroseconds(75000);
            }
        }
    }
    
}

// thread function to run Pause screen
void *runPause(void *unused) {
    unsigned int *gpioPtr = getGPIOPtr();
    Init_GPIO(CLK, 1, gpioPtr);         // init pin 11 to output
    Init_GPIO(LAT, 1, gpioPtr);         // init pin 9 to output
    Init_GPIO(DAT, 0, gpioPtr);

    /* initialize + get FBS */
    framebufferstruct = initFbInfo();
    
    /* initialize a pixel */
    Pixel *pixel;
    pixel = malloc(sizeof(Pixel));

    DrawingPauseScreen(gpioPtr, pixel);   
    
    pthread_exit(0);                    // exit thread when finish
}


// Draw Heart to display current lives
void DrawingHeart(unsigned int *gpioPtr, Pixel *pixel){

    short int *HeartPtr=(short int *) HeartImage.pixel_data;        // get pixel data of Heart
    int w;
  
    while(p.game_on == 1){    
        
        while(p.gamepause == 1){                // if pause
            if (p.restart == 1){                // if restart
                return;
            }
        }
        
        for (int j = 0; j < 5; j++){            // max lives  = 5

            if(j < p.lives){
                
                w = 0;
                for (int y = 0; y < 64; y++){           // get area to draw heart
                    for (int x = 0; x < 64; x++) {
                    
                        pixel->color = HeartPtr[w];     // get color of pixel on Heart image
                        pixel->x = x+(j*64);            // get coordinate
                        pixel->y = y;
                
                        drawPixel(pixel);               // draw 
                        w++;                            // increment index of pixel in HEart
                    }
            
                }

            }else if (j >= p.lives){                    // if more than current lives
                
                for (int y = 0; y < 64; y++){           // get area to draw heart
                    for (int x = 0; x < 64; x++) {
                    
                        pixel->color = 0x000;           // draw black pixel instead of heart
                        pixel->x = x+(j*64);            // get coordinate
                        pixel->y = y;
                
                        drawPixel(pixel);
                    }
            
                }
            }
        }

        if (p.lives == 0){
            for (int j = 0; j < 5; j++){                // get area to draw heart
                for (int y = 0; y < 64; y++){
                    for (int x = 0; x < 64; x++) {
                    
                        pixel->color = 0x000;           // draw black pixel instead of heart
                        pixel->x = x+(j*64);            // get coordinate
                        pixel->y = y;
                
                        drawPixel(pixel);
                    }
            
                }
            }
            p.loseflag = 1;         // live = 0, then lose
            p.game_on = 0;          // stop game
        }

        while(p.gamepause == 1) {}
        
    }
}

// thread function to keep draw lives display
void *runHeart(void *unused){
    
    unsigned int *gpioPtr = getGPIOPtr();
    Init_GPIO(CLK, 1, gpioPtr);         // init pin 11 to output
    Init_GPIO(LAT, 1, gpioPtr);         // init pin 9 to output
    Init_GPIO(DAT, 0, gpioPtr);

    /* initialize + get FBS */
    framebufferstruct = initFbInfo();
    
    /* initialize a pixel */
    Pixel *pixel;
    pixel = malloc(sizeof(Pixel));

    DrawingHeart(gpioPtr, pixel);    
    
    pthread_exit(0);                    // exit thread when finish
}




// Mario
void DrawingMario(unsigned int *gpioPtr, Pixel *pixel){
    short int *MapPtr;
    if (p.stage == 1){
        MapPtr=(short int *) Map1Image.pixel_data;          // get pixel data of map 1
    }else if(p.stage == 2){
        MapPtr=(short int *) Map2Image.pixel_data;          // get pixel data of map 2
    }else if(p.stage ==3){
        MapPtr=(short int *) Map3Image.pixel_data;          // get pixel data of map 3
    }else{
        MapPtr=(short int *) Map4Image.pixel_data;          // get pixel data of map 4
    }
    short int *MarioPtr=(short int *) MarioImage.pixel_data;    // get pixel data of mario
    short int *FinishPtr=(short int *) FinishImage.pixel_data;  // get pixel data of finish line
    
    int w;

    /* currenct location */
	p.current_x = 192;          // default location when start game
	p.current_y = 448;

    p.previous_x = p.current_x;     // save coordinate
    p.previous_y = p.current_y;
    
    while (p.game_on == 1){    

        while(p.gamepause == 1) {       // if pause
            if (p.restart == 1){        // if restart
                return;
            }
        }
        w = 0;

        Read_SNES(gpioPtr);             // read input
        for (w = 0; w < 16; w++) {
            if (buttons[w] == 0) {      // if button is pressed
                ApplyChange(w+1, &p.current_x, &p.current_y, &p.previous_x, &p.previous_y);     // move mario
                break;
            }	
        }
        
        w = 0;
        /*	Fixing Background	*/
        for (int y = p.previous_y; y < p.previous_y + 64; y++)          // draw previous location
        {
            for (int x = p.previous_x; x < p.previous_x + 64; x++) 
            {
                if (p.previous_x == p.finish_x*64){                     // if previous location was on finish line
                    pixel->color = FinishPtr[w];                        // get color of finish line
                }else{
                    pixel->color = MapPtr[y*width+x];                   // get color of lower map
                }
                pixel->x = x;                                           // get previous coordinate
                pixel->y = y;
        
                drawPixel(pixel);                                       // draw pixel
                w++;                                                    // increment index of finish line
            }
        }

        w = 0;
        /*	Mario location	*/
        for (int y = p.current_y; y < p.current_y + 64; y++)            // draw current location
        {
            for (int x = p.current_x; x < p.current_x + 64; x++) 
            {
                if (p.current_x == p.finish_x*64){                      // if mario hit finished line
                    if (MarioPtr[w] == 0x000){                          // if the pixel on mario is black
                        pixel->color = FinishPtr[w];                    // get color from finish line 
                    }else{
                        pixel->color = MarioPtr[w];                     // get color of mario
                    } 

                }else{                                                  // if mario on road
                    if (MarioPtr[w] == 0x000){                          // if the pixel on mario is black
                        pixel->color = MapPtr[y*width+x];               // get color of road (lower backgroudn)
                    }else{
                        pixel->color = MarioPtr[w];                     // get color of mario
                    } 
                    
                }
                pixel->x = x;                                           // get coordinate
                pixel->y = y;
        
                drawPixel(pixel);                                       // draw 
                w++;                                                    // increment index of finish line and mario
            }
        
        }
        delayMicroseconds(120000);
       
    }
    w = 0;
    for (int y = p.current_y; y < p.current_y + 64; y++)                // draw current location
    {
        for (int x = p.current_x; x < p.current_x + 64; x++) 
        {
            if (p.current_x == p.finish_x*64){                          // if mario hit finish line
                if (MarioPtr[w] == 0x000){                              // if mario pixel is black
                    pixel->color = FinishPtr[w];                        // get color of finish line
                }else{
                    pixel->color = MarioPtr[w];                         // get color of mario
                } 
                
            }else{                                                      // if mario on road
                if (MarioPtr[w] == 0x000){                              //if the pixel on mario is black
                    pixel->color = MapPtr[y*width+x];                   // get color of road (lower backgroudn)
                }else{
                    pixel->color = MarioPtr[w];                         // get color of mario
                } 
                
            }
            pixel->x = x;                                               // get coordinate
            pixel->y = y;
    
            drawPixel(pixel);                                           // draw
            w++;                                                        // increment index of finish line and mario
        }
    
    }
}

//thread function to keep running mario
void *runMario(void *unused){
    
    unsigned int *gpioPtr = getGPIOPtr();
    Init_GPIO(CLK, 1, gpioPtr);         // init pin 11 to output
    Init_GPIO(LAT, 1, gpioPtr);         // init pin 9 to output
    Init_GPIO(DAT, 0, gpioPtr);

    /* initialize + get FBS */
    framebufferstruct = initFbInfo();
    
    /* initialize a pixel */
    Pixel *pixel;
    pixel = malloc(sizeof(Pixel));
    
    DrawingMario(gpioPtr, pixel);   
    
    pthread_exit(0);                    // exit thread when finish
}



// function to draw number 
void DrawingTimeNumbers(unsigned int *gpioPtr, Pixel *pixel, int Number, int digit){
   
    short int *NUMBERPtr;
    short int *TIMEPtr=(short int *) TIMEImage.pixel_data;      // get pixel data of image of TIME

    if (Number == 0){
        NUMBERPtr = (short int *) ZEROImage.pixel_data;         // get pixel data of image of number 0
    }else if (Number == 1){
        NUMBERPtr = (short int *) ONEImage.pixel_data;          // get pixel data of image of number 1
    }else if (Number == 2){
        NUMBERPtr = (short int *) TWOImage.pixel_data;          // get pixel data of image of number 2
    }else if (Number == 3){
        NUMBERPtr = (short int *) THREEImage.pixel_data;        // get pixel data of image of number 3
    }else if (Number == 4){
        NUMBERPtr = (short int *) FOURImage.pixel_data;         // get pixel data of image of number 4
    }else if (Number == 5){
        NUMBERPtr = (short int *) FIVEImage.pixel_data;         // get pixel data of image of number 5
    }else if (Number == 6){
        NUMBERPtr = (short int *) SIXImage.pixel_data;          // get pixel data of image of number 6
    }else if (Number == 7){
        NUMBERPtr = (short int *) SEVENImage.pixel_data;        // get pixel data of image of number 7
    }else if (Number == 8){
        NUMBERPtr = (short int *) EIGHTImage.pixel_data;        // get pixel data of image of number 8
    }else if (Number == 9){
        NUMBERPtr = (short int *) NINEImage.pixel_data;         // get pixel data of image of number 9

    }

    int w = 0;
    for (int y = 0; y < 64; y++){               // get area to display the word time
        for (int x = 0; x < 192; x++){
            
            pixel->color = TIMEPtr[w];          // get color of Time word
            pixel->x = x+320;                   // get coordinate
            pixel->y = y;
        
            drawPixel(pixel);                   // draw
            w++;                                // increment index of Time word
        }
    }

    w = 0;

    for (int y = 0; y < 64; y++){               // 1 grid 64x64 for each number
        for (int x = 0; x < 64; x++){
            
            pixel->color = NUMBERPtr[w];        // get color from the number image
            if (digit == 1){                    // if the number is 1 digit
                pixel->x = x+640;               // set x location
            }else if (digit == 10){             // if number is 10 digit
                pixel->x = x+576;               // set x
            }else if (digit == 100){            // if number digit is 100
                pixel->x = x+512;               // set x
            }
            pixel->y = y;                       // set y
        
            drawPixel(pixel);
            w++;                                // increment index of number image
        }
    
    }
}

// function to display the time countdown
void DrawingTime(unsigned int *gpioPtr, Pixel *pixel){
    int one_digit = 0;
    int ten_digit = 0;
    int hundred_digit = 0;
    
    while (p.game_on == 1){

        while(p.gamepause == 1){
            if (p.restart == 1){
                return;
            }
        }

        one_digit = p.time % 10;            // get number of 1 digit
        ten_digit = (p.time/10) % 10;       // get number of 10 digitd
        hundred_digit = p.time/100;         // get number of hundred digit
        
        DrawingTimeNumbers(gpioPtr,pixel,one_digit,1);
        DrawingTimeNumbers(gpioPtr,pixel,ten_digit,10);
        DrawingTimeNumbers(gpioPtr,pixel,hundred_digit,100);
    }   
}

// thread function to run the time display
void *runTime(void *unused){
    
    unsigned int *gpioPtr = getGPIOPtr();
    Init_GPIO(CLK, 1, gpioPtr);         // init pin 11 to output
    Init_GPIO(LAT, 1, gpioPtr);         // init pin 9 to output
    Init_GPIO(DAT, 0, gpioPtr);

    /* initialize + get FBS */
    framebufferstruct = initFbInfo();
    
    /* initialize a pixel */
    Pixel *pixel;
    pixel = malloc(sizeof(Pixel));
    
    DrawingTime(gpioPtr, pixel); 
    
    pthread_exit(0);                    //exit thread when finish
}

// thread function to tick the time
void *tickTime(void *unused){
    
    while((p.time != 0) && p.game_on == 1 && p.distance != 0){
        while(p.gamepause == 1){
            if (p.restart == 1){
                pthread_exit(0);            // exit if restart is flag
            }
        }
        sleep(1);
        p.time = p.time - 1;                // decrement time
    }

    if (p.time <= 0){                       // if time = 0  
        p.loseflag = 1;                     // lose
    }
    
    pthread_exit(0);                        // exit when finish
}



// Score
void DrawingScoreNumbers(unsigned int *gpioPtr, Pixel *pixel, int Number, int digit){

    short int *NUMBERPtr;
    short int *SCOREPtr = (short int*) SCOREImage.pixel_data;   // get pixel data of Score Image

    if (Number == 0){
        NUMBERPtr = (short int *) ZEROImage.pixel_data;         // get pixel data of image of number 0
    }else if (Number == 1){
        NUMBERPtr = (short int *) ONEImage.pixel_data;          // get pixel data of image of number 1
    }else if (Number == 2){
        NUMBERPtr = (short int *) TWOImage.pixel_data;          // get pixel data of image of number 2
    }else if (Number == 3){
        NUMBERPtr = (short int *) THREEImage.pixel_data;        // get pixel data of image of number 3
    }else if (Number == 4){
        NUMBERPtr = (short int *) FOURImage.pixel_data;         // get pixel data of image of number 4
    }else if (Number == 5){
        NUMBERPtr = (short int *) FIVEImage.pixel_data;         // get pixel data of image of number 5
    }else if (Number == 6){
        NUMBERPtr = (short int *) SIXImage.pixel_data;          // get pixel data of image of number 6
    }else if (Number == 7){
        NUMBERPtr = (short int *) SEVENImage.pixel_data;        // get pixel data of image of number 7
    }else if (Number == 8){
        NUMBERPtr = (short int *) EIGHTImage.pixel_data;        // get pixel data of image of number 8
    }else if (Number == 9){
        NUMBERPtr = (short int *) NINEImage.pixel_data;         // get pixel data of image of number 9

    }


    for (int y = 0; y < 64; y++){           // a 64x64 grid 
        for (int x = 0; x < 64; x++){
            
            pixel->color = 0x000;           // get color black
            pixel->x = x+704;               // get coordinate
            pixel->y = y;
        
            drawPixel(pixel);               // draw that grid black
        }
    
    }

    int w = 0;
    for (int y = 0; y < 64; y++){           // get area to display the score word
        for (int x = 0; x < 256; x++){
            
            pixel->color = SCOREPtr[w];     // get color of the words
            pixel->x = x+768;               // get coordinate
            pixel->y = y;
        
            drawPixel(pixel);               // draw
            w++;
        }
    
    }


    w = 0 ;

    for (int y = 0; y < 64; y++){               // 1 grid for each number
        for (int x = 0; x < 64; x++){
            
            pixel->color = NUMBERPtr[w];        // get color of the number
            if (digit == 1){
                pixel->x = x+1216;              // calculate x
            }else if (digit == 10){
                pixel->x = x+1152;
            }else if (digit == 100){
                pixel->x = x+1088;
            }else if (digit == 1000){
                pixel->x = x+1024;
            }
            pixel->y = y;
        
            drawPixel(pixel);
            w++;                                 // increment pixel index of number image
        }
    }
}

// function to draw the score
void DrawingScore(unsigned int *gpioPtr, Pixel *pixel){

    int one_digit = 0;
    int ten_digit = 0;
    int hundred_digit = 0;
    int thousand_digit = 0;
    
    while (p.game_on == 1){

        while(p.gamepause == 1){
            if (p.restart == 1){
                return;
            }
        }
        /* Calculating the digits */
        one_digit = p.score % 10;
        ten_digit = (p.score/10) % 10;
        hundred_digit = (p.score/100) % 10;
        thousand_digit = p.score/1000;
        
        DrawingScoreNumbers(gpioPtr,pixel,one_digit,1);
        DrawingScoreNumbers(gpioPtr,pixel,ten_digit,10);
        DrawingScoreNumbers(gpioPtr,pixel,hundred_digit,100);
        DrawingScoreNumbers(gpioPtr,pixel,thousand_digit,1000);
    }

}

// thread function to keep track of the score
void *runScore(void *unused){
    
    unsigned int *gpioPtr = getGPIOPtr();
    Init_GPIO(CLK, 1, gpioPtr);         // init pin 11 to output
    Init_GPIO(LAT, 1, gpioPtr);         // init pin 9 to output
    Init_GPIO(DAT, 0, gpioPtr);

    /* initialize + get FBS */
    framebufferstruct = initFbInfo();
    
    /* initialize a pixel */
    Pixel *pixel;
    pixel = malloc(sizeof(Pixel));
    
    DrawingScore(gpioPtr, pixel);    
    
    pthread_exit(0);                    // exit thread when finish
}


// Coin
void CheckCoin(int time, int object, int lane){
    for(int check = 0; check < time; check ++){                         // set moving speed for coin
        delayMicroseconds(1);
        if (p.current_x == object*64 && p.current_y == lane*64){        // if mario hit coin
            if ((p.score + 10)>9999){                                   // if score > 9999
                p.score = 9999;                                         // max score 9999
            }else{
                p.score = p.score + 10;                                 // score = 10 for 1 coin
            }
            break;
        }
    }
}

// Coin (value pack 1)
void DrawingCoin(unsigned int *gpioPtr, Pixel *pixel){

    short int *MapPtr;
    if (p.stage == 1){
        MapPtr=(short int *) Map1Image.pixel_data;          // get pixel data of map 1
    }else if(p.stage == 2){
        MapPtr=(short int *) Map2Image.pixel_data;          // get pixel data of map 2
    }else if(p.stage ==3){
        MapPtr=(short int *) Map3Image.pixel_data;          // get pixel data of map 3
    }else{
        MapPtr=(short int *) Map4Image.pixel_data;          // get pixel data of map 4
    }

    short int *FinishPtr=(short int *) FinishImage.pixel_data;  // get pixel data of Finish line
    short int *CoinPtr=(short int *) CoinImage.pixel_data;      // get pixel data of Coin
    
    int w;
    int lane;

    while(p.game_on == 1){
        

        int coin_x = 19;                    // coin appear from far right

        lane = 0;

        if (p.distance > 0){
            if (p.time < 68){           // 2s into the game
                srand(time(NULL));
                lane = rand()%12;       // possibility the coin appear
                lane = lane + 5;        // get lane
            }
        }

        while (p.game_on == 1){
            while(p.gamepause == 1) {
                if (p.restart == 1){
                    return;
                }
            }
            if (lane == 5 || lane == 6 || lane == 7 || lane == 8 || lane == 9 || lane == 10){   // which lane coin appear

                w = 0;
                /*	Fixing Background	*/
                for (int y = lane*64; y < lane*64 + 64; y++)                        // get area of coin appear on lane
                {
                    for (int x = (coin_x+1)*64 ; x < (coin_x+1)*64 + 64; x++) 
                    {
                        
                        if ((coin_x+1)*64 == p.finish_x*64){                        // if coin hit finish line
                            pixel->color = FinishPtr[w];                            // get color of finish line
                        }else{
                            pixel->color = MapPtr[y*width+x];                       // get color of lower background
                        }
                        pixel->x = x;                                               // get coordinate
                        pixel->y = y;
                
                        drawPixel(pixel);                                           // draw
                        w++;                                                        // increment index of finish line
                    }
                
                }

                if (coin_x == -1){      // coin not go out of screen
                    break;
                }

                w = 0;
                /* Coin Location */
                for (int y = lane*64; y < lane*64 + 64; y++)                    // get area of coin appear on lane
                {
                    for (int x = coin_x*64; x < coin_x*64 + 64; x++) 
                    {
                        if (coin_x*64 == p.finish_x*64){                        // if coin hit finish line
                            if (CoinPtr[w] == 0x000){                           // if coin pixel is black
                                pixel->color = FinishPtr[w];                    // get color of finish line
                            }else{
                                pixel->color = CoinPtr[w];                      // get color of coin
                            }
                        }else{                                                  // if coin on road
                            if (CoinPtr[w] == 0x000){                           // if coin pixel is black
                                pixel->color = MapPtr[y*width+x];               // get color of road
                            }else{
                                pixel->color = CoinPtr[w];                      // get color of coin
                            }
                        }

                        pixel->x = x;                                           // get coorinate
                        pixel->y = y;
                        if(p.gamepause == 1) {
                            return;
                        }
                        drawPixel(pixel);                                       // draw
                        w++;                                                    // increment index of coin and finish line
                    }
                
                }

                if (p.stage == 1){
                    CheckCoin(170000,coin_x,lane);
                    if (p.current_x == coin_x*64 && p.current_y == lane*64){    // if mario hit coin
                        break;
                    }
                }else if (p.stage ==2 ){
                    CheckCoin(150000,coin_x,lane);
                    if (p.current_x == coin_x*64 && p.current_y == lane*64){    // if mario hit coin
                        break;
                    }
                }else if (p.stage == 3){
                    CheckCoin(130000,coin_x,lane);
                    if (p.current_x == coin_x*64 && p.current_y == lane*64){    // if mario hit coin
                        break;
                    }
                }else{
                    CheckCoin(100000,coin_x,lane);
                    if (p.current_x == coin_x*64 && p.current_y == lane*64){    // if mario hit coin
                        break;
                    }
                }

                coin_x = coin_x - 1;                                            // coin move left 1 grid at time
            }else{
                break;
            }
        }
        sleep(1);
    }

}

// thread function to control coin
void *runCoin(void *unused){
    
    unsigned int *gpioPtr = getGPIOPtr();
    Init_GPIO(CLK, 1, gpioPtr);         // init pin 11 to output
    Init_GPIO(LAT, 1, gpioPtr);         // init pin 9 to output
    Init_GPIO(DAT, 0, gpioPtr);

    /* initialize + get FBS */
    framebufferstruct = initFbInfo();
    
    /* initialize a pixel */
    Pixel *pixel;
    pixel = malloc(sizeof(Pixel));
    
    DrawingCoin(gpioPtr, pixel); 
    
    pthread_exit(0);                    // exit thread when finish
}



// Small Heart
void CheckHeart(int time, int object, int lane){
   for(int check = 0; check < time; check ++){                      // set small heart moving speed
        delayMicroseconds(1);
        if (p.current_x == object*64 && p.current_y == lane*64){    // if mario hit heart
            if (p.lives < 5){   
                p.lives = p.lives + 1;                              // increment live
            }

            break;
        } 
    } 
}

// Small Heart (value pack2)
void DrawingSmallHeart(unsigned int *gpioPtr, Pixel *pixel){

    short int *MapPtr;
    if (p.stage == 1){
        MapPtr=(short int *) Map1Image.pixel_data;      // get pixel data of map 1
    }else if(p.stage == 2){
        MapPtr=(short int *) Map2Image.pixel_data;      // get pixel data of map 2
    }else if(p.stage ==3){
        MapPtr=(short int *) Map3Image.pixel_data;      // get pixel data of map 3
    }else{
        MapPtr=(short int *) Map4Image.pixel_data;      // get pixel data of map 4
    }

    short int *FinishPtr=(short int *) FinishImage.pixel_data;          // get pixel data of Finish line
    short int *smallHeartPtr=(short int *) smallHeartImage.pixel_data;  // get pixel data of small heart 
    
    int w;
    int lane;

    while(p.game_on == 1){
        
        int heart_x = 19;               // small heart appear from the right

        lane = 0;

        if (p.distance > 0){
            if (p.time < 41){
                srand(time(NULL));
                lane = rand()%100;      // get possibility
                lane = lane + 5;        // get lane which small heart appear
            }
        }

        while(p.game_on == 1){
            while(p.gamepause == 1) {
                if (p.restart == 1){
                    return;
                }
            }
            if (lane == 5 || lane == 6 || lane == 7 || lane == 8 || lane == 9 || lane == 10){

                w = 0;

                /*	Fixing Background	*/
                for (int y = lane*64; y < lane*64 + 64; y++)                    // get background location
                {
                    for (int x = (heart_x+1)*64 ; x < (heart_x+1)*64 + 64; x++) 
                    {
                        if ((heart_x+1)*64 == p.finish_x){                      // if small heart hit finish lines
                            pixel->color = FinishPtr[w];                        // get color of finish line
                        }else{                                                  // if small heart on road
                            pixel->color = MapPtr[y*width+x];                   // get color of road
                        } 
                        pixel->x = x;                                           // get coordinate
                        pixel->y = y;
                
                        drawPixel(pixel);                           
                        w++;                                                    // increment pixel index of finish line
                    }
                }

                if (heart_x == -1){                                             // heart stop when it reach the left edge of screen
                    break;
                }
                
                if(p.gamepause == 1) {
                    return;
                }

                w = 0;

                /* small heart Location */
                for (int y = lane*64; y < lane*64 + 64; y++)                    // get heart location
                {
                    for (int x = heart_x*64; x < heart_x*64 + 64; x++) 
                    {
                        if (heart_x*64 == p.finish_x*64){                       // if small heart hit finish lines      
                            if (smallHeartPtr[w] == 0x000){                     // if heart pixel is black
                                pixel->color = FinishPtr[w];                    // get color of finish line
                            }else{                                      
                                pixel->color = smallHeartPtr[w];                // get color of small heart
                            }
                        }else{                                                  // if heart on road
                            if (smallHeartPtr[w] == 0x000){                     // if heart pixel is black
                                pixel->color = MapPtr[y*width+x];               // get color of map
                            }else{
                                pixel->color = smallHeartPtr[w];                // get color of small heart
                            }
                        }
                        pixel->x = x;                                           // get coordinate
                        pixel->y = y;
                        if(p.gamepause == 1) {
                            return;
                        }
                        drawPixel(pixel);
                        w++;                                                    // increment pixel index of finish line and small heart
                    }
                }
                
                
                }

                /*  Collision Check */

                if (p.stage == 1){
                    CheckHeart(170000, heart_x, lane);
                    if (p.current_x == heart_x*64 && p.current_y == lane*64){   // if mario hit heart
                        break;
                    }
                }else if (p.stage ==2 ){
                    CheckHeart(150000, heart_x, lane);
                    if (p.current_x == heart_x*64 && p.current_y == lane*64){   // if mario hit heart
                        break;
                    }
                }else if (p.stage == 3){
                    CheckHeart(130000, heart_x, lane);
                    if (p.current_x == heart_x*64 && p.current_y == lane*64){   // if mario hit heart
                        break;
                    }
                }else{
                    CheckHeart(100000, heart_x, lane);
                    if (p.current_x == heart_x*64 && p.current_y == lane*64){   // if mario hit heart
                        break;
                    }
                }

                heart_x = heart_x - 1;      // heart move from right to left, 1 grid at time
            }else{
                break;
            }
        }
        sleep(1);
    }
}

// function to run small heart when playing
void *runSmallHeart(void *unused){
    
    unsigned int *gpioPtr = getGPIOPtr();
    Init_GPIO(CLK, 1, gpioPtr);         // init pin 11 to output
    Init_GPIO(LAT, 1, gpioPtr);         // init pin 9 to output
    Init_GPIO(DAT, 0, gpioPtr);

    /* initialize + get FBS */
    framebufferstruct = initFbInfo();
    
    /* initialize a pixel */
    Pixel *pixel;
    pixel = malloc(sizeof(Pixel));

    DrawingSmallHeart(gpioPtr, pixel); 
   
    pthread_exit(0);                    // exit thread when finish
}



// Rocket
void CheckHIT(int time, int object, int lane){
    for(int check = 0; check < time; check ++){                     // set speed for rocket
        delayMicroseconds(1);
        if (p.current_x == object*64 && p.current_y == lane*64){    // if rocket hit mario
            p.lives = p.lives - 1;                                  // decrement live
            p.distance = p.distance + 2;                            // add 2 distance
            break;
        }
    }
}

// Rocket
void DrawingRocket(unsigned int *gpioPtr, Pixel *pixel){
    short int *MapPtr;
    if (p.stage == 1){
        MapPtr=(short int *) Map1Image.pixel_data;                  // get pixel data of map 1
    }else if(p.stage == 2){
        MapPtr=(short int *) Map2Image.pixel_data;                  // get pixel data of map 2
    }else if(p.stage ==3){
        MapPtr=(short int *) Map3Image.pixel_data;                  // get pixel data of map 3
    }else{
        MapPtr=(short int *) Map4Image.pixel_data;                  // get pixel data of map 4
    }

    short int *FinishPtr=(short int *) FinishImage.pixel_data;      // get pixel data of finish line
    short int *RocketPtr=(short int *) RocketImage.pixel_data;      // get pixel data of rocket
    
    int w;
    int lane;

    while (p.game_on == 1){
        int rocket_x = 19;                      // rocket appears on the right

        lane = 0;

        if (p.distance > 0){
            if (p.distance < 58){
                srand(time(NULL));              // seed rand
                lane = rand()%8;                // get possibility of appearance
                lane = lane + 5;                // get lane it appear
            }
        }

        while(p.game_on == 1){
            while(p.gamepause == 1) {
                if (p.restart == 1){
                    return;
                }
            }
            if (lane == 5 || lane == 6 || lane == 7 || lane == 8 || lane == 9 || lane == 10){

                w = 0;

                for (int y = lane*64; y < lane*64 + 64; y++)                        // get background locaiton
                {
                    for (int x = (rocket_x+1)*64 ; x < (rocket_x+1)*64 + 64; x++) 
                    {
                        if ((rocket_x+1)*64 == p.finish_x*64){                      // if rocket hit finish line
                            pixel->color = FinishPtr[w];                            // get color of finish line
                        }else{                                                      // if rocket on road
                            pixel->color = MapPtr[y*width+x];                       // get color of road
                        }
                        pixel->x = x;                                               // get coordinate
                        pixel->y = y;
                        
                        drawPixel(pixel);
                        w++;                                                        // increment index of pixel in finish line
                    }
                
                }

                if (rocket_x == -1){            // if rocket reach the left edge
                    break;
                }

                if(p.gamepause == 1) {
                    return;
                }

                w = 0;

                /* Rocket Location */
                for (int y = lane*64; y < lane*64 + 64; y++)                // get rocket location
                {
                    for (int x = rocket_x*64; x < rocket_x*64 + 64; x++) 
                    {
                        if (rocket_x*64 == p.finish_x*64){                  // if rocket hit finish line 
                            if (RocketPtr[w] == 0x000){                     // if rocket pixel is black
                                pixel->color = FinishPtr[w];                // get color of finish line
                            }else{
                                pixel->color = RocketPtr[w];                // get color of rocket
                            }
                        }else{                                              // if rocket on road
                           if (RocketPtr[w] == 0x000){                      // if rocket pixel is black
                                pixel->color = MapPtr[y*width+x];           // get road color
                            }else{
                                pixel->color = RocketPtr[w];                // get rocket color
                            } 
                        }

                        pixel->x = x;                                       // get coordinate
                        pixel->y = y;
                        
                        drawPixel(pixel);
                        w++;                                                // increment index of rocket and finish line 
                    }
                }

                if (p.stage == 1){
                    CheckHIT(65000,rocket_x,lane);
                    if (p.current_x == rocket_x*64 && p.current_y == lane*64){  // if rocket hit mario
                        break;
                    }
                }else if (p.stage ==2 ){
                    CheckHIT(45000,rocket_x,lane);
                    if (p.current_x == rocket_x*64 && p.current_y == lane*64){  // if rocket hit mario
                        break;
                    }
                }else if (p.stage == 3){
                    CheckHIT(35000,rocket_x,lane);
                    if (p.current_x == rocket_x*64 && p.current_y == lane*64){  // if rocket hit mario
                        break;
                    }
                }else{
                    CheckHIT(30000,rocket_x,lane);
                    if (p.current_x == rocket_x*64 && p.current_y == lane*64){  // if rocket hit mario
                        break;
                    }
                }

                rocket_x = rocket_x - 1;        // rocket move right to left, 1 grid at time
            }else{
                break;
            }
        }
        sleep(1);
    }
}

// thread function to run rocket
void *runRocket(void *unused){
    
    unsigned int *gpioPtr = getGPIOPtr();
    Init_GPIO(CLK, 1, gpioPtr);         // init pin 11 to output
    Init_GPIO(LAT, 1, gpioPtr);         // init pin 9 to output
    Init_GPIO(DAT, 0, gpioPtr);

    /* initialize + get FBS */
    framebufferstruct = initFbInfo();
    
    /* initialize a pixel */
    Pixel *pixel;
    pixel = malloc(sizeof(Pixel));
    
    DrawingRocket(gpioPtr, pixel);
    
    pthread_exit(0);                    // exit when finish
}

// Green Shell
void DrawingGreenShell(unsigned int *gpioPtr, Pixel *pixel){
    short int *MapPtr;
    if (p.stage == 1){
        MapPtr=(short int *) Map1Image.pixel_data;      // get pixel data of map 1
    }else if(p.stage == 2){
        MapPtr=(short int *) Map2Image.pixel_data;      // get pixel data of map 2
    }else if(p.stage ==3){
        MapPtr=(short int *) Map3Image.pixel_data;      // get pixel data of map 3
    }else{
        MapPtr=(short int *) Map4Image.pixel_data;      // get pixel data of map 4
    }

    short int *FinishPtr=(short int *) FinishImage.pixel_data;              // get pixel data of finish line
    short int *GreenShellPtr=(short int *) GreenShellImage.pixel_data;      // get pixel data of greenshell
    int w;
    int lane;
    
    while(p.game_on == 1){
        
        int greenShell_x = 0;                   // greenshell appears on left and goes to right

        lane = 0;

        if (p.distance > 0){
            if (p.distance <58){
                srand(time(NULL));
                lane = rand()%9;                // get possibility the shell appear
                lane = lane + 5;                // get lane
            }
        }

        while(p.game_on == 1){
            while(p.gamepause == 1){
                if (p.restart == 1){
                    return;
                }
            }
            if (lane == 5 || lane == 6 || lane == 7 || lane == 8 || lane == 9 || lane == 10){
                w = 0;

                /*  Fixing Background   */
                for (int y = lane*64; y < lane*64 + 64; y++)                                // draw background location
                {
                    for (int x = (greenShell_x-1)*64 ; x < (greenShell_x-1)*64 + 64; x++) 
                    {
                        if ((greenShell_x-1)*64 == p.finish_x*64){                          // if shell hit finish line
                            pixel->color = FinishPtr[w];                                    // get finish line color
                        }else{                                                              // if shell on road
                            pixel->color = MapPtr[y*width+x];                               // get road color
                        }
                        pixel->x = x;                                                       // get coordinate
                        pixel->y = y;
                
                        drawPixel(pixel);                                                   // draw
                        w++;                                                                // increment index of pixel in finish line
                    }
                }

                if (greenShell_x == 20){                        // break when greenshell move across the screen
                    break;
                }
                if(p.gamepause == 1) {
                    return;
                }

                w = 0;
                
                /* Shell Location */
                for (int y = lane*64; y < lane*64 + 64; y++)            // get shell location
                {
                    for (int x = greenShell_x*64; x < greenShell_x*64 + 64; x++) 
                    {
                        if(greenShell_x*64 == p.finish_x*64){           // if shell hit finish line
                            if (GreenShellPtr[w] == 0x000){             // get shell color pixel is black
                                pixel->color = FinishPtr[w];            // get finish line color 
                            }else{                                      // if shell on road
                                pixel->color = GreenShellPtr[w];        // get greenshell color
                            }
                        }else{
                            if (GreenShellPtr[w] == 0x000){             // get shell color pixel is black
                                pixel->color = MapPtr[y*width+x];       // get road color
                            }else{
                                pixel->color = GreenShellPtr[w];        // get greenshell color
                            }
                        }
                        pixel->x = x;                                   // get coordinate
                        pixel->y = y;   
                        
                        drawPixel(pixel);
                        w++;
                    }
                }

                if (p.stage == 1){
                    CheckHIT(75000,greenShell_x,lane);
                    if (p.current_x == greenShell_x*64 && p.current_y == lane*64){  // if mario hit shell
                        break;
                    }
                }else if (p.stage ==2 ){
                    CheckHIT(55000,greenShell_x,lane);
                    if (p.current_x == greenShell_x*64 && p.current_y == lane*64){  // if mario hit shell
                        break;
                    }
                }else if (p.stage == 3){
                    CheckHIT(45000,greenShell_x,lane);
                    if (p.current_x == greenShell_x*64 && p.current_y == lane*64){  // if mario hit shell
                        break;
                    }
                }else{
                    CheckHIT(35000,greenShell_x,lane);
                    if (p.current_x == greenShell_x*64 && p.current_y == lane*64){  // if mario hit shell
                        break;
                    }
                }

                greenShell_x = greenShell_x + 1;        // shell move from left to right, 1 grid at time

            }else{
                break;
            }
        }

        sleep(1); 
    }
}

// thread function to run Greenshell
void *runGreenShell(void *unused){
    
    unsigned int *gpioPtr = getGPIOPtr();
    Init_GPIO(CLK, 1, gpioPtr);         // init pin 11 to output
    Init_GPIO(LAT, 1, gpioPtr);         // init pin 9 to output
    Init_GPIO(DAT, 0, gpioPtr);

    /* initialize + get FBS */
    framebufferstruct = initFbInfo();
    
    /* initialize a pixel */
    Pixel *pixel;
    pixel = malloc(sizeof(Pixel));

    DrawingGreenShell(gpioPtr, pixel); 

    pthread_exit(0);                      // exit when finish
}




// Clock
void CheckClock(int time, int object, int lane){
    for(int check = 0; check < time; check ++){                     // set speed for clock
        delayMicroseconds(1);
        if (p.current_x == object*64 && p.current_y == lane*64){    // if clock hit mario
            p.time = p.time + 3;                                    // add 3s to timer

            break;
        }
    }
}

// Clock (value pack 3)
void DrawingClock(unsigned int *gpioPtr, Pixel *pixel){

    short int *MapPtr;
    if (p.stage == 1){
        MapPtr=(short int *) Map1Image.pixel_data;      // get pixel data of map 1
    }else if(p.stage == 2){
        MapPtr=(short int *) Map2Image.pixel_data;      // get pixel data of map 2
    }else if(p.stage ==3){
        MapPtr=(short int *) Map3Image.pixel_data;      // get pixel data of map 3
    }else{
        MapPtr=(short int *) Map4Image.pixel_data;      // get pixel data of map 4
    }

    short int *FinishPtr=(short int *) FinishImage.pixel_data;  // get pixel data of finish line
    short int *ClockPtr=(short int *) ClockImage.pixel_data;    // get pixel data of clock
    
    int w;
    int lane;

    while(p.game_on == 1){

        int clock_x = 19;

        lane = 0;

        if (p.distance > 0){
            if (p.time < 41){
                srand(time(NULL));
                lane = rand()/23;           // get possiblity of appearance
                lane = lane % 30;
                lane = lane + 5;            // get lane it appear
            }
        }

        while(p.game_on == 1){
            while(p.gamepause == 1){
                if (p.restart == 1){
                    return;
                }
            }
            if (lane == 5 || lane == 6 || lane == 7 || lane == 8 || lane == 9 || lane == 10){

                w = 0;
                
                /*	Fixing Background	*/
                for (int y = lane*64; y < lane*64 + 64; y++)                        // get background location
                {
                    for (int x = (clock_x+1)*64 ; x < (clock_x+1)*64 + 64; x++) 
                    {
                        if ((clock_x+1)*64 == p.finish_x){                          // if clock hit finish line
                            pixel->color = FinishPtr[w];                            // get color of finish line
                        }else{                                                      // if clock on road
                            pixel->color = MapPtr[y*width+x];                       // get color of road
                        }
                        pixel->x = x;                                               // get coordinate
                        pixel->y = y;
                
                        drawPixel(pixel);
                        w++;                                                        // increment index of finish line
                    }
                }

                if (clock_x == -1){         // if clock reach left edge of screen
                    break;
                }

                w = 0;
                /* Clock Location */
                for (int y = lane*64; y < lane*64 + 64; y++)                // get clock location
                {
                    for (int x = clock_x*64; x < clock_x*64 + 64; x++) 
                    {
                        if(clock_x*64 == p.finish_x*64){                    // if clock hit finish line
                            if (ClockPtr[w] == 0x000){                      // if clock pixel is black
                                pixel->color = FinishPtr[w];                // get color of finish line
                            }else{
                                pixel->color = ClockPtr[w];                 // get color of clock
                            }
                        }else{                                              // if clock on road
                            if (ClockPtr[w] == 0x000){                      // if clock pixel is black
                                pixel->color = MapPtr[y*width+x];           // get color of road
                            }else{  
                                pixel->color = ClockPtr[w];                 // get color of clock
                            }
                        }
                        pixel->x = x;                                       // get coordinate
                        pixel->y = y;
                        if(p.gamepause == 1) {
                            return;
                        }
                        drawPixel(pixel);
                        w++;                                                // increment index of pixel of clock and finish line
                    }   
                
                }
                
                /*  Collision Check */

                if (p.stage == 1){

                    CheckClock(170000, clock_x, lane);
                    
                    if (p.current_x == clock_x*64 && p.current_y == lane*64){   // if clock hit mario
                        break;
                    }
                }else if (p.stage ==2 ){
                    CheckClock(150000, clock_x, lane);
                    
                    if (p.current_x == clock_x*64 && p.current_y == lane*64){   // if clock hit mario
                        break;
                    }
                }else if (p.stage == 3){
                    CheckClock(130000, clock_x, lane);
                    
                    if (p.current_x == clock_x*64 && p.current_y == lane*64){   // if clock hit mario
                        break;
                    }
                }else{
                    CheckClock(100000, clock_x, lane);
                    
                    if (p.current_x == clock_x*64 && p.current_y == lane*64){   // if clock hit mario
                        break;
                    }
                }

                clock_x = clock_x - 1;      // clock goes right to left, 1 grid at time
            }else{
                break;
            }
        }
        sleep(1);
    }
    
}

// thread function to run clock
void *runClock(void *unused){
    
    unsigned int *gpioPtr = getGPIOPtr();
    Init_GPIO(CLK, 1, gpioPtr);         // init pin 11 to output
    Init_GPIO(LAT, 1, gpioPtr);         // init pin 9 to output
    Init_GPIO(DAT, 0, gpioPtr);

    /* initialize + get FBS */
    framebufferstruct = initFbInfo();
    
    /* initialize a pixel */
    Pixel *pixel;
    pixel = malloc(sizeof(Pixel));
    
    DrawingClock(gpioPtr, pixel); 
    
    pthread_exit(0);                    // exit thread when finish

}



// thread function to control Distance
void *distance(void *unused){
    
    while(p.game_on == 1){
        while (p.gamepause == 1){           // if pause
            if (p.restart == 1){            // if restart
                pthread_exit(0);            // exit 
            }
        }

        if (p.time != 0 && p.distance != 0){
            sleep(1);
            p.distance = p.distance - 1;        // decrease 1 distance for every second
        }
    }    
        
    pthread_exit(0);                        // exit thread when finish

}

// initialize player struct when start playing game
void init() {
    p.winflag = 0;
    p.loseflag = 0;
    p.lives = 4;
    p.score = 0;
    p.time = 70;                // timer
    p.timeleft = 0;
    p.distance = 60;
    p.stage = 1;
    p.game_on = 1;
    p.total_score = 0;
    p.gamepause = 0;
    p.finish_x = -1;
    p.restart = 0;
}

// draw lost message
void DrawLOST(unsigned int *gpioPtr, Pixel *pixel){
    short int *LOSTPtr=(short int *) LOSTImage.pixel_data;      // get pixel data of Lost image

    i = 0;

     for (int y = 0; y < height; y++)           // select entire screen
	{
		for (int x = 0; x < width; x++) 
		{	
			pixel->color = LOSTPtr[i];          // get color of Lost image
			pixel->x = x;                       // get coordinate
			pixel->y = y;
	
			drawPixel(pixel);
            i++;                                // increment pixel index of lost image
		}
	}

    while(1) {
        Read_SNES(gpioPtr);                     // read input
        for(int i =0; i < 16; i++) {
            if (buttons[i] == 0)                // if any button pressed
                mainMenu(gpioPtr, pixel);       // go to main menu
        }
    }

}

// function to draw win message
void DrawWIN(unsigned int *gpioPtr, Pixel *pixel){
    short int *WINPtr=(short int *) WINImage.pixel_data;        // get pixel data of win image

    i = 0;

     for (int y = 0; y < height; y++)           // select entire screen
	{
		for (int x = 0; x < width; x++) 
		{	
			pixel->color = WINPtr[i];           // get color of win image
			pixel->x = x;                       // get coordinate
			pixel->y = y;
	
			drawPixel(pixel);
            i++;                                // increment pixel index of win image
		}
	}

    sleep(3);

}

void DrawFinalNumbers(unsigned int *gpioPtr, Pixel *pixel,int Number, int digit){

    short int *NUMBERPtr;

    if (Number == 0){
        NUMBERPtr = (short int *) ZEROImage.pixel_data;         // get pixel data of image of number 0
    }else if (Number == 1){
        NUMBERPtr = (short int *) ONEImage.pixel_data;          // get pixel data of image of number 1
    }else if (Number == 2){
        NUMBERPtr = (short int *) TWOImage.pixel_data;          // get pixel data of image of number 2
    }else if (Number == 3){
        NUMBERPtr = (short int *) THREEImage.pixel_data;        // get pixel data of image of number 3
    }else if (Number == 4){
        NUMBERPtr = (short int *) FOURImage.pixel_data;         // get pixel data of image of number 4
    }else if (Number == 5){
        NUMBERPtr = (short int *) FIVEImage.pixel_data;         // get pixel data of image of number 5
    }else if (Number == 6){
        NUMBERPtr = (short int *) SIXImage.pixel_data;          // get pixel data of image of number 6
    }else if (Number == 7){
        NUMBERPtr = (short int *) SEVENImage.pixel_data;        // get pixel data of image of number 7
    }else if (Number == 8){
        NUMBERPtr = (short int *) EIGHTImage.pixel_data;        // get pixel data of image of number 8
    }else if (Number == 9){
        NUMBERPtr = (short int *) NINEImage.pixel_data;         // get pixel data of image of number 9

    }
    
    int w = 0 ; 
    for (int y = 360; y < 424; y++){            // get area to draw those numebr
        for (int x = 0; x < 64; x++){
            
            pixel->color = NUMBERPtr[w];        // get color of the number image
            if (digit == 1){
                pixel->x = x+512;               // calculate x coordinate
            }else if (digit == 10){
                pixel->x = x+576;
            }else if (digit == 100){
                pixel->x = x+640;
            }else if (digit == 1000){
                pixel->x = x+704;
            }
            pixel->y = y;
        
            drawPixel(pixel);
            w++;                                // increment pixel index of the number image
        }
    
    }
    
}

// draw finalscore (similar to draw score)
void DrawFINALSCORE(unsigned int *gpioPtr, Pixel *pixel){

    short int *FINALSCOREPtr=(short int *) FINALSCOREImage.pixel_data;  // get pixel data of FinalSCore

    int one_digit = 0;
    int ten_digit = 0;
    int hundred_digit = 0;
    int thousand_digit = 0;

    one_digit = p.total_score % 10;                 // calculate number of digits
    ten_digit = (p.total_score/10) % 10;
    hundred_digit = (p.total_score/100) % 10;
    thousand_digit = p.total_score/1000;

    i = 0;

     for (int y = 0; y < height; y++)               // draw entire screen
	{
		for (int x = 0; x < width; x++) 
		{	
			pixel->color = FINALSCOREPtr[i];        // get color of FinalScore
			pixel->x = x;                           // get coordinate
			pixel->y = y;
	
			drawPixel(pixel);
            i++;                                    // increment the pixel index of FinalScore
		}
	}

    sleep(1);

    DrawFinalNumbers(gpioPtr,pixel,one_digit,1);                // draw final number
    DrawFinalNumbers(gpioPtr,pixel,ten_digit,10);
    DrawFinalNumbers(gpioPtr,pixel,hundred_digit,100);
    DrawFinalNumbers(gpioPtr,pixel,thousand_digit,1000);

    while(1) {
        Read_SNES(gpioPtr);                                 // read input
        for(int i =0; i < 16; i++) {
            if (buttons[i] == 0)                            // if any button press
                mainMenu(gpioPtr, pixel);                   // got to main menu
        }
    }
}


// subroutine to start the game
void startGame(){
    unsigned int *gpioPtr = getGPIOPtr();

	Init_GPIO(CLK, 1, gpioPtr);         // init pin 11 to output
    Init_GPIO(LAT, 1, gpioPtr);         // init pin 9 to output
    Init_GPIO(DAT, 0, gpioPtr);

    /* initialize + get FBS */
	framebufferstruct = initFbInfo();
    
	/* initialize a pixel */
	Pixel *pixel;
	pixel = malloc(sizeof(Pixel));
    
    init();                             // init the struct p

    while (p.stage < 5){
        DrawingStage(gpioPtr, pixel);   // draw stage

        /* create and join thread */
        pthread_attr_t attr;

        pthread_t tmap, tmario, theart, tcoin, tscore, ttime, trtime, tgreenshell, tdistance, tsmallHeart, trocket, tfinish, tclock, tpause;

        pthread_attr_init(&attr);

        pthread_create(&tmario,&attr, runMario, NULL);
        pthread_create(&theart,&attr, runHeart, NULL);
        pthread_create(&tcoin,&attr, runCoin, NULL);
        pthread_create(&tscore,&attr, runScore, NULL);
        pthread_create(&ttime,&attr, tickTime, NULL);
        pthread_create(&trtime,&attr, runTime, NULL);
        pthread_create(&tgreenshell,&attr, runGreenShell, NULL);
        pthread_create(&tdistance,&attr, distance, NULL);
        pthread_create(&tsmallHeart,&attr, runSmallHeart, NULL);
        pthread_create(&trocket,&attr, runRocket, NULL);
        pthread_create(&tfinish,&attr, runFinish, NULL);
        pthread_create(&tclock,&attr, runClock, NULL);
        pthread_create(&tpause,&attr, runPause, NULL);
        pthread_create(&tmap,&attr, runMap, NULL);
        
        pthread_join(tmario,NULL);
        pthread_join(theart,NULL);
        pthread_join(tcoin,NULL);
        pthread_join(tscore,NULL);
        pthread_join(ttime,NULL);
        pthread_join(trtime,NULL);
        pthread_join(tgreenshell,NULL);
        pthread_join(tdistance,NULL);
        pthread_join(tsmallHeart,NULL);
        pthread_join(trocket,NULL);
        pthread_join(tfinish,NULL);
        pthread_join(tclock,NULL);
        pthread_join(tpause,NULL);
        pthread_join(tmap,NULL);

        if (p.loseflag == 1){       // if lose 
            break;
        }

        if (p.restart == 1){        // if restart
            break;
        }

        p.timeleft = p.timeleft + p.time;       // record adn add timeleft of each stage

        p.stage ++;                             // next stage
        p.time = 70;                            // reset timer
        p.distance = 60;                        // reset distance to finish line
        p.game_on = 1;                          // reset flag

    }

    if (p.restart == 1){                        // if restart then recursive Stargame
        startGame();
    }

    if(p.stage == 5){                           // if 5 stage ends
        p.winflag = 1;                          // won
    }

    if ((p.timeleft + p.lives)*3 + p.score > 9999){     // calc total score
        p.total_score = 9999;                           // max total score = 9999
    }else{
        p.total_score = (p.timeleft + p.lives)*3 + p.score; // calc total score
    }

    if(p.winflag == 1) {
        //draw win message and display score
        DrawWIN(gpioPtr, pixel);
        DrawFINALSCORE(gpioPtr, pixel);
    } 
    if (p.loseflag == 1) {
        // Drw lost and display score
        DrawLOST(gpioPtr, pixel);
        // DrawFINALSCORE(gpioPtr, pixel);
    }

    /* free pixel's allocated memory */
    free(pixel);
    pixel = NULL;
    munmap(framebufferstruct.fptr, framebufferstruct.screenSize);

}


/* main function */
int main(){

    unsigned int *gpioPtr = getGPIOPtr();
    

	Init_GPIO(CLK, 1, gpioPtr);         // init pin 11 to output
    Init_GPIO(LAT, 1, gpioPtr);         // init pin 9 to output
    Init_GPIO(DAT, 0, gpioPtr);

    /* initialize + get FBS */
	framebufferstruct = initFbInfo();
    
	/* initialize a pixel */
	Pixel *pixel;
	pixel = malloc(sizeof(Pixel));
    
    opening(gpioPtr, pixel);            // opening screen
    mainMenu(gpioPtr, pixel);           // goto main menu

	return 0;
}

/* Draw a pixel */
void drawPixel(Pixel *pixel){
	long int location = (pixel->x +framebufferstruct.xOff) * (framebufferstruct.bits/8) +
                       (pixel->y+framebufferstruct.yOff) * framebufferstruct.lineLength;
	*((unsigned short int*)(framebufferstruct.fptr + location)) = pixel->color;
}
