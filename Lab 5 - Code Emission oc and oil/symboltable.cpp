//Davie Truong
//dtruong8@ucsc.edu
//1524861

#include <iostream>
#include <bitset>
#include <string>
#include <unordered_map>
#include <vector>

#include "symboltable.h"
#include "astree.h"
#include "lyutils.h"
#include "string_set.h"

vector <symbol_table*> symbolStack;
vector <int> blockCounter;
int nextBlock = 1;
FILE* sym_file;

symbol* newSymbol(astree* node){
   if(symbolStack.empty())
   {
      symbolStack.push_back(new symbol_table);
      symbolStack.push_back(nullptr);
   }
   if(blockCounter.empty())
   {
      blockCounter.push_back(0);
   }
   symbol* temp = new symbol();
   temp->attributes = node->attributes;
   temp->fields =  nullptr;
   temp->typeID = "";
   temp->fieldName = "";
   temp->filenr = node->lloc.filenr;
   temp->linenr = node->lloc.linenr;
   temp->offset = node->lloc.offset;
   temp->block_nr = blockCounter.back();
   temp->parameters = nullptr;
   return temp;
}

const char* get_attribute(astree* node, symbol* sym){
   string output = "";
   attr_bitset currentAttribute = node->attributes;
   if(currentAttribute[ATTR_void] == 1)      {  output += "void ";      }
   if(currentAttribute[ATTR_bool] == 1)      {  output += "bool ";      }
   if(currentAttribute[ATTR_char] == 1)      {  output += "char ";      }
   if(currentAttribute[ATTR_int] == 1)       {  output += "int ";       }
   if(currentAttribute[ATTR_null] == 1)      {  output += "null ";      }
   if(currentAttribute[ATTR_string] == 1)    {  output += "string ";    }
   if(currentAttribute[ATTR_struct] == 1)    {  output += "struct ";    }
   if(currentAttribute[ATTR_array] == 1)     {  output += "array ";     }
   if(currentAttribute[ATTR_function] == 1)  {  output += "function ";  }
   if(currentAttribute[ATTR_variable] == 1)  {  output += "variable ";  }
   if(currentAttribute[ATTR_field])          {  output += "field {" + sym->fieldName + "} "; }
   if(currentAttribute[ATTR_typeid] == 1)    {  output += "\"" + sym->typeID + "\" ";        }
   if(currentAttribute[ATTR_param] == 1)     {  output += "param ";     }
   if(currentAttribute[ATTR_lval] == 1)      {  output += "lval ";      }
   if(currentAttribute[ATTR_const] == 1)     {  output += "const ";     }
   if(currentAttribute[ATTR_vreg] == 1)      {  output += "vreg ";      }
   if(currentAttribute[ATTR_vaddr] == 1)     {  output += "vaddr ";     }

   return output.c_str();
}

void check_array(astree* node){
   switch(node->children[0]->symbol){

      case TOK_CHAR:    node->attributes[ATTR_char] = 1;
                        node->attributes[ATTR_lval] = 1;
                        break;

      case TOK_INT:     node->attributes[ATTR_int] = 1;
                        node->children[0]->attributes[ATTR_variable] = 1;
                        node->children[0]->attributes[ATTR_int] = 1;
                        node->attributes[ATTR_lval] = 1;
                        break;

      case TOK_BOOL:    node->attributes[ATTR_bool] = 1;
                        node->attributes[ATTR_const] = 1;
                        node->attributes[ATTR_lval] = 1;
                        break;
   }
}

void set_attribute(astree* node){
   switch(node->symbol){

      case TOK_VOID:    node->attributes[ATTR_void] = 1;
                        if(node->children.size() > 0){
                           node->children[0]->attributes[ATTR_void] = 1;
                        }
                        break;

      case TOK_BOOL:    node->attributes[ATTR_bool] = 1;
                        node->attributes[ATTR_const] = 1;
                        node->attributes[ATTR_lval] = 1;
                        if(node->children.size() > 0){
                           node->children[0]->attributes[ATTR_bool] = 1;
                        }
                        break;

      case TOK_CHAR:    node->attributes[ATTR_char] = 1;
                        node->attributes[ATTR_lval] = 1;
                        if(node->children.size() > 0){
                           node->children[0]->attributes[ATTR_char] = 1;
                        }
                        break;
      
      case TOK_INT:     node->attributes[ATTR_int] = 1;
                        node->attributes[ATTR_lval] = 1;
                        if(node->children.size() > 0){
                           node->children[0]->attributes[ATTR_int] = 1;
                        }
                        break;

      case TOK_NULL:    node->attributes[ATTR_null] = 1;
                        node->attributes[ATTR_const] = 1;
                        break;

      case TOK_STRING:  node->attributes[ATTR_string] = 1;
                        node->attributes[ATTR_lval] = 1;
                        if(node->children.size() > 0){
                           node->children[0]->attributes[ATTR_string] = 1;
                        }
                        break;     

      case TOK_STRUCT:  node->attributes[ATTR_struct] = 1;
                        node->attributes[ATTR_typeid] = 1;         
                        node->children[0]->attributes[ATTR_struct] = 1; 
                        node->attributes[ATTR_variable] = 0;
                        node->attributes[ATTR_lval] = 0;         
                        node->children[0]->attributes[ATTR_variable] = 0;
                        node->children[0]->attributes[ATTR_lval] = 0; 
                        break;    

      case TOK_ARRAY:   node->attributes[ATTR_array] = 1;
                        break;

      case TOK_FUNCTION:   node->attributes[ATTR_function] = 1;         
                           node->children[0]->children[0]->attributes[ATTR_function] = 1;
                           node->attributes[ATTR_variable] = 0;
                           node->attributes[ATTR_lval] = 0;         
                           node->children[0]->children[0]->attributes[ATTR_variable] = 0; 
                           node->children[0]->children[0]->attributes[ATTR_lval] = 0; 
                           break;

      case TOK_PROTOTYPE:  node->children[0]->children[0]->attributes[ATTR_variable] = 0;
                           node->children[0]->children[0]->attributes[ATTR_lval] = 0;
                           break;

      case TOK_PARAMLIST:  for(size_t index = 0; index < node->children.size(); ++index){
                              node->children[index]->children[0]->attributes[ATTR_param] = 1;
                           }
                           break;

      case TOK_STRINGCON:  node->attributes[ATTR_string] = 1;
                           node->attributes[ATTR_const] = 1;
                           node->attributes[ATTR_lval] = 1;
                           break;
      
      case TOK_CHARCON:    node->attributes[ATTR_char] = 1;
                           node->attributes[ATTR_const] = 1;
                           node->attributes[ATTR_lval] = 1;
                           break;
      
      case TOK_INTCON:     node->attributes[ATTR_int] = 1;
                           node->attributes[ATTR_const] = 1;
                           node->attributes[ATTR_lval] = 1;
                           break;

      case TOK_FIELD:      node->attributes[ATTR_field] = 1;
                           if("TOK_TYPEID" == string(parser::get_tname(node->symbol))){
                              node->attributes[ATTR_struct] = 1;
                           }     
                           break;

      case TOK_TYPEID:     node->attributes[ATTR_typeid] = 1;
                           for(size_t index = 0; index < node->children.size(); ++index){
                              node->children[index]->attributes[ATTR_typeid] = 1;
                              node->children[index]->attributes[ATTR_struct] = 1;
                              break;
                           }

      case TOK_IDENT:      node->attributes[ATTR_lval] = 1;
                           node->attributes[ATTR_variable] = 1;
                           break;

      case TOK_VARDECL:    node->attributes[ATTR_variable] = 1;
                           break;

      case TOK_DECLID:     node->attributes[ATTR_lval] = 1;
                           node->attributes[ATTR_variable] = 1;
                           break;

      default:             break;
   }
}

void insertSymbol (symbol_table table, const string* key, symbol* sym, astree* node){
   table[key] = sym;
   for(size_t size = 1; size < blockCounter.size(); ++size){
      fprintf(sym_file, "   ");
   }
   fprintf(sym_file, "%s (%zu.%zu.%zu) {%zu} %s\n", 
         key->c_str(), sym->filenr, sym->linenr, sym->offset, sym->block_nr, get_attribute(node, sym));
}

void make_struct_symbol(astree* root){
   symbol_table fields;
   const string* key;
   key = root->children[0]->lexinfo;
   symbol* sym = newSymbol(root->children[0]);
   sym->typeID = root->children[0]->lexinfo->c_str();
   sym->fields = &fields;
   insertSymbol(*symbolStack[0], key, sym, root->children[0]);

   if(root->children.size() > 1){
      astree* fieldList = root->children[1];
      for(size_t index = 0; index < fieldList->children.size(); ++index){
         astree* currentField = fieldList->children[index]->children[0];
         sym = newSymbol(currentField);
         key = currentField->lexinfo;
         sym->fieldName = root->children[0]->lexinfo->c_str();
         if("TOK_TYPEID" == string(parser::get_tname(fieldList->children[index]->symbol))){
            sym->typeID = fieldList->children[index]->lexinfo->c_str();
         }  
         fprintf(sym_file, "   ");
         insertSymbol(fields, key, sym, currentField);
      }
   }
}

void make_block(astree* root){
   blockCounter.push_back(nextBlock);
   nextBlock++;
   symbolStack[blockCounter.back()] = new symbol_table;
   symbolStack.push_back(nullptr);
   parseTree(root);
   blockCounter.pop_back();
}

void make_function_symbol(astree* root){
   astree* function = root->children[0]->children[0];
   vector<symbol*> params;
   symbol* sym = newSymbol(function);
   sym->typeID = root->children[0]->lexinfo->c_str();
   sym->parameters = &params; 
   const string* key;
   key = function ->lexinfo;
   insertSymbol(*symbolStack[0], key, sym, function);

   astree* paramList = root->children[1];
   for(size_t index = 0; index < paramList->children.size(); ++index){
      astree* currentParam = paramList->children[index]->children[0];
      sym = newSymbol(currentParam);
      key = currentParam->lexinfo;
      if("TOK_TYPEID" == string(parser::get_tname(paramList->children[index]->symbol))){
         sym->typeID = paramList->children[index]->lexinfo->c_str();
      }
      ++sym->block_nr;
      params.push_back(sym);
      fprintf(sym_file, "   ");
      insertSymbol(*symbolStack[0], key, sym, currentParam);  
   }
   fprintf(sym_file, "\n");
   make_block(root->children[2]);
}

void make_call_symbol(astree* root){
   vector<symbol*> params;
   astree* call = root->children[0];
   symbol* sym = newSymbol(call);
   sym->typeID = root->children[0]->lexinfo->c_str();
   sym->parameters = &params;
   const string* key;
   key = call->lexinfo;
   insertSymbol(*symbolStack[0], key, sym, call);

   for(size_t index = 1; index < root->children.size(); ++index){
      astree* call = root->children[index];
      sym = newSymbol(call);
      key = call->lexinfo;
      ++sym->block_nr;
      params.push_back(sym);
      fprintf(sym_file, "   ");
      insertSymbol(*symbolStack[0], key, sym, call);  
   }
   fprintf(sym_file, "\n");
}

void make_prototype_symbol(astree* root){
   astree* prototype;
   if("TOK_ARRAY" == string(parser::get_tname(root->children[0]->symbol))){
      prototype = root->children[0]->children[1];
      prototype->attributes = root->children[0]->children[1]->attributes;
      prototype->attributes[ATTR_array] = 1;                           
   }else{
      prototype = root->children[0]->children[0];
   }
   symbol* sym = newSymbol(prototype);
   vector<symbol*> params;
   sym->parameters = &params;
   const string* key;
   key = prototype->lexinfo;
   insertSymbol(*symbolStack[0], key, sym, prototype);

   astree* paramList = root->children[1];
   for(size_t index = 0; index < paramList->children.size(); ++index){
      astree* prototype = paramList->children[index]->children[0];
      sym = newSymbol(prototype);
      key = prototype->lexinfo;
      if("TOK_TYPEID" == string(parser::get_tname(paramList->children[index]->symbol))){
         sym->typeID = paramList->children[index]->lexinfo->c_str();
      }  
      ++sym->block_nr;
      params.push_back(sym);
      fprintf(sym_file, "   ");
      insertSymbol(*symbolStack[0], key, sym, prototype);  
   }
   fprintf(sym_file, "\n");
}

void make_vardecl(astree* root){
   astree* vardecl = root->children[0]->children[0];
   symbol* sym = newSymbol(vardecl);
   const string* key;
   key = vardecl->lexinfo;
   if("TOK_TYPEID" == string(parser::get_tname(root->children[0]->symbol))){
         sym->typeID = root->children[0]->lexinfo->c_str();
      }  
   insertSymbol(*symbolStack[blockCounter.back()], key, sym,vardecl);
}

void initialize_data(astree* root){
   for(size_t index = 0; index < root->children.size(); ++index){
      initialize_data(root->children[index]);
   }
   set_attribute(root);
}

void parseTree(astree* root){
   for(size_t index = 0; index < root->children.size(); ++index){

      int currentNodeSymbol = root->children[index]->symbol;
      switch(currentNodeSymbol){

         case TOK_STRUCT:     make_struct_symbol(root->children[index]);
                              fprintf(sym_file, "\n");
                              break;

         case TOK_FUNCTION:   make_function_symbol(root->children[index]);
                              fprintf(sym_file, "\n");
                              break;

         case TOK_PROTOTYPE:  make_prototype_symbol(root->children[index]);
                              fprintf(sym_file, "\n");
                              break;                

         case TOK_CALL:       make_call_symbol(root->children[index]);
                              fprintf(sym_file, "\n");
                              break;     

         case TOK_VARDECL:    make_vardecl(root->children[index]);
                              break;

         case TOK_IF:         make_block(root->children[index]->children[1]);
                              break;

         case TOK_IFELSE:     make_block(root->children[index]->children[1]);
                              make_block(root->children[index]->children[2]);
                              break;

         default:             break;
      }
   }
}