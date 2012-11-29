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

#include <string>
using std::string;

#include <vector>
using std::vector;

#include <queue>
using std::priority_queue;

#include "AutocompleteUtils.h"

class TAutocomplete
{
    private:
		TTrie     trie;
		TKeyboard keyboard;

		typedef priority_queue<TCandidate> TCandidates;

		// autocomplete routines	
		void autocomplete(const string::const_iterator &begin, const string::const_iterator &end, vector<string> &suggestions, const size_t max_suggestions);
		void expand(const TCandidate &candidate, TCandidates &candidates, const string::const_iterator &query_begin, const string::const_iterator &query_end, const float &min_prob);
		void split(const TCandidate &candidate, const string::const_iterator &query_begin, const string::const_iterator &query_end,
			       float &best_left, TCandidate &best, float &best_right, TAction &action);
        void add_candidates(TCandidates &candidates, const TCandidate  &candidate,  const float &min_prob, 
					        const float &best_left, const TCandidate  &best, const float &best_right, TAction &best_action);
		// generation of successor candidates
        void expand_matched_query(const TCandidate &candidate, TCandidates &candidates);
		bool expand_no_correction(const float &hit_prob, float &sum_transition_prob, const TCandidate &candidate, const TAction &action, 
			                      const string::const_iterator &query_end, float &best_left, TCandidate &best, float &best_right);
		bool expand_substitute_char(const bool &insert_char, const float &substitution_prob, float &sum_transition_prob, const TCandidate &candidate, TAction &action, 
			                        const string::const_iterator  &query_begin, const string::const_iterator  &query_end, const float &begin_penalty,
									float &best_left, TCandidate &best, float &best_right);
        bool expand_delete_char(const float &deletion_prob, const TCandidate  &candidate, const string::const_iterator &query_end,
			                    float &best_left, TCandidate &best, float &best_right);    	
        bool expand_transpose_char(const float &transposition_prob, const TCandidate &candidate, const string::const_iterator &query_end,
			                       float &best_left, TCandidate &best, float &best_right);

		// utility routines
        bool transition_prob(const TCandidate &candidate, TTrie::Node &subtree, const string::const_iterator  &query_begin, 
                              const float &begin_penalty, float &transition_prob, bool &exact_match);

		bool transpose(const TCandidate &candidate, const string::const_iterator &query_end, string &transposition, TTrie::Node **transposition_end);

    public:
		
		void autocomplete(const string         &query,  // no need to normalize query
			                    vector<string> &suggestions,
						  const size_t         max_suggestions = 5);

		void load(const string &file_name);
};


