#include "main.h"

#include "conio.h"
#include "strutil.h"

const char* HELP_COMMANDS =
"Show help\n"
"\n"
"syntax: 'help' (cmd)\n"
"\n"
"cmd = 'cd' 'dir' 'drives' 'exit' 'ver' 'width'\n";

const char* HELP_CD =
"Change directory\n"
"\n"
"syntax: 'cd' (path|..)\n";

const char* HELP_DIR = 
"List files in current directory or given directory\n"
"\n"
"syntax: 'dir' [path]\n";

const char* HELP_DRIVES =
"List drives along with disambiguation\n"
"\n"
"syntax: 'drives'\n";

const char* HELP_MKDIR =
"Create a sub-directory in the current location\n"
"\n"
"syntax: 'mkdir' (name)\n";

const char* HELP_EXIT =
"Exit TIPIFM\n"
"\n"
"syntax: 'exist' | 'quit'\n";

const char* HELP_VER =
"Print information about TIPIFM\n"
"\n"
"syntax: 'ver'\n";

const char* HELP_WIDTH =
"Change display width. TIPIFM starts in 40 column mode\n"
"\n"
"syntax: 'width' (40|80)\n";

#define CMD_HELP(x,y) else if (0==strcmpi(x,tok)) { cprintf(y); }

void handleHelp() {
  char* tok = strtok(0, " ");
  if (tok == 0) {
    cprintf(HELP_COMMANDS);
  } 
  CMD_HELP("cd", HELP_CD)
  CMD_HELP("dir",HELP_DIR)
  CMD_HELP("drives",HELP_DRIVES)
  CMD_HELP("exit",HELP_EXIT)
  CMD_HELP("mkdir",HELP_MKDIR)
  CMD_HELP("quit",HELP_EXIT)
  CMD_HELP("ver",HELP_VER)
  CMD_HELP("width",HELP_WIDTH)
  else cprintf("unknown command: %s\n", tok);
}
