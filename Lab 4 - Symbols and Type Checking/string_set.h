//Davie Truong
//dtruong8@ucsc.edu
//1524861

#ifndef __STRING_SET__
#define __STRING_SET__

#include <string>
#include <unordered_set>
using namespace std;

#include <stdio.h>

// Creating the public object "string_set"
struct string_set {
	string_set();									// defining the object
	static unordered_set<string> set;				// create the "set" object to hold unordered strings 
	
	static const string* intern(const char*);		// inserts a new string into the set
													//	returns a pointer to the string just inserted
													//	or if it is already there nothing is inserted, 
													//	the previously-inserted string is returned
	
	static void dump(FILE*);						// dumps out the string set in debug format
													//	print the header number followed by spaces, then the
													//	hash number and then the address of the string followed
													//	by the string itself
};

#endif
