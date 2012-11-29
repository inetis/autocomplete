Autocomplete (word completeion) function predicts word user intended to type in the search box.
 
Autocomplete takes a dictionary of weighted words and produces a list of suggestions for user's query.
 
The results on cities database:
 
    query       suggestions
    -----------------------------------------------------------------------------------------
    Lis Agne    Los Angeles, Los Angelitos, Les Agnels, Les Agneliers, Les Agneliers Bas
    nw yr       New York, Nwoya, Nweyon, Na Yan, Nwarem
    cpenh       Copenhagen, Cepni, Spencer, Cienega, Cuenha
    -----------------------------------------------------------------------------------------
 
Autocomplete is implemented in C++. Use it as follows:
 
    TAutocomplete ac;
    ac.load("cities.txt");                 // load dictionary once
    vector<string> suggestions;            // autocomplete suggestions
 
    ac.autocomplete("cpenh", suggestions); // find suggestions for input
                                           // query "cpenh"; max number
                                           // of sugegstions (default 5)
                                           // can be set as the last
                                           // parameter of the method`
 
See also: http://blog.inetis.com/improved-autocomplete-function-for-google-maps
