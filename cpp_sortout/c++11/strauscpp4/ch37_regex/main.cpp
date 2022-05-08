#include <iostream>
#include <string>
#include <regex>

using namespace std;

/*

New features:
* Regular expressions: regex (37)

Examples:
1. regex_match
2. regex_search
3. regex_replace
4. Iterators

*/

// 1. regex_match
// 2. regex_search
void show_regex_match() {

    // To look for a pattern matching a whole sequence with a known length, 
    // such as a line of text, use regex_match():

    // Simple regular expression matching
    std::string fnames[] = { "foo.txt", "bar.txt", "baz.dat", "zoidberg" };
    std::regex txt_regex("[a-z]+\\.txt");

    for (const auto &fname : fnames) {
        std::cout << fname << ": " << std::regex_match(fname, txt_regex) << '\n';
    }

    // Extraction of a sub-match
    // file with txt extension
    std::regex base_regex("([a-z]+)\\.txt");
    std::smatch base_match;

    for (const auto &fname : fnames) {
        if (std::regex_match(fname, base_match, base_regex)) {
            // The first sub_match is the whole string; the next
            // sub_match is the first parenthesized expression.
            for (size_t i = 0; i < base_match.size(); ++i) {
                std::ssub_match base_sub_match = base_match[i];
                std::string base = base_sub_match.str();
                std::cout << fname << " has a base of " << base << '\n';
            }
        }
    }

    // Extraction of several sub-matches
    // symbols before dot and after dot
    std::regex pieces_regex("([a-z]+)\\.([a-z]+)");
    std::smatch pieces_match;

    for (const auto &fname : fnames) {
        if (std::regex_match(fname, pieces_match, pieces_regex)) {
            std::cout << fname << '\n';
            for (size_t i = 0; i < pieces_match.size(); ++i) {
                std::ssub_match sub_match = pieces_match[i];
                std::string piece = sub_match.str();
                std::cout << "  submatch " << i << ": " << piece << '\n';
            }
        }
    }
    // To look for a pattern in a part of an arbitrary length sequence, such as a file, use regex_search()
}

// 3. regex_replace
void show_regex_replace() {

    std::string text = "Quick brown fox";
    
    // alternatives
    std::regex vowel_re("a|e|i|o|u");

    // write the results to an output iterator
    std::regex_replace(std::ostreambuf_iterator<char>(std::cout),
        text.begin(), text.end(), vowel_re, "*");

    // construct a string holding the results
    std::cout << '\n' << std::regex_replace(text, vowel_re, "[$&]") << '\n';
}

// 4. Iterators
void show_regex_iterators() {
    std::string s{ "aa aas awwa ss" };
    
    // string to the whitespace
    std::string expr{R"(\s+(\w+))"};
    std::regex r(expr);

    // iterate over match
    for (std::sregex_iterator it = { s.begin(), s.end(), r }; it != std::sregex_iterator{}; ++it) {

        for (size_t i = 0; i < (*it).size(); ++i) {
            std::ssub_match sub_match = (*it)[i];
            std::string piece = sub_match.str();
            std::cout << "  submatch " << i << ": " << piece << '\n';
        }
    }

    std::string text = "Quick brown fox.";

    // iterate over submatches

    // tokenization (non-matched fragments)
    // Note that regex is matched only two times: when the third value is obtained
    // the iterator is a suffix iterator.
    
    // whitespace, one or more
    std::regex ws_re("\\s+"); 

    // -1 inverts submatch

    // so, search whitespaces
    std::copy(std::sregex_token_iterator(text.begin(), text.end(), ws_re, 1),
        std::sregex_token_iterator(),
        std::ostream_iterator<std::string>(std::cout, "\n"));

    // ...search words (between whitespaces)
    std::copy(std::sregex_token_iterator(text.begin(), text.end(), ws_re, -1),
        std::sregex_token_iterator(),
        std::ostream_iterator<std::string>(std::cout, "\n"));
}

int main() {
    show_regex_match();
    show_regex_replace();
    show_regex_iterators();
    return 0;
}
