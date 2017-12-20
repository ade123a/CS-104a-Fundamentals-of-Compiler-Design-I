//Davie Truong
//dtruong8@ucsc.edu
//1524861

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

#include "astree.h"
#include "auxlib.h"
#include "lyutils.h"
#include "string_set.h"

const string CPP = "/usr/bin/cpp -nostdinc";
constexpr size_t LINESIZE = 1024;
string cpp_command;

FILE * tok_file;

// Scans the filename and creates .tok file using the same name
void makeTok (string name){
      // Create the file name
      string filename = name.substr(0, name.size() - 3);
      string filenameWithTokSuffix = filename + ".tok";
      // Creates the .tok file
      tok_file = fopen(filenameWithTokSuffix.c_str(), "w");

      // Check to see if the file openned correctly
      if (tok_file == NULL){
         fprintf (stderr, "oc: fatal error: unable to open '.tok' file");
      }
      else{
         for(;;){ // Infinite loop
               int tok = yylex();
               if (tok == YYEOF){   // Only exit when we hit the end of the file 
                     break;
               }
               else{    // Insert the data into the .tok file with the approproate format
                  fprintf(tok_file, "%3lu %lu.%04lu %4d %-10.10s (%s)\n",
                          lexer::lloc.filenr, lexer::lloc.linenr, lexer::lloc.offset, tok, parser::get_tname(tok), yytext);
                  string_set::intern(yytext);               // puts the contents of the file into a string set so it can later be dumpped into .str
                  DEBUGF('m', "token=%d", yytext);
               }
         }
      }
      fclose(tok_file);
}

// Open a pipe from the C preprocessor.
// Exit failure if can't.
// Assigns opened pipe to FILE* yyin.
void cpp_popen (const char* filename) {
   cpp_command = CPP + " " + filename;
   yyin = popen (cpp_command.c_str(), "r");
   if (yyin == NULL) {
      syserrprintf (cpp_command.c_str());
      exit (exec::exit_status);
   }else {
      if (yy_flex_debug) {
         fprintf (stderr, "-- popen (%s), fileno(yyin) = %d\n",
                  cpp_command.c_str(), fileno (yyin));
      }
      lexer::newfilename (cpp_command);
      makeTok(filename);
   }
}

void cpp_pclose() {
   int pclose_rc = pclose (yyin);
   eprint_status (cpp_command.c_str(), pclose_rc);
   if (pclose_rc != 0) exec::exit_status = EXIT_FAILURE;
}


void scan_opts (int argc, char** argv) {
   int option; 
   yy_flex_debug = 0;
   yydebug = 0;

   // Gets the full file name with extension
   string fullFileName = basename(argv[optind]);
   
   // Checking for correct file input
   if (fullFileName.size() <= 3 || fullFileName.substr(fullFileName.size() - 3, 3) != ".oc")
   {
      fprintf(stderr, "oc: fatal error: invalid filename\n File must be of type '.oc' ");
   }
   else{
      while ((option = getopt(argc, argv, "lyD:@:")) != -1) {
            switch (option) {
            case 'l':
                  yy_flex_debug = 1;  
                  break;
            case 'y':
                  yydebug = 1;    
                  break;
            case 'D':
                  cpp_command = CPP + " -D" + string(optarg) + fullFileName;
                   break;
            case '@':
                  set_debugflags(optarg);
                  break;
            default:
                  errprintf("%: Invalid option used \n", optopt);
                  break;
            }
      }
   }
   cpp_popen (fullFileName.c_str());
}


int main(int argc, char** argv) {

   exec::execname = basename (argv[0]);
   exec::exit_status = EXIT_SUCCESS;

   // Does the operations to make the .tok file
   scan_opts(argc, argv);
   cpp_pclose();

   // Gets the file name and attaches the .str to it
   string fullFileName = basename(argv[optind]);
   string filename = fullFileName.substr(0, fullFileName.size() - 3);
   string filenameWithStrSuffix = filename + ".str";

   // Creates the .str file
   FILE* tok_file = fopen(filenameWithStrSuffix.c_str(), "w");

   // Dumps the string set data of yytext into the .str file
   string_set::dump(tok_file);
   fclose(tok_file);

   return exec::exit_status;
}