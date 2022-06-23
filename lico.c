#include <unistd.h>
#include <termios.h>
// #include<string.h>

void enableRawMode(){

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