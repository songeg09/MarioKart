#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "framebuffer.h"

#include <unistd.h>
#include <wiringPi.h>
#include "initGPIO.h"

#include <pthread.h>

#include <time.h>

#include "OpeningBackground.h"
#include "MainMenu.h"
#include "Arrow.h"

#include "Coin.h"
#include "Heart.h"
#include "Mario.h"
#include "ExMap.h"

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

#include "Rocket.h"
#include "GreenShell.h"
#include "smallHeart.h"

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

// bool isButtonPressed = false;

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
        gpioPtr[GPSET0] = 1 << LAT;
    } else {
        gpioPtr[GPCLR0] = 1 << LAT;
    }
}

// function to write on pin 11 (CLOCK)
void Write_Clock(int n, unsigned int *gpioPtr) { 
    if (n == 1) {
        gpioPtr[GPSET0] = 1 << CLK;
    } else {
        gpioPtr[GPCLR0] = 1 << CLK;
    }
}


// function to Read Data in pin 10 (DAT)
unsigned int Read_Data(unsigned int *gpioPtr) {
    return (gpioPtr[GPLEV0] >> DAT) & 1;             // change pin 3 to 10. read DATA
}



// function to print message declaring which button is pressed
void ApplyChange(int cycle, int *c_x, int *c_y, int *p_x, int *p_y) {
    *p_y = *c_y;
    *p_x = *c_x;
    switch (cycle)
    {
    case 1:
        break;
    case 2:
        break;
    case 3:
        break;
    case 4:
        break;                               // Program terminates
    case 5:                                    // UP
        if (*c_y - 64 >= 320){
		    *c_y = *c_y - 64;
            break;
        }else{
            break;
        }
    case 6:                                    // DOWN
        if (*c_y + 128 <= 720){
		    *c_y = *c_y + 64;
            break;
        }else{
            break;
        }
    case 7:                                    // LEFT
        if (*c_x - 64 >= 0){
		    *c_x = *c_x - 64;
            break;
        }else{
            break;
        }
    case 8:                                    // RIGHT
        if (*c_x + 128 <= 1280){
		    *c_x = *c_x + 64;
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
    // default:
    
    }
}



int buttons[16];

// Read_SNES
void Read_SNES(unsigned int *gpioPtr) {
    // MOV button, #0   
    for(int i=0; i<16; i++) {       // reset buttons
        buttons[i] = 1;
    }
    // Write_Clock(1, gpioPtr);        // set pin 11
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
        // else {
        //     buttons[cycle-1] = 1;
        // }

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

int i;
int width = 1280;
int height = 720;

/* Definitions */
typedef struct {
	short int color;
	int x, y;
} Pixel;

struct fbs framebufferstruct;
void drawPixel(Pixel *pixel);


void opening(unsigned int *gpioPtr, Pixel *pixel){

    short int *OpeningBackgroundPtr=(short int *) OpeningBackgroundImage.pixel_data;
    i = 0;

     for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++) 
		{	
			pixel->color = OpeningBackgroundPtr[i];
			pixel->x = x;
			pixel->y = y;
	
			drawPixel(pixel);
            i++;
		}
	}

    do{
        Read_SNES(gpioPtr);
    }while (buttons[3]==1);

}

void mainMenu(unsigned int *gpioPtr, Pixel *pixel){
    short int *MainMenuPtr=(short int *) MainMenuImage.pixel_data;
    i = 0;

    for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++) 
		{	
			pixel->color = MainMenuPtr[i]; // white background
			pixel->x = x;
			pixel->y = y;
	
			drawPixel(pixel);
            i++;
		}
	}

    short int *ArrowPtr=(short int *) ArrowImage.pixel_data;
    int cursor = 0; // curosr is on play by default

    while(1){
        i = 0;
        
        // when cursor is on play
        if (cursor == 0){
            // Fixing the background
            for (int y = 520; y < 584; y++){
			    for (int x = 460; x < 524; x++) {
                
                    pixel->color = MainMenuPtr[y*width+x];
				    pixel->x = x;
				    pixel->y = y;
		
				    drawPixel(pixel);
                    i++;
			    }
		
		    }

            i = 0;
            // Draw the arrow
            for (int y = 370; y < 434; y++){
			    for (int x = 460; x < 524; x++) {
                
                    if (ArrowPtr[i] == 0x000){
                        pixel->color = MainMenuPtr[y*width+x];
                    }else{
                        pixel->color = ArrowPtr[i];
                    } 
				    pixel->x = x;
				    pixel->y = y;
		
				    drawPixel(pixel);
                    i++;
			    }
		
		    }
         
        // when cursor is on quit button
        }else if (cursor == 1){
            // Fixing the background
            for (int y = 370; y < 434; y++){
			    for (int x = 460; x < 524; x++) {
                
                    pixel->color = MainMenuPtr[y*width+x];
				    pixel->x = x;
				    pixel->y = y;
		
				    drawPixel(pixel);
                    i++;
			    }
		
		    }

            i = 0;

            // Draw the arrow
            for (int y = 520; y < 584; y++){
			    for (int x = 460; x < 524; x++) {
                
                    if (ArrowPtr[i] == 0x000){
                        pixel->color = MainMenuPtr[y*width+x];
                    }else{
                        pixel->color = ArrowPtr[i];
                    } 
				    pixel->x = x;
				    pixel->y = y;
		
				    drawPixel(pixel);
                    i++;
			    }
		
		    }
        }

        Read_SNES(gpioPtr);
        if (cursor == 0){
            if (buttons[5] == 0){
                cursor = 1;
            }else if (buttons[8] == 0){
                break;
            }
        
        }else if (cursor == 1){
            if (buttons[4] == 0){
                cursor = 0;
            }else if (buttons[8] == 0){
                for (int y = 0; y < height; y++){
			        for (int x = 0; x < width; x++) {
                
                        pixel->color = 0x000;
				        pixel->x = x;
				        pixel->y = y;
		
				        drawPixel(pixel);
			        }
		
		        }
                exit(0);               // Terminate the program
            }
        }

        delayMicroseconds(75000);
    }


}


struct player {
    int current_x;
    int current_y;

    int previous_x;
    int previous_y;

    int lives;

    int score;

    int timeleft;

    int time;

    int distance;

    int stage;

    int winflag;

    int loseflag;
};

struct player p;

void DrawingFinish(unsigned int *gpioPtr, Pixel *pixel){
    short int *ExMapPtr=(short int *) ExMapImage.pixel_data;
    short int *FinishPtr=(short int *) FinishImage.pixel_data;

    int finish_x = 19;
    int w = 0;

    while(p.winflag == 0 && p.loseflag == 0){
        /*	Fixing Background	*/
        
        for (int y = 320; y < 704; y++ )
        {
            for (int x = (finish_x+1)*64 ; x < (finish_x+1)*64 + 64; x++) 
            {
                pixel->color = ExMapPtr[y*width+x]; 
                pixel->x = x;
                pixel->y = y;
                drawPixel(pixel);
        
            }
        }
        
        

        if (finish_x == -1){
            break;
        }


        if (p.current_x == finish_x*64){
            p.stage++;

            //p.distance = 100;
            p.distance = 10;
            
            p.timeleft = p.timeleft + p.time;

            if (p.stage < 4){

                p.time = 120;

            }
        }
  
        w = 0;
        for (int y = 320; y < 704; y++ )
        {
            for (int x = (finish_x)*64 ; x < (finish_x)*64 + 64; x++) 
            {   
                pixel->color = FinishPtr[w]; 
                pixel->x = x;
                pixel->y = y;
                drawPixel(pixel);
                w++;
            }
        }
        
        
        if (p.stage == 1){
            delayMicroseconds(170000);
        }else if (p.stage ==2 ){
            delayMicroseconds(150000);
        }else if (p.stage == 3){
            delayMicroseconds(130000);
        }else{
            delayMicroseconds(100000);
        }
        
        finish_x = finish_x - 1;
        
    }

}

void DrawingMap(unsigned int *gpioPtr, Pixel *pixel){

    short int *ExMapPtr=(short int *) ExMapImage.pixel_data;
    int w = 0;

    for (int y = 320; y < 720; y++) {
        for (int x = 0; x < 1280; x++){
            pixel->color = ExMapPtr[y*width+x]; 
            pixel->x = x;
            pixel->y = y;

            drawPixel(pixel);
        }
    }


    while(p.winflag == 0 && p.loseflag == 0){
        /*	Back ground	*/
        for (int y = 64; y < 320; y++)
        {
            for (int j = 0; j < 20; j++) 
            {
                for (int x = 0; x < 64; x++){
                    pixel->color = ExMapPtr[y*width+(j*64)+x-(19*64)+(w*64)]; 
                    pixel->x = x+(j*64);
                    pixel->y = y;

                    drawPixel(pixel);
                }
            }
        }
        if (w == 19){
            w = 0;
        }else{
            w++;
        }

        
        if (p.stage == 1){
            delayMicroseconds(170000);
        }else if (p.stage ==2 ){
            delayMicroseconds(150000);
        }else if (p.stage == 3){
            delayMicroseconds(130000);
        }else{
            delayMicroseconds(100000);
        }
        
    }
}

void DrawingHeart(unsigned int *gpioPtr, Pixel *pixel){

    short int *HeartPtr=(short int *) HeartImage.pixel_data;
    int w;
    while(1){
        
        for (int j = 0; j < 5; j++){

            if(j < p.lives){
                w = 0;
                for (int y = 0; y < 64; y++){
                    for (int x = 0; x < 64; x++) {
                    
                        pixel->color = HeartPtr[w];
                        pixel->x = x+(j*64);
                        pixel->y = y;
                
                        drawPixel(pixel);
                        w++;
                    }
            
                }

            }else if (j >= p.lives){
                for (int y = 0; y < 64; y++){
                    for (int x = 0; x < 64; x++) {
                    
                        pixel->color = 0x000;
                        pixel->x = x+(j*64);
                        pixel->y = y;
                
                        drawPixel(pixel);
                    }
            
                }
            }
        }

        if (p.lives == 0){
            for (int j = 0; j < 5; j++){
                for (int y = 0; y < 64; y++){
                    for (int x = 0; x < 64; x++) {
                    
                        pixel->color = 0x000;
                        pixel->x = x+(j*64);
                        pixel->y = y;
                
                        drawPixel(pixel);
                    }
            
                }
            }
            break;
        }

    }

    p.loseflag = 1;

}

void DrawingMario(unsigned int *gpioPtr, Pixel *pixel){
    short int *ExMapPtr=(short int *) ExMapImage.pixel_data;
    short int *MarioPtr=(short int *) MarioImage.pixel_data;

    int w;

    /* currenct location */
	p.current_x = 192;
	p.current_y = 448;

    p.previous_x = p.current_x;
    p.previous_y = p.current_y;
    

    while(p.winflag == 0 && p.loseflag == 0){
        
		Read_SNES(gpioPtr);

        w = 0;

		for (w = 0; w < 16; w++) {
            if (buttons[w] == 0) {      // if button is pressed
                ApplyChange(w+1, &p.current_x, &p.current_y, &p.previous_x, &p.previous_y);
                break;
			}	
		}
       

		/*	Fixing Background	*/
		for (int y = p.previous_y; y < p.previous_y + 64; y++)
		{
			for (int x = p.previous_x; x < p.previous_x + 64; x++) 
			{
                
                pixel->color = ExMapPtr[y*width+x]; 
				pixel->x = x;
				pixel->y = y;
		
				drawPixel(pixel);
		
			}
		
		}

        w = 0;

		/*	Mario location	*/
		for (int y = p.current_y; y < p.current_y + 64; y++)
		{
			for (int x = p.current_x; x < p.current_x + 64; x++) 
			{
                
                if (MarioPtr[w] == 0x000){
                    pixel->color = ExMapPtr[y*width+x];
                }else{
                    pixel->color = MarioPtr[w];
                } 
				pixel->x = x;
				pixel->y = y;
		
				drawPixel(pixel);
                w++;
			}
		
		}
        if (p.stage == 1){
            delayMicroseconds(170000);
        }else if (p.stage ==2 ){
            delayMicroseconds(150000);
        }else if (p.stage == 3){
            delayMicroseconds(130000);
        }else{
            delayMicroseconds(100000);
        }


	}
}

void DrawingCoin(unsigned int *gpioPtr, Pixel *pixel, int lane){

    short int *ExMapPtr=(short int *) ExMapImage.pixel_data;
    short int *CoinPtr=(short int *) CoinImage.pixel_data;

    int w;
    int coin_x = 19;

    while(p.winflag == 0 && p.loseflag == 0){
        if (lane == 5 || lane == 6 || lane == 7 || lane == 8 || lane == 9 || lane == 10){
            /*	Fixing Background	*/
            for (int y = lane*64; y < lane*64 + 64; y++)
            {
                for (int x = (coin_x+1)*64 ; x < (coin_x+1)*64 + 64; x++) 
                {
                    
                    pixel->color = ExMapPtr[y*width+x]; 
                    pixel->x = x;
                    pixel->y = y;
            
                    drawPixel(pixel);
            
                }
            
            }

            if (coin_x == -1){
                break;
            }

            w = 0;

            if (p.current_x == coin_x*64 && p.current_y == lane*64){
                if ((p.score + 10)>9999){
                    p.score = 9999;
                }else{
                    p.score = p.score + 10;
                }
                
                break;
            }

            /* Coin Location */
            for (int y = lane*64; y < lane*64 + 64; y++)
            {
                for (int x = coin_x*64; x < coin_x*64 + 64; x++) 
                {
                    if (CoinPtr[w] == 0x000){
                        pixel->color = ExMapPtr[y*width+x];
                    }else{
                        pixel->color = CoinPtr[w];
                    }
                    pixel->x = x;
                    pixel->y = y;
            
                    drawPixel(pixel);
                    w++;
                }
            
            }
            if (p.stage == 1){
                delayMicroseconds(170000);
            }else if (p.stage ==2 ){
                delayMicroseconds(150000);
            }else if (p.stage == 3){
                delayMicroseconds(130000);
            }else{
                delayMicroseconds(100000);
            }

            coin_x = coin_x - 1;
        }else{
            break;
        }
    }

}

void DrawingScoreNumbers(unsigned int *gpioPtr, Pixel *pixel, int Number, int digit){

    short int *ZEROPtr=(short int *) ZEROImage.pixel_data;
    short int *ONEPtr=(short int *) ONEImage.pixel_data;
    short int *TWOPtr=(short int *) TWOImage.pixel_data;
    short int *THREEPtr=(short int *) THREEImage.pixel_data;
    short int *FOURPtr=(short int *) FOURImage.pixel_data;
    short int *FIVEPtr=(short int *) FIVEImage.pixel_data;
    short int *SIXPtr=(short int *) SIXImage.pixel_data;
    short int *SEVENPtr=(short int *) SEVENImage.pixel_data;
    short int *EIGHTPtr=(short int *) EIGHTImage.pixel_data;
    short int *NINEPtr=(short int *) NINEImage.pixel_data;

    short int *SCOREPtr=(short int *) SCOREImage.pixel_data;


    for (int y = 0; y < 64; y++){
        for (int x = 0; x < 64; x++){
            
            pixel->color = 0x000;
            pixel->x = x+704;
            pixel->y = y;
        
            drawPixel(pixel);
        }
    
    }

    int w = 0;

    for (int y = 0; y < 64; y++){
        for (int x = 0; x < 256; x++){
            
            pixel->color = SCOREPtr[w];
            pixel->x = x+768;
            pixel->y = y;
        
            drawPixel(pixel);
            w++;
        }
    
    }

    w = 0 ;

    if (Number == 0){
        for (int y = 0; y < 64; y++){
            for (int x = 0; x < 64; x++){
                
                pixel->color = ZEROPtr[w];
                if (digit == 1){
                    pixel->x = x+1216;
                }else if (digit == 10){
                    pixel->x = x+1152;
                }else if (digit == 100){
                    pixel->x = x+1088;
                }else if (digit == 1000){
                    pixel->x = x+1024;
                }
                pixel->y = y;
            
                drawPixel(pixel);
                w++;
            }
		
	    }
    }else if (Number == 1){
        for (int y = 0; y < 64; y++){
            for (int x = 0; x < 64; x++){
                
                pixel->color = ONEPtr[w];
                if (digit == 1){
                    pixel->x = x+1216;
                }else if (digit == 10){
                    pixel->x = x+1152;
                }else if (digit == 100){
                    pixel->x = x+1088;
                }else if (digit == 1000){
                    pixel->x = x+1024;
                }
                pixel->y = y;
        
                drawPixel(pixel);
                w++;
            }
    
        }
    }else if (Number == 2){
        for (int y = 0; y < 64; y++){
            for (int x = 0; x < 64; x++){
                
                pixel->color = TWOPtr[w];
                if (digit == 1){
                    pixel->x = x+1216;
                }else if (digit == 10){
                    pixel->x = x+1152;
                }else if (digit == 100){
                    pixel->x = x+1088;
                }else if (digit == 1000){
                    pixel->x = x+1024;
                }
                pixel->y = y;
        
                drawPixel(pixel);
                w++;
            }
    
        }
    }else if ( Number == 3){
        for (int y = 0; y < 64; y++){
            for (int x = 0; x < 64; x++){
                
                pixel->color = THREEPtr[w];
                if (digit == 1){
                    pixel->x = x+1216;
                }else if (digit == 10){
                    pixel->x = x+1152;
                }else if (digit == 100){
                    pixel->x = x+1088;
                }else if (digit == 1000){
                    pixel->x = x+1024;
                }
                pixel->y = y;
        
                drawPixel(pixel);
                w++;
            }
        }
    }else if (Number == 4){
        for (int y = 0; y < 64; y++){
            for (int x = 0; x < 64; x++){
                
                pixel->color = FOURPtr[w];
                if (digit == 1){
                    pixel->x = x+1216;
                }else if (digit == 10){
                    pixel->x = x+1152;
                }else if (digit == 100){
                    pixel->x = x+1088;
                }else if (digit == 1000){
                    pixel->x = x+1024;
                }
                pixel->y = y;
        
                drawPixel(pixel);
                w++;
            }
    
        }
    }else if (Number == 5){
        for (int y = 0; y < 64; y++){
            for (int x = 0; x < 64; x++){
                
                pixel->color = FIVEPtr[w];
               if (digit == 1){
                    pixel->x = x+1216;
                }else if (digit == 10){
                    pixel->x = x+1152;
                }else if (digit == 100){
                    pixel->x = x+1088;
                }else if (digit == 1000){
                    pixel->x = x+1024;
                }
                pixel->y = y;
        
                drawPixel(pixel);
                w++;
            }
    
        }
    }else if (Number == 6){
        for (int y = 0; y < 64; y++){
            for (int x = 0; x < 64; x++){
                
                pixel->color = SIXPtr[w];
                if (digit == 1){
                    pixel->x = x+1216;
                }else if (digit == 10){
                    pixel->x = x+1152;
                }else if (digit == 100){
                    pixel->x = x+1088;
                }else if (digit == 1000){
                    pixel->x = x+1024;
                }
                pixel->y = y;
        
                drawPixel(pixel);
                w++;
            }
    
        }
    }else if (Number == 7){
        for (int y = 0; y < 64; y++){
            for (int x = 0; x < 64; x++){
                
                pixel->color = SEVENPtr[w];
                if (digit == 1){
                    pixel->x = x+1216;
                }else if (digit == 10){
                    pixel->x = x+1152;
                }else if (digit == 100){
                    pixel->x = x+1088;
                }else if (digit == 1000){
                    pixel->x = x+1024;
                }
                pixel->y = y;
        
                drawPixel(pixel);
                w++;
            }
    
        }
    }else if (Number == 8){
        for (int y = 0; y < 64; y++){
            for (int x = 0; x < 64; x++){
                
                pixel->color = EIGHTPtr[w];
                if (digit == 1){
                    pixel->x = x+1216;
                }else if (digit == 10){
                    pixel->x = x+1152;
                }else if (digit == 100){
                    pixel->x = x+1088;
                }else if (digit == 1000){
                    pixel->x = x+1024;
                }
                pixel->y = y;
        
                drawPixel(pixel);
                w++;
            }
    
        }
    }else if (Number == 9){
        for (int y = 0; y < 64; y++){
            for (int x = 0; x < 64; x++){
                
                pixel->color = NINEPtr[w];
                if (digit == 1){
                    pixel->x = x+1216;
                }else if (digit == 10){
                    pixel->x = x+1152;
                }else if (digit == 100){
                    pixel->x = x+1088;
                }else if (digit == 1000){
                    pixel->x = x+1024;
                }
                pixel->y = y;
        
                drawPixel(pixel);
                w++;
            }
    
        }
    }
}

void DrawingTimeNumbers(unsigned int *gpioPtr, Pixel *pixel, int Number, int digit){
    short int *ZEROPtr=(short int *) ZEROImage.pixel_data;
    short int *ONEPtr=(short int *) ONEImage.pixel_data;
    short int *TWOPtr=(short int *) TWOImage.pixel_data;
    short int *THREEPtr=(short int *) THREEImage.pixel_data;
    short int *FOURPtr=(short int *) FOURImage.pixel_data;
    short int *FIVEPtr=(short int *) FIVEImage.pixel_data;
    short int *SIXPtr=(short int *) SIXImage.pixel_data;
    short int *SEVENPtr=(short int *) SEVENImage.pixel_data;
    short int *EIGHTPtr=(short int *) EIGHTImage.pixel_data;
    short int *NINEPtr=(short int *) NINEImage.pixel_data;
    short int *TIMEPtr=(short int *) TIMEImage.pixel_data;

    int w = 0;

    for (int y = 0; y < 64; y++){
        for (int x = 0; x < 192; x++){
            
            pixel->color = TIMEPtr[w];
            pixel->x = x+320;
            pixel->y = y;
        
            drawPixel(pixel);
            w++;
        }
    
    }

    w = 0;

    if (Number == 0){
        for (int y = 0; y < 64; y++){
            for (int x = 0; x < 64; x++){
                
                pixel->color = ZEROPtr[w];
                if (digit == 1){
                    pixel->x = x+640;
                }else if (digit == 10){
                    pixel->x = x+576;
                }else if (digit == 100){
                    pixel->x = x+512;
                }
                pixel->y = y;
            
                drawPixel(pixel);
                w++;
            }
		
	    }
    }else if (Number == 1){
        for (int y = 0; y < 64; y++){
            for (int x = 0; x < 64; x++){
                
                pixel->color = ONEPtr[w];
                if (digit == 1){
                    pixel->x = x+640;
                }else if (digit == 10){
                    pixel->x = x+576;
                }else if (digit == 100){
                    pixel->x = x+512;
                }
                pixel->y = y;
        
                drawPixel(pixel);
                w++;
            }
    
        }
    }else if (Number == 2){
        for (int y = 0; y < 64; y++){
            for (int x = 0; x < 64; x++){
                
                pixel->color = TWOPtr[w];
                if (digit == 1){
                    pixel->x = x+640;
                }else if (digit == 10){
                    pixel->x = x+576;
                }else if (digit == 100){
                    pixel->x = x+512;
                }
                pixel->y = y;
        
                drawPixel(pixel);
                w++;
            }
    
        }
    }else if ( Number == 3){
        for (int y = 0; y < 64; y++){
            for (int x = 0; x < 64; x++){
                
                pixel->color = THREEPtr[w];
                if (digit == 1){
                    pixel->x = x+640;
                }else if (digit == 10){
                    pixel->x = x+576;
                }else if (digit == 100){
                    pixel->x = x+512;
                }
                pixel->y = y;
        
                drawPixel(pixel);
                w++;
            }
        }
    }else if (Number == 4){
        for (int y = 0; y < 64; y++){
            for (int x = 0; x < 64; x++){
                
                pixel->color = FOURPtr[w];
                if (digit == 1){
                    pixel->x = x+640;
                }else if (digit == 10){
                    pixel->x = x+576;
                }else if (digit == 100){
                    pixel->x = x+512;
                }
                pixel->y = y;
        
                drawPixel(pixel);
                w++;
            }
    
        }
    }else if (Number == 5){
        for (int y = 0; y < 64; y++){
            for (int x = 0; x < 64; x++){
                
                pixel->color = FIVEPtr[w];
                if (digit == 1){
                    pixel->x = x+640;
                }else if (digit == 10){
                    pixel->x = x+576;
                }else if (digit == 100){
                    pixel->x = x+512;
                }
                pixel->y = y;
        
                drawPixel(pixel);
                w++;
            }
    
        }
    }else if (Number == 6){
        for (int y = 0; y < 64; y++){
            for (int x = 0; x < 64; x++){
                
                pixel->color = SIXPtr[w];
                if (digit == 1){
                    pixel->x = x+640;
                }else if (digit == 10){
                    pixel->x = x+576;
                }else if (digit == 100){
                    pixel->x = x+512;
                }
                pixel->y = y;
        
                drawPixel(pixel);
                w++;
            }
    
        }
    }else if (Number == 7){
        for (int y = 0; y < 64; y++){
            for (int x = 0; x < 64; x++){
                
                pixel->color = SEVENPtr[w];
                if (digit == 1){
                    pixel->x = x+640;
                }else if (digit == 10){
                    pixel->x = x+576;
                }else if (digit == 100){
                    pixel->x = x+512;
                }
                pixel->y = y;
        
                drawPixel(pixel);
                w++;
            }
    
        }
    }else if (Number == 8){
        for (int y = 0; y < 64; y++){
            for (int x = 0; x < 64; x++){
                
                pixel->color = EIGHTPtr[w];
                if (digit == 1){
                    pixel->x = x+640;
                }else if (digit == 10){
                    pixel->x = x+576;
                }else if (digit == 100){
                    pixel->x = x+512;
                }
                pixel->y = y;
        
                drawPixel(pixel);
                w++;
            }
    
        }
    }else if (Number == 9){
        for (int y = 0; y < 64; y++){
            for (int x = 0; x < 64; x++){
                
                pixel->color = NINEPtr[w];
                if (digit == 1){
                    pixel->x = x+640;
                }else if (digit == 10){
                    pixel->x = x+576;
                }else if (digit == 100){
                    pixel->x = x+512;
                }
                pixel->y = y;
        
                drawPixel(pixel);
                w++;
            }
    
        }
    }
}

void DrawingScore(unsigned int *gpioPtr, Pixel *pixel){

    int one_digit = 0;
    int ten_digit = 0;
    int hundred_digit = 0;
    int thousand_digit = 0;

    while(1){

        one_digit = p.score % 10;
        DrawingScoreNumbers(gpioPtr,pixel,one_digit,1);

        ten_digit = (p.score/10) % 10;
        DrawingScoreNumbers(gpioPtr,pixel,ten_digit,10);

        hundred_digit = (p.score/100) % 10;
        DrawingScoreNumbers(gpioPtr,pixel,hundred_digit,100);

        thousand_digit = p.score/1000;
        DrawingScoreNumbers(gpioPtr,pixel,thousand_digit,1000);
        
        
    }


}

void DrawingTime(unsigned int *gpioPtr, Pixel *pixel){
    int one_digit = 0;
    int ten_digit = 0;
    int hundred_digit = 0;

    while(1){

        one_digit = p.time % 10;
        DrawingTimeNumbers(gpioPtr,pixel,one_digit,1);

        ten_digit = (p.time/10) % 10;
        DrawingTimeNumbers(gpioPtr,pixel,ten_digit,10);

        hundred_digit = p.time/100;
        DrawingTimeNumbers(gpioPtr,pixel,hundred_digit,100);
        
    }
}

void DrawingGreenShell(unsigned int *gpioPtr, Pixel *pixel, int lane){
    short int *ExMapPtr=(short int *) ExMapImage.pixel_data;
    short int *GreenShellPtr=(short int *) GreenShellImage.pixel_data;

    int w;
    int greenShell_x = 0;

    while(greenShell_x != -2){
        if (lane == 5 || lane == 6 || lane == 7 || lane == 8 || lane == 9 || lane == 10){
            for (int y = lane*64; y < lane*64 + 64; y++)
            {
                for (int x = (greenShell_x-1)*64 ; x < (greenShell_x-1)*64 + 64; x++) 
                {
                    
                    pixel->color = ExMapPtr[y*width+x]; 
                    pixel->x = x;
                    pixel->y = y;
            
                    drawPixel(pixel);
            
                }
            
            }

            if (greenShell_x == 20){
                break;
            }

            w = 0;

            if (p.current_x == greenShell_x*64 && p.current_y == lane*64){
                p.lives = p.lives - 1;
                p.distance = p.distance + 2;
                break;
            }

            /* Shell Location */
            for (int y = lane*64; y < lane*64 + 64; y++)
            {
                for (int x = greenShell_x*64; x < greenShell_x*64 + 64; x++) 
                {
                    if (GreenShellPtr[w] == 0x000){
                        pixel->color = ExMapPtr[y*width+x];
                    }else{
                        pixel->color = GreenShellPtr[w];
                    }
                    pixel->x = x;
                    pixel->y = y;
            
                    drawPixel(pixel);
                    w++;
                }
            
            }
            if (p.stage == 1){
                delayMicroseconds(75000);
            }else if (p.stage ==2 ){
                delayMicroseconds(65000);
            }else if (p.stage == 3){
                delayMicroseconds(55000);
            }else{
                delayMicroseconds(45000);
            }

            greenShell_x = greenShell_x + 1;
        }else{
            break;
        }
    }
}

void DrawingSmallHeart(unsigned int *gpioPtr, Pixel *pixel, int lane){

    short int *ExMapPtr=(short int *) ExMapImage.pixel_data;
    short int *smallHeartPtr=(short int *) smallHeartImage.pixel_data;

    int w;
    int heart_x = 19;

    while(p.winflag == 0 && p.loseflag == 0){
        if (lane == 5 || lane == 6 || lane == 7 || lane == 8 || lane == 9 || lane == 10){
            /*	Fixing Background	*/
            for (int y = lane*64; y < lane*64 + 64; y++)
            {
                for (int x = (heart_x+1)*64 ; x < (heart_x+1)*64 + 64; x++) 
                {
                    
                    pixel->color = ExMapPtr[y*width+x]; 
                    pixel->x = x;
                    pixel->y = y;
            
                    drawPixel(pixel);
                }
            }

            if (heart_x == -1){
                break;
            }

            w = 0;

            if (p.current_x == heart_x*64 && p.current_y == lane*64){
                if (p.lives < 5){
                    p.lives = p.lives + 1;
                }

                break;
            }

            /* Coin Location */
            for (int y = lane*64; y < lane*64 + 64; y++)
            {
                for (int x = heart_x*64; x < heart_x*64 + 64; x++) 
                {
                    if (smallHeartPtr[w] == 0x000){
                        pixel->color = ExMapPtr[y*width+x];
                    }else{
                        pixel->color = smallHeartPtr[w];
                    }
                    pixel->x = x;
                    pixel->y = y;
            
                    drawPixel(pixel);
                    w++;
                }
            
            }
            if (p.stage == 1){
                delayMicroseconds(170000);
            }else if (p.stage ==2 ){
                delayMicroseconds(150000);
            }else if (p.stage == 3){
                delayMicroseconds(130000);
            }else{
                delayMicroseconds(100000);
            }

            heart_x = heart_x - 1;
        }else{
            break;
        }
    }
}

void DrawingRocket(unsigned int *gpioPtr, Pixel *pixel, int lane){
    short int *ExMapPtr=(short int *) ExMapImage.pixel_data;
    short int *RocketPtr=(short int *) RocketImage.pixel_data;

    int w;
    int rocket_x = 19;

    while(rocket_x != -2){
        if (lane == 5 || lane == 6 || lane == 7 || lane == 8 || lane == 9 || lane == 10){
            for (int y = lane*64; y < lane*64 + 64; y++)
            {
                for (int x = (rocket_x+1)*64 ; x < (rocket_x+1)*64 + 64; x++) 
                {
                    
                    pixel->color = ExMapPtr[y*width+x]; 
                    pixel->x = x;
                    pixel->y = y;
            
                    drawPixel(pixel);
            
                }
            
            }

            if (rocket_x == -1){
                break;
            }

            w = 0;

            if (p.current_x == rocket_x*64 && p.current_y == lane*64){
                p.lives = p.lives - 1;
                p.distance = p.distance + 2;
                break;
            }

            /* Shell Location */
            for (int y = lane*64; y < lane*64 + 64; y++)
            {
                for (int x = rocket_x*64; x < rocket_x*64 + 64; x++) 
                {
                    if (RocketPtr[w] == 0x000){
                        pixel->color = ExMapPtr[y*width+x];
                    }else{
                        pixel->color = RocketPtr[w];
                    }
                    pixel->x = x;
                    pixel->y = y;
            
                    drawPixel(pixel);
                    w++;
                }
            
            }
            if (p.stage == 1){
                delayMicroseconds(75000);
            }else if (p.stage ==2 ){
                delayMicroseconds(65000);
            }else if (p.stage == 3){
                delayMicroseconds(55000);
            }else{
                delayMicroseconds(45000);
            }

            rocket_x = rocket_x - 1;
        }else{
            break;
        }
    }
}

void *runMap(void *unused){
    while (p.stage <5){
        unsigned int *gpioPtr = getGPIOPtr();
        Init_GPIO(CLK, 1, gpioPtr);         // init pin 11 to output
        Init_GPIO(LAT, 1, gpioPtr);         // init pin 9 to output
        Init_GPIO(DAT, 0, gpioPtr);

        /* initialize + get FBS */
        framebufferstruct = initFbInfo();
        
        /* initialize a pixel */
        Pixel *pixel;
        pixel = malloc(sizeof(Pixel));

        short int *ExMapPtr=(short int *) ExMapImage.pixel_data;

        for (int y = 320; y < 720; y++) {
            for (int x = 0; x < 1280; x++){
                pixel->color = ExMapPtr[y*width+x]; 
                pixel->x = x;
                pixel->y = y;

                drawPixel(pixel);
            }
        }

        DrawingMap(gpioPtr, pixel);    

        
    }
    pthread_exit(0);
}

void *runMario(void *unused){
    while (p.stage < 5){
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

       
    }
    pthread_exit(0);
}

void *runHeart(void *unused){
    while (p.stage < 5){
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
    }
    pthread_exit(0);
}

void *runCoin(void *unused){
    while (p.stage < 5){
        unsigned int *gpioPtr = getGPIOPtr();
        Init_GPIO(CLK, 1, gpioPtr);         // init pin 11 to output
        Init_GPIO(LAT, 1, gpioPtr);         // init pin 9 to output
        Init_GPIO(DAT, 0, gpioPtr);

        /* initialize + get FBS */
        framebufferstruct = initFbInfo();
        
        /* initialize a pixel */
        Pixel *pixel;
        pixel = malloc(sizeof(Pixel));

        while ((p.winflag == 0) && (p.loseflag == 0)){
            if (p.distance > 0){
                if (p.distance < 70){
                    srand(time(NULL));
                    int random = 0;
                    random = rand()%6;

                    DrawingCoin(gpioPtr, pixel, random+5); 
                    sleep(2); 
                }
            }
        }

        
    }
    pthread_exit(0);
}

void *runScore(void *unused){
    while (p.stage < 5){
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
    }

    pthread_exit(0);
}

void *runTime(void *unused){
    while (p.stage < 5){
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

    }   

    pthread_exit(0);
}

void *tickTime(void *unused){
    while (p.stage < 5){
        while((p.time != 0) && p.winflag == 0 && p.loseflag == 0){
            sleep(1);
            p.time = p.time - 1;
        }

        p.loseflag = 1;
    }

    pthread_exit(0);
}

void *runGreenShell(void *unused){
    while (p.stage < 5){
        unsigned int *gpioPtr = getGPIOPtr();
        Init_GPIO(CLK, 1, gpioPtr);         // init pin 11 to output
        Init_GPIO(LAT, 1, gpioPtr);         // init pin 9 to output
        Init_GPIO(DAT, 0, gpioPtr);

        /* initialize + get FBS */
        framebufferstruct = initFbInfo();
        
        /* initialize a pixel */
        Pixel *pixel;
        pixel = malloc(sizeof(Pixel));

        while ((p.winflag == 0) && (p.loseflag == 0)){
            if (p.distance > 0){
                srand(time(NULL));
                int random = 0;
                random = rand()%9;
                if (random == 0){
                    random = 10;
                }

                DrawingGreenShell(gpioPtr, pixel, random);

                sleep(1); 
            }   
        }

    }   

    pthread_exit(0);
}

void *runRocket(void *unused){
    while (p.stage < 5){
        unsigned int *gpioPtr = getGPIOPtr();
        Init_GPIO(CLK, 1, gpioPtr);         // init pin 11 to output
        Init_GPIO(LAT, 1, gpioPtr);         // init pin 9 to output
        Init_GPIO(DAT, 0, gpioPtr);

        /* initialize + get FBS */
        framebufferstruct = initFbInfo();
        
        /* initialize a pixel */
        Pixel *pixel;
        pixel = malloc(sizeof(Pixel));

        while ((p.winflag == 0) && (p.loseflag == 0)){
            if (p.distance > 0){
                srand(time(NULL));
                int random = 0;
                random = rand()%8;
                if (random == 0){
                    random = 10;
                }

                DrawingRocket(gpioPtr, pixel, random);

                sleep(1);
            }    
        }

    }   

    pthread_exit(0);
}

void *distance(void *unused){
    while(p.stage < 5){
        while((p.time != 0) && p.winflag == 0 && p.loseflag == 0 && p.distance != 0){
            sleep(1);
            p.distance = p.distance - 1;
        }    
        

    }
    pthread_exit(0);

}

void *runSmallHeart(void *unused){
    while (p.stage < 5){
        unsigned int *gpioPtr = getGPIOPtr();
        Init_GPIO(CLK, 1, gpioPtr);         // init pin 11 to output
        Init_GPIO(LAT, 1, gpioPtr);         // init pin 9 to output
        Init_GPIO(DAT, 0, gpioPtr);

        /* initialize + get FBS */
        framebufferstruct = initFbInfo();
        
        /* initialize a pixel */
        Pixel *pixel;
        pixel = malloc(sizeof(Pixel));

        while ((p.winflag == 0) && (p.loseflag == 0)){
            if (p.distance > 0){
                if (p.distance < 70){
                    srand(time(NULL));
                    int random = 0;
                    random = rand()%7;

                    DrawingSmallHeart(gpioPtr, pixel, random); 
                    sleep(10); 
                }  
            }
        }

    }

    pthread_exit(0);
}

void *runFinish(void *unused){
    while (p.stage < 5){
        unsigned int *gpioPtr = getGPIOPtr();
        Init_GPIO(CLK, 1, gpioPtr);         // init pin 11 to output
        Init_GPIO(LAT, 1, gpioPtr);         // init pin 9 to output
        Init_GPIO(DAT, 0, gpioPtr);

        /* initialize + get FBS */
        framebufferstruct = initFbInfo();
        
        /* initialize a pixel */
        Pixel *pixel;
        pixel = malloc(sizeof(Pixel));

         while ((p.winflag == 0) && (p.loseflag == 0)){
            if (p.distance == 0){

                DrawingFinish(gpioPtr, pixel);  
            }  
        }

    }

    pthread_exit(0);
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
    
    opening(gpioPtr, pixel);

    mainMenu(gpioPtr, pixel);

    p.winflag = 0;
    p.loseflag = 0;
    p.lives = 4;
    p.score = 0;
    p.time = 120;
    p.distance = 10;
    p.stage = 1;


    pthread_attr_t attr;

    pthread_t tmap, tmario, theart, tcoin, tscore, ttime, trtime, tgreenshell, tdistance, tsmallHeart, trocket, tfinish;


    pthread_attr_init(&attr);
    pthread_create(&tmap,&attr, runMap, NULL);
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


    //pthread_join(tmap,NULL);
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
	
	/* free pixel's allocated memory */
	free(pixel);
	pixel = NULL;
	munmap(framebufferstruct.fptr, framebufferstruct.screenSize);
	
	return 0;
}



/* Draw a pixel */
void drawPixel(Pixel *pixel){
	long int location = (pixel->x +framebufferstruct.xOff) * (framebufferstruct.bits/8) +
                       (pixel->y+framebufferstruct.yOff) * framebufferstruct.lineLength;
	*((unsigned short int*)(framebufferstruct.fptr + location)) = pixel->color;
}


