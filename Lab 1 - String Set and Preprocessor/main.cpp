// Use cpp to scan a file and print line numbers.
// Print out each input line read in, then strtok it for
// tokens.

#include <string>
using namespace std;

#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wait.h>
#include <unistd.h>


#include "string_set.h"
#include "auxlib.h"

const string CPP = "/usr/bin/cpp -nostdinc";
constexpr size_t LINESIZE = 1024;

// Chomp the last character from a buffer if it is delim.
void chomp(char* string, char delim) {
   size_t len = strlen(string);
   if (len == 0) return;
   char* nlpos = string + len - 1;
   if (*nlpos == delim) *nlpos = '\0';
}

// Run cpp against the lines of the file.
void cpplines(FILE* pipe, const char* filename) {
   int linenr = 1;
   char inputname[LINESIZE];
   strcpy(inputname, filename);
   for (;;) {
      char buffer[LINESIZE];
      char* fgets_rc = fgets(buffer, LINESIZE, pipe);
      if (fgets_rc == NULL) break;
      chomp(buffer, '\n');
      int sscanf_rc = sscanf(buffer, "# %d \"%[^\"]\"",
         &linenr, inputname);
      if (sscanf_rc == 2) {
         continue;
      }
      char* savepos = NULL;
      char* bufptr = buffer;
      for (int tokenct = 1;; ++tokenct) {
         char* token = strtok_r(bufptr, " \t\n", &savepos);    
         bufptr = NULL;
         if (token == NULL) break;                          
         string_set::intern(token);                      
      }
      ++linenr;
   }
}

int main(int argc, char** argv) {

   exec::execname = basename (argv[0]);
   exec::exit_status = EXIT_SUCCESS;

   int option;
   // int yy_flex_debug;
   // int yydebug;
   string dString = "";

   while ((option = getopt(argc, argv, "lyD:@:")) != -1) {
      switch (option) {
      case 'l':
         // yy_flex_debug = 1;  For assignment 2
         break;
      case 'y':
         //yydebug = 1;    For assignment 2
         break;
      case 'D':
         dString = " -D" + string(optarg);
         break;
      case '@':
         set_debugflags(optarg);
         break;
      default:
         errprintf("%: Invalid option used \n", optopt);
         break;
      }
   }

   if (optind >= argc) {
      eprintf("oc: fatal error: no input filename\n");
      exit(EXIT_FAILURE);
   }

   string fullFileName = basename(argv[optind]);

   if (fullFileName.size() <= 3 || fullFileName.substr(fullFileName.size() - 3, 3) != ".oc")
   {
      eprintf("oc: fatal error: invalid filename\n");
      exit(EXIT_FAILURE);
   }

   string filename = fullFileName.substr(0, fullFileName.size() - 3);

   string filenameWithStrSuffix = filename + ".str";
   FILE *fileStr = fopen(filenameWithStrSuffix.c_str(), "w");

   string command = CPP + dString + " " + fullFileName;

      FILE* pipe = popen(command.c_str(), "r");
      if (pipe == NULL) {
         exec::exit_status = EXIT_FAILURE;
         fprintf(stderr, "%s: %s: %s\n",
            exec::execname.c_str(), command.c_str(), strerror(errno));
      }
      else {
         cpplines(pipe, fullFileName.c_str());
         int pclose_rc = pclose(pipe);
         eprint_status(command.c_str(), pclose_rc);
         if (pclose_rc != 0) exec::exit_status = EXIT_FAILURE;
      }

      string_set::dump(fileStr);
      fclose(fileStr);

   return exec::exit_status;
}