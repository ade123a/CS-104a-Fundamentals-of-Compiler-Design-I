//Davie Truong
//dtruong8@ucsc.edu
//1524861

// $Id: astree.cpp,v 1.9 2017-10-04 15:59:50-07 - - $

#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

#include "astree.h"
#include "string_set.h"
#include "lyutils.h"       // contains set of declarations to link program to flex and bison
#include "symboltable.h"

FILE * ast_file;

astree::astree (int symbol_, const location& lloc_, const char* info) {    
   symbol = symbol_;
   lloc = lloc_;
   lexinfo = string_set::intern (info);           // Get rid of any refernce to the intern 
                                                  // function in the exising code and store in lex info
   attributes = 0;
   blockNumber = 0;
   // vector defaults to empty -- no children
}

astree::~astree() {                             // Destructors, post order traversal
   while (not children.empty()) {               
      astree* child = children.back();          // Find the last entry in the vecotor
      children.pop_back();                      // and pop it off, remove the last entry in the vector and destroys it 
      delete child;
   }
   if (yydebug) {                               // Runs if -y is enabled
      fprintf (stderr, "Deleting astree (");    // Print a message indicating what we are doing
      astree::dump (stderr, this);              // Dump the astree
      fprintf (stderr, ")\n");
   }
}

astree* astree::adopt (astree* child1, astree* child2) {    // Allows an exising tree to add in a new child
   if (child1 != nullptr) children.push_back (child1);      
   if (child2 != nullptr) children.push_back (child2);
   return this;                                             // Returns a pointer that points to the current node
}

astree* astree::adopt_sym (astree* child, int symbol_) {    // Used by the parser, to change the operator symbol
   symbol = symbol_;
   return adopt (child);
}


void astree::dump_node (FILE* outfile) {                    // Debug function 
   fprintf (outfile, "%p->{%s %zd.%zd.%zd \"%s\":",
            this, parser::get_tname (symbol),               
            lloc.filenr, lloc.linenr, lloc.offset,
            lexinfo->c_str());
   for (size_t child = 0; child < children.size(); ++child) {
      fprintf (outfile, " %p", children.at(child));
   }
}

void astree::dump_tree (FILE* outfile, int depth) {         // Debug function
   fprintf (outfile, "%*s", depth * 3, "");
   dump_node (outfile);
   fprintf (outfile, "\n");
   for (astree* child: children) child->dump_tree (outfile, depth + 1);
   fflush (NULL);
}

void astree::dump (FILE* outfile, astree* tree) {
   if (tree == nullptr) fprintf (outfile, "nullptr");
                   else tree->dump_node (outfile);
}

void astree::print (FILE* outfile, astree* tree, int depth) {              // Print information about the current node in three and then recursively call the child

   for (int i = 0; i < depth; i++){
      fprintf(outfile, "|  ");
   }
   const char *tname = parser::get_tname(tree->symbol);
   if (strstr (tname, "TOK_") == tname) tname += 4;

   fprintf (outfile, "%s \"%s\" (%zd.%zd.%zd)\n",
            tname, tree->lexinfo->c_str(),
            tree->lloc.filenr, tree->lloc.linenr, tree->lloc.offset);
   for (astree* child: tree->children) {
      astree::print (outfile, child, depth + 1);
   }
}

void destroy (astree* tree1, astree* tree2) {               // Deletes 1 or 2 of the trees
   if (tree1 != nullptr) delete tree1;
   if (tree2 != nullptr) delete tree2;
}

void errllocprintf (const location& lloc, const char* format,
                    const char* arg) {
   static char buffer[0x1000];
   assert (sizeof buffer > strlen (format) + strlen (arg));
   snprintf (buffer, sizeof buffer, format, arg);
   errprintf ("%s:%zd.%zd: %s", 
              lexer::filename (lloc.filenr), lloc.linenr, lloc.offset,
              buffer);
}

astree* astree::swap_sym (astree* root, int symbol){
   root->symbol = symbol;
   return root;
}

void astree::debug_print (astree* tree, int depth){
   printf ("%s \"%s\" (%zd.%zd.%zd)\n",
            parser::get_tname (tree->symbol), tree->lexinfo->c_str(),
            tree->lloc.filenr, tree->lloc.linenr, tree->lloc.offset);
   for (astree* child: tree->children) {
      astree::debug_print (child, depth + 1);
   }
}