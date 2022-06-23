#include <unistd.h>
#include <termios.h>
#include <stdio.h>
// #include<string.h>

void enableRawMode(){

    struct termios raw;

    tcgetattr(STDIN_FILENO, &raw);

    raw.c_lflag &= ~(ECHO);

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw); 

}

int main()
{
    char c;
    // char text[50] = "";
    while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q'){
        //  strncat(text, &c, 1);
    }
    // printf(text, '\n');
    
    return 0;
}