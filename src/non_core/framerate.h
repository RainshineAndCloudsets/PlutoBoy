#ifndef FRAMERATE_H
#define FRAMERATE_H

#define DEFAULT_FPS_TIMES_10 597

extern int limiter; // FPS limiter ON or OFF

//Set a framerate and start the counter
void start_framerate(int fps);


/*Adjusts speed of game to the current framerate
 *by sleeping for the required time*/
void adjust_to_framerate();


#endif
