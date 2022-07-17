// Shell prompt - export PS1="\[$(tput setaf 1)\]\[$(tput bold)\]soumyanil@text-editor: \[$(tput sgr0)\]"

// ------ includes ------

#include <ctype.h>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <errno.h>
// #include<string.h>

// ------ define ------

#define CTRL_KEY(k) ((k)&0x1f)
#define LICO_VERSION "0.0.1"

enum editorKey
{
    ARROW_UP = 1000,
    ARROW_DOWN,
    ARROW_LEFT,
    ARROW_RIGHT,
};

// ------ data ------

struct editorConfig
{
    int cx, cy;
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

int editorReadKey()
{
    int nread;
    char c = '\0';

    while ((nread = read(STDIN_FILENO, &c, 1)) != 1)
    {
        if (nread == -1 && errno != EAGAIN)
            die("read");
    }

    if (c == '\x1b')
    {
        char seq[3];

        if (read(STDIN_FILENO, &seq[0], 1) != 1)
            return '\x1b';
        if (read(STDIN_FILENO, &seq[1], 1) != 1)
            return '\x1b';

        if (seq[0] == '[')
        {
            switch (seq[1])
            {
            case 'A':
                return ARROW_UP;

            case 'B':
                return ARROW_DOWN;

            case 'C':
                return ARROW_RIGHT;

            case 'D':
                return ARROW_LEFT;
            }
        }
        return '\x1b';
    }
    else
    {
        return c;
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

// ------ append buffer ------

struct abuf
{
    char *b;
    int len;
};

#define ABUF_INIT \
    {             \
        NULL, 0   \
    }

void abAppend(struct abuf *ab, const char *s, int len)
{
    char *new = realloc(ab->b, ab->len + len);

    if (new == NULL)
        return;

    memcpy(&new[ab->len], s, len);
    ab->b = new;
    ab->len += len;
}

void abFree(struct abuf *ab)
{
    free(ab->b);
}

// ------ output ------

void editorDrawRows(struct abuf *ab)
{
    int y;
    for (y = 0; y < E.screenRows; y++)
    {
        // write(STDOUT_FILENO, "~", 1);
        if (y == E.screenRows / 3)
        {
            char welcome[80];
            int welcomelen = snprintf(welcome, sizeof(welcome), "LICO EDITOR -- Version %s", LICO_VERSION);
            if (welcomelen > E.screenColums)
                welcomelen = E.screenColums;
            int padding = (E.screenColums - welcomelen) / 2;
            if (padding)
            {
                abAppend(ab, "~", 1);
                padding--;
            }
            while (padding--)
                abAppend(ab, " ", 1);
            abAppend(ab, welcome, welcomelen);
        }
        else
        {
            abAppend(ab, "~", 1);
        }

        abAppend(ab, "\x1b[K", 3); // erase in line
        if (y < E.screenRows - 1)
        {
            // write(STDOUT_FILENO, "\r\n", 2);
            abAppend(ab, "\r\n", 2);
        }
    }
}

void editorRefreshScreen()
{
    // write(STDOUT_FILENO, "\x1b[2J", 4);
    // write(STDOUT_FILENO, "\x1b[H", 3);
    // write(STDOUT_FILENO, "\x1b[H", 3);

    struct abuf ab = ABUF_INIT;

    abAppend(&ab, "\x1b[?25l", 6); // reset mode -cursor hiding
    // abAppend(&ab, "\x1b[2J", 4);   // clear entire screen
    abAppend(&ab, "\x1b[H", 3);

    editorDrawRows(&ab);
    // abAppend(&ab, "\x1b[H", 3);

    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", E.cx + 1, E.cy + 1);
    abAppend(&ab, buf, strlen(buf));

    abAppend(&ab, "\x1b[?25h", 6); // set mode - cursor reappear(might not work in some terminals)

    write(STDOUT_FILENO, ab.b, ab.len);
    abFree(&ab);
}

// ------ input ------

void editorMoveCursor(int key)
{
    switch (key)
    {
    case ARROW_UP:
        E.cx--;
        break;

    case ARROW_DOWN:
        E.cx++;
        break;

    case ARROW_LEFT:
        E.cy--;
        break;

    case ARROW_RIGHT:
        E.cy++;
        break;

    default:
        break;
    }
}

void editorProcessKeypress()
{
    int c = editorReadKey();

    switch (c)
    {
    case CTRL_KEY('q'):
        write(STDOUT_FILENO, "\x1b[2J", 4);
        write(STDOUT_FILENO, "\x1b[E", 3);
        exit(0);
        break;

    case ARROW_UP:
    case ARROW_DOWN:
    case ARROW_LEFT:
    case ARROW_RIGHT:
        editorMoveCursor(c);
        break;
    }
}

// ------ init ------

void initEditor()
{
    E.cx = 0;
    E.cy = 0;

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