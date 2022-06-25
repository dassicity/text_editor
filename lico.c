#include <ctype.h>
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
    raw.c_iflag &= ~(IXON);
    raw.c_lflag &= ~(ECHO | ICANON | ISIG);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw); 

}

int main()
{
    enableRawMode();
    char c;
    // char text[50] = "";
    while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q'){
        if ( iscntrl(c)){
            printf("%d\n", c);
        }
        else {
            printf("%d ('%c)\n", c, c);
        }
        //  strncat(text, &c, 1);
    }
    // printf(text, '\n');
    
    return 0;
}