// Shell prompt - export PS1="\[$(tput setaf 1)\]\[$(tput bold)\]soumyanil@text-editor: \[$(tput sgr0)\]"

// ------ includes ------

#include <ctype.h>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
// #include<string.h>

// ------ define ------

#define CTRL_KEY(k) ((k)&0x1f)

// ------ data ------

struct termios original_termios;

// ------ terminal ------

void die(const char *s)
{
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[E", 3);
    perror(s);
    exit(1);
}

void disableRawMode()
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios) == -1)
        die("tcsetattr");
}

void enableRawMode()
{
    if (tcgetattr(STDIN_FILENO, &original_termios) == -1)
        die("tcgetattr");

    atexit(disableRawMode);

    struct termios raw = original_termios;
    raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        die("tcsetattr");
}

char editorReadKey()
{
    int nread;
    char c = '\0';

    while ((nread = read(STDIN_FILENO, &c, 1)) != 1)
    {
        if (nread == -1 && errno != EAGAIN)
            die("read");
    }

    return c;
}

void editorRefreshScreen()
{
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[E", 3);
}

// ------ input ------

void editorProcessKeypress()
{
    char c = editorReadKey();

    switch (c)
    {
    case CTRL_KEY('q'):
        write(STDOUT_FILENO, "\x1b[2J", 4);
        write(STDOUT_FILENO, "\x1b[E", 3);
        exit(0);
        break;
    }
}

// ------ init ------

int main()
{
    enableRawMode();

    while (1)
    {
        editorRefreshScreen();
        editorProcessKeypress();
    }

    return 0;
}