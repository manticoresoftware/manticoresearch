/*
   Copyright (c) 2013, ebuinfo.com
Author: veelion@ebuinfo.com
All rights reserved.
*/

#include "cedarpp.h"

#include <cstdio>
#include <cstdlib>
#include <stdint.h>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>

// this trie_value_t should be same as what in sphinx.cpp
// trie value is not used, so just keep it one byte
typedef char trie_value_t;
typedef cedar::da<trie_value_t> trie_t;


using namespace std;


void loadwords(std::vector<std::string> &words, const char* fn_words) {
	char* line = 0;
	ssize_t read = 0;
	size_t  size = 0;
	words.clear();
	FILE* fp = std::fopen (fn_words, "r");
	while ((read = getline (&line, &size, fp)) != -1){
		// one word one line
		if (line[0] == '#') {
			printf("comment line: %s\n", line);
			continue;
		}
		// strip space, \n, \t at both end
		int head = 0;
		while(line[head] == ' ' ||
				line[head] == '\n' ||
				line[head] == '\t') {
			head++;
		}
		if (line[head] == '#') {
			printf("comment line: %s\n", line);
			continue;
		}
		int new_len = strlen(line+head);
		if(head) {
			memcpy(line, line+head, new_len);
		}
		new_len--;
		while(line[new_len] == ' ' ||
				line[new_len] == '\n' ||
				line[new_len] == '\t') {
			line[new_len] = 0;
			new_len--;
		}
		new_len++;
		//printf("new line:%s, %d\n", line, new_len);
		//skip short word only with one Chinese character
		if(new_len <= 3) continue;
		string word = string(line);
		words.push_back(word);
	}
	std::fclose (fp);
	// sort words to speed up trie search
	sort(words.begin(), words.end());
	for (string &s : words) {
		cout << s << endl;
	}
}


int make_dict(const char* fn_words, const char* fn_dict) {
	std::vector<string> words;
	loadwords(words, fn_words);
	if(words.empty()) {
		printf("no words loaded\n");
		return 1;
	}
	trie_value_t default_value = 8; // any value is ok
	trie_t dict;
	trie_t::result_pair_type result_pair[32];
	for(int i = 0; i < words.size(); i++) {
		dict.update(words[i].c_str(), words[i].size(), default_value);
	}

	if (dict.save (fn_dict) != 0) {
		std::fprintf (stderr, "cannot save dict: %s\n", fn_dict);
		std::exit (1);
	}
	//
	std::fprintf (stderr, "#keys: %ld\n", dict.num_keys ());
	std::fprintf (stderr, "size: %ld\n", dict.size ());
	std::fprintf (stderr, "nonzero_size: %ld\n", dict.nonzero_size ());
	return 0;
}


int main (int argc, char **argv) {
	if (argc < 3) {
		std::fprintf (stderr, "Usage: %s file_words file_dictionary_to_save \n", argv[0]);
		std::exit (1);
	}
	make_dict(argv[1], argv[2]);

	return 0;
}
