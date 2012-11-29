Autocomplete (word completion) function predicts word user intended to type in the search box.


## Features

* Takes into account word similarity and weights suggestion importance
* Able to handle large dictionaries 
* Fast

## Examples

    query       Suggestions
    -----------------------------------------------------------------------------------------
    Lis Agne    Los Angeles, Los Angelitos, Les Agnels, Les Agneliers, Les Agneliers Bas
    nw yr       New York, Nwoya, Nweyon, Na Yan, Nwarem
    cpenh       Copenhagen, Cepni, Spencer, Cienega, Cuenha
    -----------------------------------------------------------------------------------------


## Technical information

http://blog.inetis.com/improved-autocomplete-function-for-google-maps

## Usage

```sh
TAutocomplete ac;
ac.load("cities.txt");                 // load dictionary once
vector<string> suggestions;            // autocomplete suggestions

ac.autocomplete("cpenh", suggestions); // find suggestions for input
                                       // query "cpenh"; max number
                                       // of sugegstions (default 5)
                                       // can be set as the last
                                       // parameter of the method
```


## Where is it being tested?

* Microsoft Visual Studio C++ Version 10.0.30319.1 on Windows XP
* GCC 4.6, 4.7 on FreeBSD and Fedora Linux


## License

Autocomplete is licensed under MIT http://www.opensource.org/licenses/MIT

### Copyright

Copyright (c) 2012, Matevz Kovacic, Inetis Ltd <info@inetis.com>