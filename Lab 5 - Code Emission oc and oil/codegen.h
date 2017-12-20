//Davie Truong
//dtruong8@ucsc.edu
//1524861

#ifndef __CODEGEN_H__
#define __CODEGEN_H__

#include <bitset>
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>

#include "astree.h"
#include "auxlib.h"
#include "lyutils.h"

using namespace std;

extern FILE * oil_file;

const char * makeName(astree * node, string currentName);
const char * updateType(astree * node, const string * structName);
void printStruct(astree * child, int depth);
void printStringConstant(astree * node, int depth);
void printReturn(astree * node, int depth);
void printBinary(astree * node);
void printUnary(astree * node);
void printConditional(astree * node);
void printWhile(astree * node, int depth);
void printCall(astree * node, int depth);
void printExpression(astree * node, int depth);
string registerType(string type);
void printNewAllocation(astree * node, int depth);
void printEquation(astree * node, int depth);
void tokParsing(astree * node, int depth, astree* nextNode);
void printFunction(astree * node, int depth);
void printOilData(astree * root, int depth);

#endif