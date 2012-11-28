#include <cstdio>
#include <cstring>

#include "Autocomplete.h"

TAutocomplete ac;
bool loaded = false;
string result;

void load()
{
	ac.load("cities.txt");
    loaded = true;
}

extern "C" const char* complete(const char* s)
{
    if(!loaded) { load(); }

    string query(s);
	vector<string> results;
	ac.autocomplete(query, results, 5);

    result = "[";
    bool first = true;
    vector<string>::const_iterator i;
	for(i = results.begin(); i != results.end(); i++)
    {
        if(first) { first = false; }
        else { result.append(",", 1); }

        result.append("\"", 1);
        result.append((*i).c_str());
        result.append("\"", 1);
    }
    result.append("]", 1);

    return result.c_str();
}
