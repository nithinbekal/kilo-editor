#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#define CTRL_KEY(k) ((k) & 0x1f)

struct termios orig_termios;

void die(const char* s) {
  perror(s);
  exit(1);
}

/* Terminal */

void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) die("tcsetattr");
}

void enableRawMode() {
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) die("tcgetattr");
  atexit(disableRawMode);

  struct termios raw = orig_termios;

  raw.c_cflag |=  (CS8);
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_oflag &= ~(OPOST);

  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 10;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

char editorReadKey() {
  int nread;
  char c;

  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN) die("read");
  }

  return c;
}

/* Input */

void editorProcessKeyPress() {
  char c = editorReadKey();

  switch (c) {
  case CTRL_KEY('q'):
    exit(0);
    break;
  }
}

/* Output */

void editorRefreshScreen() {
  // function defined in unistd.h
  // "\xib" is the byte indicating escape char (27).
  // [2J is an escape sequence that clears the entire screen.
  // 4 indicates we're writing 4 bytes to the terminal
  write(STDIN_FILENO, "\x1b[2J", 4);

  // Reposition the curson to 1,1.
  write(STDIN_FILENO, "\x1b[H", 3);
}


int main() {
  enableRawMode();

  while (1) {
    editorRefreshScreen();
    editorProcessKeyPress();
  }

  return 0;
}
