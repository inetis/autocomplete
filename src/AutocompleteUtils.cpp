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

#include "AutocompleteUtils.h"

#include <stdexcept>
using std::runtime_error;

#include <algorithm>
using std::min;
using std::max;

#include <fstream>
using std::ifstream;

#include <sstream>
using std::stringstream;

/*******************
*   TTrie      * 
********************/
TTrie::TTrie()
	: sum_weight(.0)
{
	nodes.push_back(Node(' ', .0));
}

string error(const string &message, const size_t &line)
{
	stringstream s;
    s << message << " line " << line;
	return s.str();
}

void TTrie::load(const string file_name)
{
	// init Trie
	if (sum_weight > .0)
	{
		nodes.clear();
		nodes.push_back(Node(' ', .0));
		sum_weight = .0;
	}

	ifstream f(file_name.c_str());
	if (!f)
		throw runtime_error("TTrie::load - cannot open file " + file_name);

	float  freq;
	string word;
    size_t line(0);

	while (f >> freq)
	{
	    if (!(f.ignore() && getline(f, word, '\n')))
			throw runtime_error(error("TTrie::load cannot read word", line));

        if (word[word.size() - 1] == '\r')  // for unix
			word.resize(word.size() - 1);

		add(word, freq);	
		++line;
	}
	  
	if (!f.eof())
		throw runtime_error(error("TTrie::load cannot read weight", line));

	if (sum_weight == .0)
		throw runtime_error("TTrie::load " + file_name + " is empty");

	finalize(0);
}

void TTrie::add(const string &s, const float &weight)
{
	if (weight <= (float).0)
		throw runtime_error("TTrie:add error: weight must be positive number");

	sum_weight += weight;
	add(0, s.begin(), s.end(), weight);
}

void TTrie::add(const size_t                 node_id, 
	                  string::const_iterator begin, 
				const string::const_iterator end, 
				const float                  weight)
{
	if (begin == end)
	{
		// check if inserted string is already in trie by checking if current node contains (char)0 subtree
      	for (Node::TSubTreeIt i(nodes[node_id].sub_trees.begin()); i != nodes[node_id].sub_trees.end(); ++i)
		   if (nodes[*i].c == (char)0)
		   {
			   nodes[*i].prob += weight;
			   return;
		   }

		// add new (char)0 delimited node in trie - denoting the end of word
    	nodes[node_id].sub_trees.push_back(nodes.size());  // add new subtree in current node; note tha subtree is actually created at the next step
		nodes.push_back(Node((char)0, weight));
		return;
	}
 
	for (Node::TSubTreeIt i(nodes[node_id].sub_trees.begin()); i != nodes[node_id].sub_trees.end(); ++i)
		if (nodes[*i].c == *begin)
		{
			add(*i, ++begin, end, weight);
			return;
		}

	// insert new tree in subtree list
	nodes[node_id].sub_trees.push_back(nodes.size());  // add new subtree at current node; note that subtree is actually created at the next step
	nodes.push_back(Node(*begin, .0));  
   	add(nodes[node_id].sub_trees.back(), ++begin, end, weight);
}


void TTrie::finalize(const size_t node_id)
{
	if (nodes[node_id].sub_trees.empty())
	{
   	    nodes[node_id].prob /= sum_weight;
		return;
	}

	// finalize subtrees
	nodes[node_id].prob = (float).0;
	for (Node::TSubTreeIt i(nodes[node_id].sub_trees.begin()); i != nodes[node_id].sub_trees.end(); ++i)
    {
		finalize(*i);
		nodes[node_id].prob = max(nodes[node_id].prob, nodes[*i].prob);
	}

	// sort subtrees -- most probable are at the begining of nodes's subtree list
    struct SubtreeComparer 
	{
		const vector<Node> &nodes;

        SubtreeComparer(const vector<Node> &nodes)
			: nodes(nodes) {}
		
		bool operator() (const size_t &lhs, const size_t &rhs) const 
		{
			return nodes[lhs].prob > nodes[rhs].prob; 
		}
	 } comparer(nodes);

	sort(nodes[node_id].sub_trees.begin(), nodes[node_id].sub_trees.end(), comparer);
}





/*******************
*   TKeyboard      * 
********************/
TKeyboard::TKeyboard()
{

     /*
        south Slavic keyboard layout 
           - you can change layout for other keyboard types
		   - key can be positioned on multiple positions (e.g. spacebar)
		   - homologous non-caron characters are sometimes used instead of caron characters (e.g c insted of è or æ)
		   - z and y can be changed on some querty keyboards
     */

	const char col_delimiter('\1');
	const char spacebar('\2');

	const unsigned int n_rows = 5;
	string layout[n_rows] =
	         {
                "  `~ \1    1!      \1    2@  \1    3#    \1    4$    \1    5%€    \1    6^      \1    7&    \1    8*    \1    9(    \1    0)      \1    -_         \1  =+        ", 
                "     \1    Qq      \1    Ww  \1    Ee    \1    Rr    \1    Tt     \1    YyzZ    \1    Uu    \1    Ii    \1    Oo    \1    Op      \1   ŠšSs[{      \1  ÐðDd]}    ",
                "     \1    Aa      \1    Ss  \1    Dd    \1    Ff    \1    Gg     \1    Hh      \1    Jj    \1    Kk    \1    Ll    \1    ÈèCc;:  \1    ÆæCc'\"    \1  \\|ŽžZz   ", 
                "     \1    ZzYy    \1    Xx  \1    Cc    \1    Vv    \1    Bb     \1    Nn      \1    Mm    \1    ,<    \1    .>    \1    /?      \1               \1            ",
                "     \1            \1        \1    \2    \1    \2    \1    \2     \1    \2      \1    \2    \1    \2    \1    \2    \1            \1               \1            "  
	         };

	for (unsigned int row(0); row < n_rows; ++row)
	{
		const string& line(layout[row]);

		unsigned char col(0);
    	for (unsigned int j(0); j < line.size(); ++j)
		{
			if ((const char)line[j] == col_delimiter)
				++col;

			if (line[j] == spacebar) // encoded space character
					keyboard[(unsigned char)' '].insert(Pos(row, col));
			else
			if (line[j] != ' ')
				keyboard[(unsigned char)line[j]].insert(Pos(row, col));
		}
	}

}


unsigned int TKeyboard::distance(const Pos &p, const Pos &q)
{
	// hamming distance
	unsigned int result(abs(p.row - q.row) + abs(p.col - q.col));

	// diagonal neighbours distance is set to 1
	if (result == 2 && p.row != q.row && p.col != q.col)
		return 1;

	return result;
}

unsigned int TKeyboard::distance(const unsigned char p, const unsigned char q)
{
	// taking care for off keyboard characters
    unsigned int result(20);

	if (keyboard[p].empty() || keyboard[q].empty())
	   return result;

   typedef set<Pos>::const_iterator TIt;

   // find min hamming distance
   for (TIt ip(keyboard[p].begin()); ip != keyboard[p].end(); ++ip)
      for (TIt iq(keyboard[q].begin()); iq != keyboard[q].end(); ++iq)
		  result = min(result, distance(*ip, *iq));

   return result;
}



