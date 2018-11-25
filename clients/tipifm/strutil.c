#include "strutil.h"

#include <conio.h>
#include <string.h>

void getstr(int x, int y, char* var, int limit) {
  conio_cursorFlag = 1;
  gotoxy(x,y);
  cputs(var);

  unsigned char normal_cursor = conio_cursorChar;

  unsigned char key = 0;
  int idx = strlen(var);
  while(key != 13) {
    // should set cursor to current char
    conio_cursorChar = var[idx];
    if (conio_cursorChar == 32 || conio_cursorChar == 0) {
      conio_cursorChar = normal_cursor;
    }
    gotoxy(x+idx,y);
    key = cgetc();
    int delidx = 0;
    switch(key) {
      case 3: // F1 - delete
        delidx = idx;
        while(var[delidx] != 0) {
          var[delidx] = var[delidx+1];
          delidx++;
        }
        delidx = strlen(var);
        var[delidx] = 0;
        gotoxy(x,y);
        cputs(var);
        cputs(" ");
        break;
      case 7: // F3 - erase line
        idx = 0;
        for(delidx = 0; delidx<limit; delidx++) {
          var[delidx] = 0;
        }
        gotoxy(x,y);
        cclear(limit-x);
        break;
      case 8: // left arrow
        if (idx > 0 && idx < limit) {
          gotoxy(x+idx,y);
          cputc(var[idx]);
          idx--;
          gotoxy(x+idx,y);
        }
        break;
      case 9: // right arrow
        if (var[idx] != 0) {
          cputc(var[idx]);
          idx++;
        }
        break;
      case 13: // return
        break;
      default: // alpha numeric
        if (key >= 32 && key <= 122) {
          gotoxy(x+idx,y);
          var[idx] = key;
          cputc(var[idx]);
          if (idx < limit) {
            idx++;
          }
        }
        break;
    }
  }
  int i = limit - 1;
  while(var[i] == 0 || var[i] == ' ') {
    var[i] = 0;
    i--;
  }
}

int strcmp(const char* a, const char* b) {
  int i=0;
  do {
    if (a[i] == '\0') {
      return a[i] - b[i];
    }
    i++;
  } while(a[i] == b[i]);
  return a[i] - b[i];
}

char lowerchar(char c) {
  if (c >= 'A' && c <= 'Z') {
    return c - 'A' + 'a';
  }
  return c;
}

int strcmpi(const char* a, const char* b) {
  int i=0;
  char ch = lowerchar(a[i]);
  do {
    if (ch == '\0') {
      return ch - b[i];
    }
    i++;
    ch = lowerchar(a[i]);
  } while(ch == b[i]);
  return ch - b[i];
}

int indexof(const char* str, char c) {
  int i=0;
  while(str[i] != 0) {
    if (str[i] == c) {
      return i;
    }
    i++;
  }
  return -1;
}

int basic_strcmp(const char* basstr, const char* cstr) {
  int i = 0;
  do {
    if (basstr[0] == i) {
      return 0 - cstr[i];
    }
    i++;
  } while(basstr[i+1] == cstr[i]);
  return basstr[i+1] - cstr[i];
}

int basicToCstr(const char* str, char* buf) {
  int len = (int) str[0];
  for(int i=0; i<len; i++) {
    buf[i] = str[i+1];
  }
  buf[len] = 0;
  return len;
}

char *lasts;

char* strtok(char* str, char* delim) {
  int ch;

  if (str == 0)
	  str = lasts;
  do {
	  if ((ch = *str++) == '\0')
	    return 0;
  } while (strchr(delim, ch));
  --str;
  lasts = str + strcspn(str, delim);
  if (*lasts != 0)
	  *lasts++ = 0;
  return str;
}

char* strchr(char* str, int delim) 
{
  int x;

  while (1) {
	  x = *str++;
	  if (x == delim) {
	    return str - 1;
	  }
	  if (x == 0) {
	    return (char *) 0;
	  }
  }
}

int strcspn(char* string, char* chars) {
  char c, *p, *s;

  for (s = string, c = *s; c != 0; s++, c = *s) {
	  for (p = chars; *p != 0; p++) {
	    if (c == *p) {
		    return s-string;
	    }
	  }
  }
  return s-string;
}

void strset(char* buffer, char value, int limit) {
  for(int i=0; i<limit; i++) {
    buffer[i] = value;
  }
}