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

#pragma once

// auxilliary structs for TAutocomplete class

#include <string>
using std::string;

#include <vector>
using std::vector;

#include <set>
using std::set;

//
//  TTrie
//    - words in trie are weighted 
//    - (char)0 is reserved for word terminator in Trie
//    - nodes in trie are weighted by max subtree word
//    - node subtrees are stored in descending order by weight

class TTrie
{
    public:

		TTrie();

		void load(const string file_name);

		struct Node
		{
			Node(char c, float prob)
				: c(c), prob(prob) {}

			char           c;            // Node character
			float          prob;         // probability of the most frequent word in trie rooted at Node
			vector<size_t> sub_trees;    

			typedef vector<size_t>::const_iterator TSubTreeIt;
		};
		
		Node &root() { return nodes[0]; };
		Node &node(const size_t index) { return nodes[index]; };

    private:

		vector<Node>  nodes;
		float         sum_weight;

		void add(const string s, float weight);
		void add(const size_t node_id, string::const_iterator begin,  const string::const_iterator end,  const float weight);
		void finalize(const size_t node_id);
};


//
//  state information for autocomplete search
//

struct TAction // action performed on candidate node
{
	enum operation_t {insert_char, no_correction, substitute_char, delete_char, transpose_char, no_op}  // order in enum important -> TCandidate constructor depends on it
		      operation;

	TAction(TTrie::Node &node, const operation_t &operation, const TTrie::Node::TSubTreeIt &sub_tree)
			: node(&node), operation(operation), sub_tree(sub_tree) {}

	TTrie::Node             *node;
	TTrie::Node::TSubTreeIt sub_tree;

    TAction& operator=(const TAction &rhs)
    {
		if (this != &rhs)
		{
			operation = rhs.operation;
			node      = rhs.node;
			sub_tree  = rhs.sub_tree;
		}

		return *this;
    }

	bool operator!=(const TAction &rhs) const
    {
		return operation != rhs.operation || node != rhs.node || sub_tree != rhs.sub_tree;	
	}

    TAction& operator++()
	{
		if (operation == delete_char    ||  // one time operation on node - no iteration over subtrees needed
			operation == transpose_char ||  // one time operation on node - no iteration over subtrees needed 
			sub_tree == node->sub_trees.end() || ++sub_tree == node->sub_trees.end() )
		{
			operation = static_cast<operation_t>(operation + 1);
			sub_tree  = node->sub_trees.begin();
		} 

		return *this;
	}
};


struct TCandidate
{
	TCandidate(      TTrie::Node             &node, 
		       const TAction                 &begin,
	           const TAction                 &end,
	           const string::const_iterator  &query,
	           const string                  &suggestion,
	           const float                   &query_probability,
	           const unsigned int            &n_errors)
		  	     : node(&node), begin(begin), end(end),  query(query), suggestion(suggestion), 
			 	   query_probability(query_probability), probability(query_probability * node.prob), 
				   n_errors(n_errors) { }
				   
	TCandidate(      TTrie::Node             &node, 
		       const TAction                 &begin,
	           const TAction                 &end,
	           const string::const_iterator  &query,
	           const string                  &suggestion,
	           const float                   &query_probability,
	           const float                   &probability,
	           const unsigned int            &n_errors)
		  	     : node(&node), begin(begin), end(end),  query(query), suggestion(suggestion), 
			 	   query_probability(query_probability), probability(probability), 
				   n_errors(n_errors) { }

	TCandidate(      TTrie::Node             &node, 
	           const string::const_iterator  &query,
	           const string                  &suggestion,
	           const float                   &query_probability,
	           const unsigned int            &n_errors)
		  	     : node(&node), 
				   begin(node, TAction::insert_char, node.sub_trees.begin()), // first possible action
				   end(node,   TAction::no_op, node.sub_trees.begin()),  // last possible action
				   query(query), suggestion(suggestion), 
			 	   query_probability(query_probability), probability(query_probability * node.prob), 
				   n_errors(n_errors) 
	           { 
				   if (node.sub_trees.begin() == node.sub_trees.end())  // specila case of empty sub_tree
				      begin = end;
               }

    TTrie::Node              *node;
	TAction                  begin;
	TAction                  end;
	string::const_iterator   query;
	string                   suggestion;
	float                    query_probability;
	float                    probability;
	unsigned int             n_errors;  // currently not used - might be useful for alternative error probability distrubutions


	TCandidate& operator=(const TCandidate &rhs)
	{
		if (this != &rhs)
		{
		    node               = rhs.node;
			begin              = rhs.begin;
			end                = rhs.end;
			query              = rhs.query;       
			suggestion         = rhs.suggestion;
			query_probability  = rhs.query_probability;
			probability        = rhs.probability;
			n_errors           = rhs.n_errors;
		}

		return *this;
	}

	bool operator<(const TCandidate &rhs) const
	{  
		return probability < rhs.probability;
	}
};


class TKeyboard
{
    private:

	   struct Pos  // position of a key on the keyboard
	   {
		   Pos()
			   : row((unsigned char)255), col((unsigned char)255) {}
		   
		   Pos(const unsigned char row, const unsigned char col)
			   : row(row), col(col) {}
		   
		   unsigned char row;
		   unsigned char col;
		   
		   bool operator==(const Pos &rhs) const
		   {
			   return row == rhs.row && col == rhs.col;
		   }

		   bool operator<(const Pos &rhs) const
		   {
			   if (row < rhs.row)
				   return true;

			   if (row > rhs.row)
				   return false;

			   return col < rhs.col;
		   }

		   Pos& operator=(const Pos &rhs)
		   {
			   if (this != &rhs)
			   {
				   row = rhs.row;
				   col = rhs.col;
			   }
			   
			   return *this;
		   }
       };
	   
	   set<Pos> keyboard[255];  // array of positions of characters in keyboard 

	   unsigned int distance(const Pos &p, const Pos &q);

    public:

		TKeyboard();

		unsigned int distance(const unsigned char p, const unsigned char q);
};



