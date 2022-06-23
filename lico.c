#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
// #include<string.h>
struct termios original_termios;

void disableRawMode(){
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
}

void enableRawMode(){
    tcgetattr(STDIN_FILENO, &original_termios);
    atexit(disableRawMode);

    struct termios raw = original_termios;
    raw.c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw); 

}

int main()
{
    enableRawMode();
    char c;
    // char text[50] = "";
    while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q'){
        //  strncat(text, &c, 1);
    }
    // printf(text, '\n');
    
    return 0;
}