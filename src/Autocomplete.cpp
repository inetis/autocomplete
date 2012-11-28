
#include "Autocomplete.h"

#include <stdexcept>
using std::runtime_error;

#include <algorithm>
using std::find;

void TAutocomplete::autocomplete(const string         &query,  
	                                   vector<string> &suggestions,
						         const size_t         max_suggestions)
{
	suggestions.clear(); 

	string::const_iterator begin(query.begin());
	string::const_iterator end(query.end());
	while (begin != end && *begin == ' ')
		++begin;

	if (begin != end)
		autocomplete(begin, end, suggestions, max_suggestions);
}

bool goal(const TCandidate              &candidate,
          const string::const_iterator  &query_end, 
		        float                   &min_suggestion_prob,
                vector<string>          &suggestions)
{
	if (candidate.node->sub_trees.size() > 0)  // leaf has no subtrees
		return false;

    if (candidate.query != query_end)  // query must be already matched at trie leaf
		return true;

	// remove string delimiter in trie
	string suggestion(candidate.suggestion.substr(0, candidate.suggestion.size() - 1));
	// no duplicates in results allowed
	if (find(suggestions.begin(), suggestions.end(), suggestion) == suggestions.end())
	{
		// set criterion that P(solution) must be greater that P(best solution) / 100 
		if (suggestions.empty())
			min_suggestion_prob = candidate.probability / (float)100.;

		suggestions.push_back(suggestion);
	}
	
	return true;
}

//
//
// perform autocomplete using best-first search over trie
//
void TAutocomplete::autocomplete(const string::const_iterator  &query_begin, 
			                     const string::const_iterator  &query_end, 
						               vector<string>          &suggestions,
						         const size_t                   max_suggestions)
{ 
   TCandidates candidates;

   candidates.push(TCandidate(trie.root(),     // start at the trie root
					 		  query_begin,     // at the beginning of the user query
					 		  "",              // with empty suggestion
							  (float)1.,       // with all probability mass assigned to empty query
							  0));             // and no typing errors so far

   float    min_suggestion_prob((float).0);    // min probability of acceptable candidate
   unsigned int iteration(0);

   do
   {
	   TCandidate candidate(candidates.top());	  
	   candidates.pop();

	   if (candidate.probability < min_suggestion_prob ||              // no probable candidates left
           (min_suggestion_prob == (float).0 && ++iteration > 10000))  // no solution found in first 1000 iterations
		   break;  

	   if ( ! goal(candidate, query_end, min_suggestion_prob, suggestions) ) 
		   expand(candidate, candidates, query_begin, query_end, min_suggestion_prob);

   } while (candidates.size() > 0 && suggestions.size() < max_suggestions);
}




//
// general all possible corrections of current candidate
//    - all one edit operations on current query candidate are considered
//
void TAutocomplete::expand(const TCandidate                &candidate, 
	                             TCandidates               &candidates, 
						   const string::const_iterator    &query_begin,
				           const string::const_iterator    &query_end,
						   const float                     &min_prob)
{
	if (candidate.query == query_end)  // query is alreay matched to trie interior node ->
	{
       	expand_matched_query(candidate, candidates);
		return;
	}

	// expand partially matched query at interior node of the trie

	TCandidate best(candidate);            // best candidate among subtrees
	float best_left, best_right;           // max probability among left/right subtrees of the best node 
	TAction best_action(candidate.begin);  // max probability candidate successor action 

	split(candidate, query_begin, query_end, best_left, best, best_right, best_action);
	add_candidates(candidates, candidate, min_prob, best_left, best, best_right, best_action);
}

void TAutocomplete::expand_matched_query(const TCandidate  &candidate, 
	                                           TCandidates &candidates) 
{
	// emulate depth first trie traversal as the most promising leaf is in the left subtree
	if (candidate.begin != candidate.end)
	{
		TAction action(candidate.begin);
		TTrie::Node &first(trie.node(*action.sub_tree));

		candidates.push(TCandidate(first,                            // advance in trie via sub_tree
			                       TAction(first, TAction::no_correction, first.sub_trees.begin()),
			                       TAction(first, TAction::no_correction, first.sub_trees.end()),
							       candidate.query,                          // query stays the same as it is already matched
			                       candidate.suggestion + first.c,           // add current node character to candidate suggestion
							       candidate.query_probability,              // query probability stays the same since query is already matched
								   candidate.query_probability * first.prob, // update candidate probability 
							       candidate.n_errors));                     // number of errors stays the same since query is already matched

		TAction::operation_t old_operation(action.operation);
		if (++action != candidate.end && action.operation == old_operation)  // prevent rolling to the next operation -> only allow one iteration over subtrees
			candidates.push(TCandidate(*candidate.node,                         // stay in the same node in trie
			                           TAction(*candidate.node, TAction::no_correction, action.sub_tree),
			                           TAction(*candidate.node, TAction::no_correction, action.node->sub_trees.end()),
			                           candidate.query,                         // query stays the same as we have not  moved in trie
							           candidate.suggestion,                    // suggestion stays the same as we have not  moved in trie
									   candidate.query_probability,             // query probability stays the same as the query is already matched
									   candidate.query_probability * trie.node(*action.sub_tree).prob,  // next best node is used for subtree list probability estimation
									   candidate.n_errors));                    // number of errors stays the same as the query is already matched
	}
}


/*
error modelling according to data in:
TrueKeys: Identifying and Correcting Typing Errors for People with Motor Impairments
Shaun K. Kane, Jacob O. Wobbrock, Mark Harniss, Kurt L. Johnson


typing error rate: 

Error type               Motor-impaired        Non-impaired
--------------------------------------------------------------------
Insertions                 113 (76.4%)          45 (60.0%)
Deletions                    8 (5.4%)           12 (16.0%)
Substitutions               21 (14.2%)          13 (17.3%)
Transpositions               6 (4.1%)            5 (6.6%)
---------------------------------------------------------------------

- based on various studies http://panko.shidler.hawaii.edu/HumanErr/Basic.htm
  we assume (for unskilled typist) prior probability of typing error (per key pressed): 5%

Properties of error types: 
- insertion error  (60%)
   - usually near keyboard keys
   - repetition of the previous character

- substitution error:  (17%)
   - usually adjacent keyboard keys 

- deletion errors  (16%) 
   - usually medial characters
   
- transposition errors (6%)
  - usually cross-hands 

*/

void error_probabilities(const TCandidate              &candidate, 
	                           TKeyboard               &keyboard,
	                     const string::const_iterator  &query_begin,
		                       float                   &hit_prob, 
		                       float                   &insertion_prob, 
						       float                   &begin_insertion_penalty,
	                           float                   &substitution_prob, 
						       float                   &begin_substitution_penalty,
	                           float                   &deletion_prob, 
						       float                   &transposition_prob)
{
	insertion_prob     = (float).16;   // 16% deletion errors -> 16% insertion prob
    substitution_prob  = (float).17; 
    deletion_prob      = (float).60;   // 60% insertion errors errors -> 60% deletion prob
	transposition_prob = (float).06;  

	begin_insertion_penalty    = (float).05;
	begin_substitution_penalty = (float).1;

	// insertion error less likely at the beginning of query
	if (candidate.query == query_begin)
		deletion_prob *= (float).05;
	else
	if (candidate.query == query_begin + 1)
		deletion_prob *= (float).1;
	else // insertion error usually at near keys
		if (keyboard.distance((unsigned char)*(candidate.query), candidate.node->c) > 2)
			deletion_prob *= (float).25;
	
	// weight with error probability per key pressed
	const float keypress_error_prob((float).05);             // per-key pressed probability of error
	hit_prob = (float)1.0 - keypress_error_prob;             // per-key pressed probability of entering correct key

	insertion_prob     *= keypress_error_prob;   
    substitution_prob  *= keypress_error_prob; 
    deletion_prob      *= keypress_error_prob;   
	transposition_prob *= keypress_error_prob;  
}


/*
   orders candidates successor states into ordered list: [left candidates, best candidate, right candidates] 
   return best candidate and admissible probability estimate of left and right candidate sets
*/
void TAutocomplete::split(const TCandidate              &candidate,
	                      const string::const_iterator  &query_begin,
				          const string::const_iterator  &query_end,
	                             float                  &best_left,
	                             TCandidate             &best,
						         float                  &best_right,
								 TAction                &best_action)
{
	// probabilities for various types of errors
	float hit_prob, insertion_prob, substitution_prob, deletion_prob, transposition_prob;
	float begin_insertion_penalty, begin_substitution_penalty;
	
	error_probabilities(candidate, keyboard, query_begin,
		                hit_prob, 
		                insertion_prob,    begin_insertion_penalty,
	                    substitution_prob, begin_substitution_penalty,
	                    deletion_prob, 
						transposition_prob);

	float sum_transition_no_correction((float).0);   // normalization factor for avancing without correction
	float sum_transition_insert((float).0);          // normalization factor for insert characters actions
	float sum_transition_substitute((float).0);      // normalization factor for substitute characters actions 

	best.probability = best_left = best_right = (float).0;     
	best_action = candidate.begin;

	for (TAction action(candidate.begin); action != candidate.end; ++action)
		switch (action.operation)
		{
		   case TAction::no_correction: 
			   {
				   if (expand_no_correction(hit_prob, sum_transition_no_correction, candidate, action, query_end, best_left, best, best_right))
					   best_action = action;
				   break;
			   }

           case TAction::insert_char: 
			   {				  
				   if (expand_substitute_char(true,  insertion_prob, sum_transition_insert, candidate, action, query_begin, query_end, begin_insertion_penalty, best_left, best, best_right))
					   best_action = action;
					  
				   break;
			   }

           case TAction::substitute_char: 
			   {				  
				   if (expand_substitute_char(false,  substitution_prob, sum_transition_substitute, candidate, action, query_begin, query_end, begin_substitution_penalty, best_left, best, best_right))
					   best_action = action;
				   break;
			   }

		   case TAction::delete_char: 
			   {
				   if (expand_delete_char(deletion_prob, candidate, query_end, best_left, best, best_right))
					   best_action = action;
					
				   break;
			   }

		   case TAction::transpose_char: 
			   {				  				  
				   if (expand_transpose_char(transposition_prob, candidate, query_end, best_left, best, best_right))
					   best_action = action;
					
				   break;
			   }

		   default:
			   throw std::runtime_error("illegal action in TAutocomplete::split");
	    }

}


string::const_iterator next_char(string::const_iterator begin, const string::const_iterator end)
{
	if (begin == end || ++begin == end || *begin != ' ')  // try to skip to the next non-blank char
		return begin;

	string::const_iterator next(begin + 1);
	while (next != end && *next == ' ')
	{
		begin = next;
		++next;
	}

	if (next == end)
		return next;

	return begin;
}

bool update_candidates(const TCandidate  &new_candidate,
	                         float       &best_left,
							 TCandidate  &best,
							 float       &best_right)
{
	if (best < new_candidate)
	{
		best_left  = best.probability;
		best       = new_candidate;
		best_right = (float).0;
		return true;
	} else
		if (best_right < new_candidate.probability)
			best_right = new_candidate.probability;

	return false;
}

bool TAutocomplete::expand_no_correction(const float                   &hit_prob, 
	                                           float                   &sum_transition_prob,
	                                     const TCandidate              &candidate,
										 const TAction                 &action,
									     const string::const_iterator  &query_end,
										       float                   &best_left,
											   TCandidate              &best,
											   float                   &best_right)
{
	if (sum_transition_prob == (float).0)
	{
		for (TTrie::Node::TSubTreeIt i(candidate.node->sub_trees.begin()); i != (candidate.node->sub_trees.end()); ++i)
			if (keyboard.distance(trie.node(*i).c, *candidate.query) == 0) 
				sum_transition_prob += hit_prob;
		
		if (sum_transition_prob == (float).0)
			sum_transition_prob = (float)-1.;  // prevent multiple calculation of number of hits
	}

	if (sum_transition_prob < (float).0)
		return false;

	TTrie::Node &sub_tree(trie.node(*action.sub_tree));

	if (keyboard.distance(sub_tree.c, *candidate.query) == 0) 
		return update_candidates(TCandidate(sub_tree,                                  // advance in trie via matched subtree
         					   		        next_char(candidate.query, query_end),     // advance query as we've found a match               
					                        candidate.suggestion + sub_tree.c,         // add matched subtree character to candidate suggestion
							                candidate.query_probability * hit_prob *   // query probability updated with keystroke hit rate
											hit_prob / sum_transition_prob,            // is normalized over all transitions in trie
							                candidate.n_errors),                       // number of errors stays the samae as we've found the match
                                 best_left,
					             best,
						         best_right);

	return false;
}

bool TAutocomplete::expand_substitute_char(const bool                    &insert_char, 
	                                       const float                   &substitution_prob, 
										         float                   &sum_transition_prob, 
										   const TCandidate              &candidate, 
										         TAction                 &action, 
			                               const string::const_iterator  &query_begin, 
										   const string::const_iterator  &query_end, 
										   const float                   &begin_penalty,
									             float                   &best_left, 
												 TCandidate              &best, 
												 float                   &best_right)
{
	if (sum_transition_prob == (float).0)
		for (TTrie::Node::TSubTreeIt i(candidate.node->sub_trees.begin()); i != (candidate.node->sub_trees.end()); ++i)
		{
			float prob;
		    bool  exact_match;
			
			if ( transition_prob(candidate, trie.node(*i), query_begin, begin_penalty, prob, exact_match) &&
				(insert_char || ! exact_match)) // with substitution exatch match does not count as it is already handled by expand_exact_match(...)
			sum_transition_prob += prob;
		}

	if (sum_transition_prob == (float).0)
		return false;

	float prob;
	bool  exact_match;

	TTrie::Node &succ_node(trie.node(*action.sub_tree));
    if ( transition_prob(candidate, succ_node, query_begin, begin_penalty, prob, exact_match) &&
		 (insert_char || ! exact_match))

		 return update_candidates(TCandidate(succ_node,                                            // advance in trie via succ_node subtree
						  	                 insert_char ? candidate.query :                       // if char is inserted query must stary the same      
									                next_char(candidate.query, query_end),         // if char is updated query is advanced to the next char
							                 candidate.suggestion + succ_node.c,                   // add succ_node subtree character to candidate suggestion
							                 candidate.query_probability *                         // query probability update 
							                 substitution_prob * prob / sum_transition_prob,       // is normalized over all transitions in trie
							                 candidate.n_errors + 1),                              // substitution increases number of errors
                                 best_left,
					             best,
						         best_right);

	return false;
}

bool TAutocomplete::transition_prob(const TCandidate              &candidate,
	                                 TTrie::Node                  &subtree,
					                const string::const_iterator  &query_begin,
                                    const float                   &begin_penalty,
									      float                   &transition_prob,
							              bool                    &exact_match)
{
	if (subtree.c == (char)0) // leaf node -> no expansion allowed 
		return false;

	unsigned int distance(keyboard.distance(subtree.c, *candidate.query));

	if (distance == 0)
		transition_prob = (float).95;
	else
	if (distance == 1)
		transition_prob = (float).1;
	else
	if (distance < 4)
		transition_prob = (float).05;
	else
	if (distance < 8)
		transition_prob = (float).0025;
	else
		transition_prob = (float).00005;

	if (candidate.query == query_begin && distance > 0)
		transition_prob *= begin_penalty;  // operation at the beginning of query is less likely

	exact_match = distance == 0;
	return true;
} 


bool TAutocomplete::expand_delete_char(const float                   &deletion_prob,
	                                   const TCandidate              &candidate, 
									   const string::const_iterator  &query_end,
									         float                   &best_left, 
											 TCandidate              &best, 
											 float                   &best_right) 
{
		return update_candidates(TCandidate(*candidate.node,                              // no advance in trie
		                                    candidate.begin,
		                                    candidate.end,
							                next_char(candidate.query, query_end),        // delete character by advancing in user query
			                                candidate.suggestion,                         // candidate suggestion stays the same
							                candidate.query_probability * deletion_prob,  // query probability is updates
							                candidate.n_errors + 1),                      // deletion adds one more error
                                 best_left,
					             best,
						         best_right);
}



bool TAutocomplete::expand_transpose_char(const float                   &transposition_prob,
                                          const TCandidate              &candidate, 
								          const string::const_iterator  &query_end,
										        float                   &best_left, 
											    TCandidate              &best, 
											    float                   &best_right) 
{

	TTrie::Node *transposition_end(nullptr);
	string transposition;
	if (transpose(candidate, query_end, transposition, &transposition_end))
		return update_candidates(TCandidate(*transposition_end,                                // advance to the node after transposition
							                next_char(candidate.query + 1, query_end),         // skip two query characters because of transposition
			                                candidate.suggestion + transposition,              // add transposition to suggestion
							                candidate.query_probability * transposition_prob,  // query probability is updated
							                candidate.n_errors + 1),                           // transposition adds one more error
                                 best_left,
					             best,
						         best_right);

	return false;
}



bool TAutocomplete::transpose(const TCandidate              &candidate, 
	                          const string::const_iterator  &query_end,
							        string                  &transposition,
                                    TTrie::Node             **transposition_end)
{
	if (candidate.query + 1 == query_end)
		return false;

	// the next query character must match one of the subtrees
	char next_char(*(candidate.query + 1));
	for (TTrie::Node::TSubTreeIt i(candidate.node->sub_trees.begin()); i != candidate.node->sub_trees.end(); ++i)
	{
		TTrie::Node &subtree(trie.node(*i));
		if (keyboard.distance(next_char, subtree.c) == 0)
		{
			for (TTrie::Node::TSubTreeIt i(subtree.sub_trees.begin()); i != subtree.sub_trees.end(); ++i)
			{
				*transposition_end = &trie.node(*i);
				if (keyboard.distance(*candidate.query, (*transposition_end)->c) == 0)
				{
					transposition  = subtree.c;
					transposition += (*transposition_end)->c;
					return true;
				}
			}

			return false;
		}
	}

	return false;
}


void TAutocomplete::add_candidates(      TCandidates &candidates, 
	                               const TCandidate  &candidate, 
					               const float       &min_prob, 
					               const float       &best_left, 
					               const TCandidate  &best, 
					               const float       &best_right, 
					                     TAction     &best_action)
{
	if (best.probability > min_prob)
	{ 
		// best node
		candidates.push(best);

		// left subtree
		if (best_left > min_prob)
			candidates.push(TCandidate(*candidate.node,                // no advance in trie
		                               candidate.begin,                // expand from leftmost action
									   best_action,                    // until best action
									   candidate.query,                // query stays the same
	                                   candidate.suggestion,           // suggestion stays the same
	                                   candidate.query_probability,    // query probability stays the same
									   best_left,                      // best node in left subtreees is set as a probability estimate
	                                   candidate.n_errors));           // number of error stays the same as sugegstion stays the same and we have not advanced in trie

		// right subtree
		if (best_right > min_prob)
			candidates.push(TCandidate(*candidate.node,                // no advance in trie
		                               ++best_action,                  // expand from after best action
									   candidate.end,                  // until end
									   candidate.query,                // query stays the same
	                                   candidate.suggestion,           // suggestion stays the same
	                                   candidate.query_probability,    // query probability stays the same
									   best_right,                     // best node in right subtreees is set as a probability estimate
	                                   candidate.n_errors));           // number of error stays the same as sugegstion stays the same and we have not advanced in trie
	}
}

void TAutocomplete::load(const string &file_name)
{
	trie.load(file_name);
}
