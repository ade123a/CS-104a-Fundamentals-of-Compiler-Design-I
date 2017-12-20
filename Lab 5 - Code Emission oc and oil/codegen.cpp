//Davie Truong
//dtruong8@ucsc.edu
//1524861

#include <iostream>
#include <string>
#include <vector>

#include "codegen.h"
#include "symboltable.h"

FILE * oil_file;
size_t counter = 0;

const char * makeName(astree * node, string currentName){
   string newTokenName;
   int symbol = node->symbol;
   if(node->blockNumber != 0){
      switch (symbol){
         case TOK_WHILE:
         case TOK_IF:
         case TOK_IFELSE:     newTokenName = currentName + "_" + to_string(node->lloc.filenr) + "_"
                                                               + to_string(node->lloc.linenr) + "_"
                                                               + to_string(node->lloc.offset) + ":;";
                              break;
         case NUMBER:         newTokenName = currentName;
                              break;
         case TOK_FUNCTION:   newTokenName = "__" + currentName;
                              break;
         default:             newTokenName = "_" + to_string(node->blockNumber) + "_" + *node->lexinfo;
      }
   }
   else{
      switch (symbol){
         case TOK_STRUCT:        newTokenName = "s_" + currentName;
                                 break;
         case TOK_TYPEID:        newTokenName = "f_" + *node->lexinfo + "_" + currentName;
                                 break;
         case TOK_WHILE:
         case TOK_IF:
         case TOK_IFELSE:        newTokenName = currentName + "_" + to_string(node->lloc.filenr) + "_"
                                                                  + to_string(node->lloc.linenr) + "_"
                                                                  + to_string(node->lloc.offset) + ":;";
                                 break;
         case TOK_DECLID:        newTokenName = "_" + to_string(node->blockNumber) + "_" + *node->lexinfo;    
                                 break;        
         default:                newTokenName = "__" + currentName;       
                                 break;           
      }
   }
   return newTokenName.c_str();
}

const char * updateType(astree * node, const string * structName){
   string newType;
   string currentType = *node->lexinfo;
   if(currentType == "int"){
      newType = "int";
   }else if(currentType == "char" || currentType == "bool"){
      newType = "char";
   }else if(currentType == "string"){
      newType = "char*";
   }
   else{
      if(structName == nullptr){
         currentType = "ERROR" + currentType;
         return currentType.c_str();
      }
      astree * tempStruct = node;
      tempStruct->symbol = TOK_STRUCT;
      tempStruct->lexinfo = structName;
      newType = "struct " + string(makeName(tempStruct, currentType)) + "*";
   }
   return newType.c_str();
}

void printStruct(astree * child, int depth){
   astree * structName = child->children[0];
   string tempDepth = string(depth * 3, ' ');
   fprintf(oil_file, "struct %s{\n", makeName(child, *structName->lexinfo));
   depth++;
   for(size_t i = 1; i < child->children.size(); ++i){
      astree * type = child->children[i];
      astree * field = type->children[0];
      string currentName = *field->lexinfo;
      field->lexinfo = structName->lexinfo;
      fprintf(oil_file, "%s%s %s;\n", tempDepth.c_str(), updateType(type, structName->lexinfo), 
                                       makeName(child, *structName->lexinfo));
   }
   fprintf(oil_file, "};\n");
}

void printStringConstant(astree * node, int depth){
   if(node->symbol == '='){
      if(node->children[1]->symbol == TOK_STRINGCON){
         fprintf(oil_file, "char* %s = %s\n", makeName(node, *node->children[0]->children[0]->lexinfo),
                                                (*node->children[1]->lexinfo).c_str());
      }
   }
   else{
      for(astree * child: node->children){
         printStringConstant(child, depth);
      }
   }
}

void printReturn(astree * node, int depth){
   astree* returnValue = node->children[0];
   string tempDepth = string(depth * 3, ' ');
   fprintf(oil_file, "%s", tempDepth.c_str());
   if(returnValue == nullptr){
      fprintf(oil_file, "return;\n");
   }
   else{
      if(returnValue->symbol == TOK_IDENT){
         fprintf(oil_file, "return %s;\n", makeName(returnValue, *returnValue->lexinfo));
      }
      else{
         fprintf(oil_file, "return %s;\n", (*returnValue->lexinfo).c_str());
      }
   }
}

void printBinary(astree * node){
   astree * left = node->children[0];
   astree * right = node->children[1];
   fprintf(oil_file, "char b%zd = %s %s %s;\n", counter++, makeName(left, *left->lexinfo), (*node->lexinfo).c_str(),
                                                makeName(right, *right->lexinfo));
}

void printUnary(astree * node){
   fprintf(oil_file, "char b%zd = %s%s;\n", counter++, (*node->lexinfo).c_str(), makeName(node, *node->lexinfo));
}

void printConditional(astree * node){
   size_t size = node->children.size();
   if(size == 1){
      printUnary(node);
   }
   else if(size ==2){
      printBinary(node);
   }
   else{
      fprintf(oil_file, "char b%zd = %s;\n", counter++, makeName(node, *node->lexinfo));
   }
}

void printWhile(astree * node, int depth){
   fprintf(oil_file, "%s\n", makeName(node, *node->lexinfo));
   printConditional(node->children[0]);
   fprintf(oil_file, "if(!b%zd) goto break_%zd_%zd_%zd;\n", counter-1, node->lloc.filenr, node->lloc.linenr, node->lloc.offset);
   tokParsing(node->children[1], depth, node);
   fprintf(oil_file, "goto %s\n", makeName(node, *node->lexinfo));
   fprintf(oil_file, "break_%zd_%zd_%zd:\n", node->lloc.filenr, node->lloc.linenr, node->lloc.offset);
}

void printCall(astree * node, int depth){
   node->children[0]->symbol = TOK_FUNCTION;
   string tempDepth = string(depth * 3, ' ');
   fprintf(oil_file, "%s%s (", tempDepth.c_str(), makeName(node->children[0], *node->children[0]->lexinfo));
   for(size_t i = 1; i < node->children.size(); i++){
      astree * argument = node->children[i];
      fprintf(oil_file, "%s", makeName(argument, *argument->lexinfo));
      if(i+1 != node->children.size()){
         fprintf(oil_file, ", ");
      }
   }
   fprintf(oil_file, ")");
}

void printExpression(astree * node, int depth){
   astree * left = node->children[0];
   switch(left->symbol){
      case TOK_IDENT:      fprintf(oil_file, "%s %s ", makeName(left, *left->lexinfo), (*node->lexinfo).c_str());
                           break;
      case '+':            
      case '-':            if(left->children.size() == 1){
                              fprintf(oil_file, "%s%s", (*left->lexinfo).c_str(), (*left->children[0]->lexinfo).c_str());
                              break;
                           }
      case '*':
      case '/':            printExpression(left, depth);
                           fprintf(oil_file, "%s ", (*left->lexinfo).c_str());
                           break;
      case TOK_STRINGCON:
      case TOK_CHARCON:
      case NUMBER:         fprintf(oil_file, "%s %s ", (*left->lexinfo).c_str(), (*node->lexinfo).c_str());
                           break;
      case TOK_CALL:       printCall(left, depth);
                           break;
   }
   astree * right = node->children[1];
   switch(right->symbol){
      case TOK_IDENT:      fprintf(oil_file, "%s", makeName(right, *right->lexinfo));
                           break;
      case TOK_STRINGCON:
      case TOK_CHARCON:
      case NUMBER:
      case TOK_CALL:       printCall(right, depth);
                           fprintf(oil_file, " ");
                           break;
   }
}

string registerType(string type){
   if(type == "char"){
      return "c";
   }
   else if(type == "string"){
      return "s";
   }
   else{
      return "i";
   }
}

void printNewAllocation(astree * node, int depth){
   astree * newAllocation = node->children[0];
   if(newAllocation->symbol == TOK_TYPEID){
      fprintf(oil_file, "struct %s* p%zd = xcalloc (1, sizeof (struct %s));\n", 
                           (*newAllocation->lexinfo).c_str(), counter++, makeName(newAllocation, *newAllocation->lexinfo));
   }
   else if(node->symbol == TOK_NEWARRAY){
      fprintf(oil_file, "%s* p%zd = xcalloc (%s, sizeof (%s));\n", 
                           updateType(node->children[0], node->children[0]->lexinfo), counter++, (*node->children[1]->lexinfo).c_str(),
                           makeName(node->children[0], *node->children[0]->lexinfo));
   }
   else if(node->symbol == TOK_NEWSTRING){
      fprintf(oil_file, "char* p%zd = xcalloc (opnd s, sizeof (char));\n", counter++);
   }
   else{
      fprintf(oil_file, "ERROR: %s;\n", (*node->lexinfo).c_str());
   }
   depth += 1; 
}

void printEquation(astree * node, int depth){
   astree * left = node->children[0];
   astree * right = node->children[1];
   string tempDepth = string(depth * 3, ' ');
   
   if(left->symbol != TOK_IDENT){
      switch(right->symbol){
         case TOK_IDENT:         fprintf(oil_file, "%s%s %s = %s;\n", tempDepth.c_str(), updateType(left, left->lexinfo),
                                                      makeName(left->children[0], *left->children[0]->lexinfo), 
                                                      makeName(right, *right->lexinfo));
                                 break;
         case NUMBER:        
         case TOK_CHARCON:
         case TOK_STRINGCON:     fprintf(oil_file, "%s%s %s = %s;\n", tempDepth.c_str(), updateType(left, left->lexinfo),
                                                      makeName(left->children[0], *left->children[0]->lexinfo),
                                                      (*right->lexinfo).c_str());
                                 break;         
         case TOK_CHAR:          fprintf(oil_file, "%s%s %s = %s (%s);\n", tempDepth.c_str(), updateType(left, left->lexinfo),
                                                      makeName(left->children[0], *left->children[0]->lexinfo),
                                                      makeName(right, *right->lexinfo), makeName(right->children[0],
                                                      *right->children[0]->lexinfo));
                                 break;
         case TOK_NEWARRAY:      printNewAllocation(right, depth);      
                                 fprintf(oil_file, "%s* %s = p%zd;\n", updateType(left->children[0], left->children[0]->lexinfo),
                                                      (*left->children[1]->lexinfo).c_str(), counter-1);
                                 break;
         case TOK_NEWSTRING:
         case TOK_NEW:           printNewAllocation(right, depth);
                                 fprintf(oil_file, "%s %s = p%zd;\n", updateType(left, left->lexinfo),
                                                      (*left->children[0]->lexinfo).c_str(), counter-1);
                                 break;
         case '!':               fprintf(oil_file, "%s%s %s = !%s;\n", tempDepth.c_str(), updateType(left, left->lexinfo),
                                                      makeName(left->children[0], *left->children[0]->lexinfo),
                                                      makeName(right->children[0], *right->children[0]->lexinfo));
                                 break;
         case '+':
         case '-':               if(right->children.size() == 1){
                                    fprintf(oil_file, "%s%s %s = %s%s;\n", tempDepth.c_str(), updateType(left, left->lexinfo), 
                                                      makeName(left, *left->lexinfo), (*right->lexinfo).c_str(),
                                                      makeName(right->children[0], *right->children[0]->lexinfo));
                                 break;
                                 }
         case '*':
         case '/':               fprintf(oil_file, "%s%s %s%zd = ", tempDepth.c_str(), updateType(left, left->lexinfo),
                                                      registerType(*left->lexinfo).c_str(), counter++);
                                 printExpression(right, depth);
                                 fprintf(oil_file, ";\n");
                                 fprintf(oil_file, "%s%s %s = %s%zd;\n", tempDepth.c_str(), updateType(left, left->lexinfo),
                                                      makeName(left->children[0], *left->children[0]->lexinfo),
                                                      registerType(*left->lexinfo).c_str(), counter-1);
                                 break;    
         default:                fprintf(oil_file, "%s%s %s%zd = ", tempDepth.c_str(), updateType(left, left->lexinfo),
                                                      registerType(*left->lexinfo).c_str(), counter++);
                                 printCall(right, depth);
                                 fprintf(oil_file, ";\n");
                                 fprintf(oil_file, "%s%s %s = %s%zd;\n", tempDepth.c_str(), updateType(left, left->lexinfo),
                                                      makeName(left->children[0], *left->children[0]->lexinfo),
                                                      registerType(*left->lexinfo).c_str(), counter-1);
                                 break;                               
      }
   }
   else{
      switch (right->symbol){
         case TOK_IDENT:         fprintf(oil_file, "%s%s = %s;\n", tempDepth.c_str(), 
                                                      makeName(left, *left->lexinfo), 
                                                      makeName(right, *right->lexinfo));
                                 break;
         case NUMBER:        
         case TOK_CHARCON:
         case TOK_STRINGCON:        
         case TOK_CHAR:          fprintf(oil_file, "%s%s = %s (%s);\n", tempDepth.c_str(), makeName(left, *left->lexinfo),
                                                      makeName(right, *right->lexinfo), makeName(right->children[0],
                                                      *right->children[0]->lexinfo));
                                 break;
         case TOK_NEWARRAY:      printNewAllocation(right, depth);      
                                 fprintf(oil_file, "%s = p%zd;\n", (*left->lexinfo).c_str(), counter-1);
                                 break;
         case TOK_NEWSTRING:
         case TOK_NEW:           printNewAllocation(right, depth);
                                 fprintf(oil_file, "%s = p%zd;\n", (*left->lexinfo).c_str(), counter-1);
                                 break;
         case '!':               fprintf(oil_file, "%s%s = %s%s;\n", tempDepth.c_str(), makeName(left, *left->lexinfo),
                                                      (*right->lexinfo).c_str(), makeName(right->children[0], 
                                                      *right->children[0]->lexinfo));
                                 break;
         case '+':
         case '-':               if(right->children.size() == 1){
                                    fprintf(oil_file, "%s%s = %s%s;\n", tempDepth.c_str(), makeName(left, *left->lexinfo), 
                                                      (*right->lexinfo).c_str(), makeName(right->children[0], 
                                                      *right->children[0]->lexinfo));
                                 break;
                                 }
         case '*':
         case '/':               fprintf(oil_file, "%sint %s%zd = ", tempDepth.c_str(), registerType(*left->lexinfo).c_str(), counter++);
                                 printExpression(right, depth);
                                 fprintf(oil_file, "%s%s = %s%zd;\n", tempDepth.c_str(), makeName(left, *left->lexinfo),
                                                      registerType(*left->lexinfo).c_str(), counter-1);
                                 break;    
         default:                fprintf(oil_file, "%s%s %s%zd;\n", tempDepth.c_str(), (*left->lexinfo).c_str(),
                                                      registerType(*left->lexinfo).c_str(), counter++);
                                 printCall(right, depth);
                                 fprintf(oil_file, ";\n");
                                 fprintf(oil_file, "%s%s = %s%zd;\n", tempDepth.c_str(), makeName(left, *left->lexinfo),
                                                      registerType(*left->lexinfo).c_str(), counter-1);
                                 break;    
      }
   }
}

void tokParsing(astree * node, int depth, astree* nextNode){
   string tempDepth = string(depth * 3, ' ');
   switch (node->symbol){
      case TOK_BLOCK:            for(astree* child: node->children){
                                    tokParsing(child, depth+1, nextNode);
                                 }
                                 break;
      case TOK_WHILE:            printWhile(node, 0);
                                 break;
      case TOK_IF:               printConditional(node->children[0]);
                                 fprintf(oil_file, "if(!b%zdgoto fi_%zd_%zd_%zd;\n", counter-1, node->lloc.filenr, node->lloc.linenr,
                                                      node->lloc.offset);
                                 tokParsing(node->children[1], depth, nextNode);
                                 fprintf(oil_file, "fi_%zd_%zd_%zd:;\n", node->lloc.filenr, node->lloc.linenr, node->lloc.offset);
                                 break;
      case '=':                  printEquation(node, depth);
                                 break;
      case TOK_RETURN:           printReturn(node, depth);
                                 break;
      case TOK_PROTOTYPE:        break;                           
      case TOK_CALL:             printCall(node, depth);
                                 fprintf(oil_file, ";\n");
                                 break;
      default:                   fprintf(oil_file, "%s%s\n", tempDepth.c_str(), parser::get_tname(node->symbol));
                                 break;                           
   }
}

void printFunction(astree * node, int depth){
   astree * returnType = node->children[0];
   astree * functionName = returnType->children[0];
   functionName->symbol = TOK_FUNCTION;
   fprintf(oil_file, "%s %s (\n", updateType(returnType, returnType->lexinfo), makeName(functionName, *functionName->lexinfo));
   depth++;
   int nextNode = 2;
   astree * parameterList = node->children[1];
   if(parameterList->symbol == TOK_PARAMLIST){
      for(size_t i = 0; i < parameterList->children.size(); i++){
         astree * parameterType = parameterList->children[i];
         astree * parameterName = parameterType->children[0];
         fprintf(oil_file, "%s %s (\n", updateType(parameterType, parameterType->lexinfo), makeName(parameterName, *parameterName->lexinfo));
         if(i+1 != parameterList->children.size()){
            fprintf(oil_file, ",\n");
         }
      }
   }
   else{
      nextNode = 1;
   }
   fprintf(oil_file, ")\n{\n");
   astree * block = node->children[nextNode];
   tokParsing(block, depth, nullptr);
   fprintf(oil_file, "}\n");
}

void printOilData(astree * root, int depth){
   for(astree * child: root->children){
      if(child->symbol == TOK_STRUCT){
         printStruct(child, depth);
      }
   }
   for(astree * child: root->children){
      printStringConstant(child, depth);
   }
   for(astree * child: root->children){
      if(child->symbol == '=' && child->children[0]->children.size() != 0){
         astree * type = child->children[0];
         astree * declid = type->children[0];
         if(declid->symbol == TOK_DECLID && type->symbol != TOK_STRING){
            fprintf(oil_file, "%s %s;\n", updateType(type, nullptr), makeName(type, *declid->lexinfo));
         }
      }
   }
   for(astree * child: root->children){
      if(child->symbol == TOK_FUNCTION){
         printFunction(child, depth);
      }
   }
   fprintf(oil_file, "void __ocmain (void)\n{\n");
   for(astree * child: root->children){
      if(child->symbol != TOK_FUNCTION && child->symbol != TOK_STRUCT){
         tokParsing(child, 1, nullptr);
      }
   }
   fprintf(oil_file, "}\nend\n");
}