// Shell prompt - export PS1="\[$(tput setaf 1)\]\[$(tput bold)\]soumyanil@text-editor: \[$(tput sgr0)\]"

// ------ includes ------

#include <ctype.h>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <errno.h>
// #include<string.h>

// ------ define ------

#define CTRL_KEY(k) ((k)&0x1f)

// ------ data ------

struct editorConfig
{
    int screenRows, screenColums;
    struct termios original_termios;
};

struct editorConfig E;

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
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.original_termios) == -1)
        die("tcsetattr");
}

void enableRawMode()
{
    if (tcgetattr(STDIN_FILENO, &E.original_termios) == -1)
        die("tcgetattr");

    atexit(disableRawMode);

    struct termios raw = E.original_termios;
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

int getCursorPositions(int *rows, int *columns)
{
    char buf[32];
    unsigned int i = 0;

    if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4)
        return -1;

    while (i < sizeof(buf))
    {
        if (read(STDIN_FILENO, &buf[i], 1) != 1)
            break;
        if (buf[i] == 'R')
            break;
        i++;
    }

    buf[i] = '\0';

    if (buf[0] != '\x1b' || buf[1] != '[')
        return -1;
    if (sscanf(&buf[2], "%d;%d", rows, columns) != 2)
        return -1;
    // printf("\r\n&buf[1] = '%s' \r\n", &buf[1]);

    // editorReadKey();
    return 0;
}

int getWindowSize(int *rows, int *cols)
{
    struct winsize ws;

    if ((ioctl(STDIN_FILENO, TIOCGWINSZ, &ws)) == -1 || ws.ws_col == 0)
    {
        if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12)
            return -1;
        return getCursorPositions(rows, cols);
    }
    else
    {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 1;
    }
}

// ------ output ------

void editorDrawRows()
{
    int y;
    for (y = 0; y < E.screenRows; y++)
    {
        write(STDOUT_FILENO, "~", 1);

        if (y < E.screenRows - 1)
        {
            write(STDOUT_FILENO, "\r\n", 2);
        }
    }
}

void editorRefreshScreen()
{
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);

    editorDrawRows();
    write(STDOUT_FILENO, "\x1b[H", 3);
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

void initEditor()
{
    if (getWindowSize(&E.screenRows, &E.screenColums) == -1)
        die("Get Window Size");
}

int main()
{
    enableRawMode();
    initEditor();

    while (1)
    {
        editorRefreshScreen();
        editorProcessKeypress();
    }

    return 0;
}