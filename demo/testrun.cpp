/*
Copyright (C) 2012 Matevz Kovacic 

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <cstdio>

#include "Autocomplete.h"

void test(TAutocomplete &ac, const string &query)
{
	fprintf(stdout, "%s\n========\n", query.c_str());

	vector<string> results;
	ac.autocomplete(query, results);   

	for (vector<string>::const_iterator i(results.begin()); i != results.end(); ++i)
		fprintf(stdout, "%s\n", (*i).c_str());

	fprintf(stdout, "========\n\n");
}


int main()
{
	TAutocomplete ac;
//	ac.load("autocomplete.txt");
    ac.load("cities.txt");       

	test(ac, "nw yr");            //  1.102 expanded nodes 
	test(ac, "Lis Agnel    ");    //  2.412 expanded nodes 
	test(ac, "   hust");          //     66 expanded nodes 
	test(ac, "slvenj g");         //  2.545 expanded nodes 
	test(ac, "cpenh");            //    579 expanded nodes 
	test(ac, "smarje");           //     77 expanded nodes 
	test(ac, "fucking");          //    185 expanded nodes 
	test(ac, "frugle");           //    342 expanded nodes 
	
	return 0;
}

