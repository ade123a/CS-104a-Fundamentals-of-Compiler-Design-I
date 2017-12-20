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
#include <iostream>

#include "astree.h"
#include "auxlib.h"
#include "lyutils.h"
#include "string_set.h"
#include "symboltable.h"
#include "codegen.h"

const string CPP = "/usr/bin/cpp -nostdinc";
constexpr size_t LINESIZE = 1024;
string cpp_command;

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
   }
}

void cpp_pclose() {
   int pclose_rc = pclose (yyin);
   eprint_status (cpp_command.c_str(), pclose_rc);
   if (pclose_rc != 0) exec::exit_status = EXIT_FAILURE;
}


void scan_opts (int argc, char** argv) {
   int option; 
   yy_flex_debug = false;
   yydebug = false;

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
                  yy_flex_debug = true;  
                  break;
            case 'y':
                  yydebug = true;    
                  break;
            case 'D':
                  cpp_command = CPP + " -D" + optarg + " " +fullFileName;
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

   // Gets the file name and creates the .str, .tok, .ast filenames
   string fullFileName = basename(argv[optind]);
   string filename = fullFileName.substr(0, fullFileName.size() - 3);
   string filenameWithStr = filename + ".str";
   string filenameWithTok = filename + ".tok";
   string filenameWithAst = filename + ".ast";
   string filenameWithSym = filename + ".sym";
   string filenameWithOil = filename + ".oil";

   // Creates the .str, .tok, .ast file
   FILE* str_file = fopen(filenameWithStr.c_str(), "w");
   tok_file = fopen(filenameWithTok.c_str(), "w");
   ast_file = fopen(filenameWithAst.c_str(), "w");
   sym_file = fopen(filenameWithSym.c_str(), "w");
   oil_file = fopen(filenameWithOil.c_str(), "w");

   scan_opts(argc, argv);
   yyparse();                // Adds data to the tok_file and does the parsing to create the ast          
   cpp_pclose();

   // Dumps the string set data into the .str file
   string_set::dump(str_file);
   astree::print(ast_file, parser::root);       // Prints the data tree into the ast_file

   initialize_data(parser::root);
   parseTree(parser::root);

   printOilData(parser::root, 0);

   fclose(str_file);
   fclose(tok_file);
   fclose(ast_file);
   fclose(sym_file);
   fclose(oil_file);

   return exec::exit_status;
}