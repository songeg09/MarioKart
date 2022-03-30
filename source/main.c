#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "framebuffer.h"

#include <unistd.h>
#include <wiringPi.h>
#include "initGPIO.h"

#include <pthread.h>

#include "OpeningBackground.h"
#include "MainMenu.h"
#include "Arrow.h"

#include "Mario.h"
#include "ExMap.h"



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
        if (*c_y - 64 >= 0){
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
};

struct player p;

void DrawingMap(unsigned int *gpioPtr, Pixel *pixel){

    short int *ExMapPtr=(short int *) ExMapImage.pixel_data;
    i = 0;
    int check;
    while(1){
        /*	Back ground	*/
        for (int y = 0; y < height; y++)
        {
            for (int j = 0; j < 20; j++) 
            {
                check = (y*width)+(j*64)+1;
                if ((check % 1217) != 0){
                    for (int x = 0; x < 64; x++){
                        pixel->color = ExMapPtr[y*width+(j*64)+x+(i*64)]; 
                        pixel->x = x+(j*64);
                        pixel->y = y;

                        drawPixel(pixel);
                    }
                    // pixel->color = ExMapPtr[y*width+j+(i*64)]; 
                    // pixel->x = x;
                    // pixel->y = y;
                }else{
                    for (int x = 0; x < 64; x++){
                        pixel->color = ExMapPtr[y*width+(j*64)+x-(19*64)+(i*64)]; 
                        pixel->x = x+(j*64);
                        pixel->y = y;

                        drawPixel(pixel);
                    }
                    // pixel->color = ExMapPtr[y*width+j-(19*64)+(i*64)]; 
                    // pixel->x = x;
                    // pixel->y = y;
                }
                // pixel->color = ExMapPtr[y*width+x]; 
                // pixel->x = x;
                // pixel->y = y;
                
            }
        }
        if (i == 19){
                i = 0;
        }else{
            i++;
        }
        
        delayMicroseconds(150000);
    }
}

void DrawingMario(unsigned int *gpioPtr, Pixel *pixel){
    short int *ExMapPtr=(short int *) ExMapImage.pixel_data;
    short int *MarioPtr=(short int *) MarioImage.pixel_data;

    while(1){
        
		Read_SNES(gpioPtr);

        i = 0;

		for (i = 0; i < 16; i++) {
            if (buttons[i] == 0) {      // if button is pressed
                ApplyChange(i+1, &p.current_x, &p.current_y, &p.previous_x, &p.previous_y);
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

        i = 0;

		/*	Mario location	*/
		for (int y = p.current_y; y < p.current_y + 64; y++)
		{
			for (int x = p.current_x; x < p.current_x + 64; x++) 
			{
                
                if (MarioPtr[i] == 0x000){
                    pixel->color = ExMapPtr[y*width+x];
                }else{
                    pixel->color = MarioPtr[i];
                } 
				pixel->x = x;
				pixel->y = y;
		
				drawPixel(pixel);
                i++;
			}
		
		}
        delayMicroseconds(75000);


	}
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
    
    //opening(gpioPtr, pixel);

    //mainMenu(gpioPtr, pixel);


    // pthread_t tmap;
    // pthread_attr_t attr;

    // pthread_attr_init(&attr);
    // pthread_create(&tmap,&attr,)


	/* currenct location */
	p.current_x = 192;
	p.current_y = (height / 2) + 64;

    p.previous_x = p.current_x;
    p.previous_y = p.current_y;
    
    DrawingMap(gpioPtr, pixel);

    DrawingMario(gpioPtr, pixel);


	
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
