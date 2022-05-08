Questions:
-1 strategy - 37.4.2

New C++14 features:
* Regular expressions: regex (37)

Advices:
* In <regex>, the standard library provides regular expressions
	* regex_match(): Match a regular expression against a string (of known size).
	* regex_search(): Search for a string that matches a regular expression in an (arbitrarily long) stream of data
	* regex_replace(): Search for strings that match a regular expression in an (arbitrarily long) stream of data and replace them
	* regex_iterator: iterate over matches and submatches
	* regex_token_iterator: iterate over non-matches
* The regex library can recognize several variants of the notation for regular expressions. The default notation used 
  a variant of the ECMA standard used for ECMAScript (more commonly known as JavaScript)
* Like string, regex is an alias for the version that uses chars:
using regex = basic_regex<char>;
* The meaning of regular expression patterns is controlled by syntax_option_type constants defined
  identically in regex_constants and regex. Use the default unless you have a good reason not to
  Good reasons include a large body of exist- ing regular expressions in a non-default notation
* The main use of regex is through the search, match, and replace functions, 
  but there are also a few operations on regex itself (flags, locale etc)
* There is no (standard) way of reading out its pattern. If you need to output a pattern, 
  keep a copy of the string used to initialize
* Results from a regular expression match are gathered in a match_results object 
  which contains one or more sub_match objects
* As for basic_string and basic_ostream, a few standard aliases are provided for the most common match_results:
using cmatch = match_results<const char*>;
using wcmatch = match_results<const wchar_t*>;
using smatch = match_results<string::const_iterator>; 
using wsmatch = match_results<wstring::const_iterator>;

* We can subscript a regex_match to access a sub_match, for example, m[i]
  If a subscript, i, refers to a nonexistent sub_match, the result of m[i] represents an unmatched sub_match
* In regex_replace(), formatting is done using a format() function
* The functions for applying regular expression patterns to data are regex_search() for searching 
  in a sequence of characters, regex_match() for matching a fixed-length sequence of characters, 
  and regex_replace() for doing replacement of patterns
* Submatches do not have to be output in order:
* A regex_iterator is a bidirectional iterator that searches a sequence 
  for the next match of a pattern when incremented
* The usual set of aliases is provided:
using cregex_iterator = regex_iterator<const char*>;
using wcregex_iterator = regex_iterator<const wchar_t*>;
using sregex_iterator = regex_iterator<string::const_iterator>; 
using wsregex_iterator = regex_iterator<wstring::const_iterator>;

* A regex_token_iterator is an adaptor for regex_iterator that iterates over sub_matches of the match_results found
* The -1 option basically inverts the strategy for reporting matches by representing each character sequence 
  that does not match as a sub_match. This is often referred to as token splitting
* A regex_traits<T> represents the correspondence between a character type, a string type, and a
  locale as needed for a regex implementer


Book advices: 