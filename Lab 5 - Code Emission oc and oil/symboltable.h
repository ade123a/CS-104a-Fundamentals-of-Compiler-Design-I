//Davie Truong
//dtruong8@ucsc.edu
//1524861

#ifndef __SYMBOLTABLE_H__
#define __SYMBOLTABLE_H__

#include <iostream>
#include <bitset>
#include <string>
#include <unordered_map>
#include <vector>

#include "auxlib.h"
#include "astree.h"

extern FILE* sym_file;

enum { ATTR_void, ATTR_bool, ATTR_char, ATTR_int, ATTR_null,
       ATTR_string, ATTR_struct, ATTR_array, ATTR_function,
       ATTR_variable, ATTR_field, ATTR_typeid, ATTR_param,
       ATTR_lval, ATTR_const, ATTR_vreg, ATTR_vaddr,
       ATTR_bitset_size,
};
using attr_bitset = bitset<ATTR_bitset_size>;

struct symbol;
using symbol_table = unordered_map<const string*, symbol*>;
using symbol_entry = pair<string*, symbol*>;

struct symbol{
   attr_bitset attributes;
   symbol_table* fields;
   string typeID;
   string fieldName;
   size_t filenr, linenr, offset, block_nr;
   vector <symbol*>* parameters;
};

struct astree;

const char* get_attribute(astree* node, symbol* sym);
void check_array(astree* node);
void set_attribute(astree* node);
void insertSymbol (symbol_table table, const string* key, symbol* sym, astree* node);
void make_struct_symbol(astree* root);
void make_block(astree* root);
void make_function_symbol(astree* root);
void make_call_symbol(astree* root);
void make_prototype_symbol(astree* root);
void make_vardecl(astree* root);
void initialize_data(astree* root);
void parseTree(astree* root);

#endif