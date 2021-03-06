// © 2013 the Search Authors under the MIT license. See AUTHORS for the list of authors.

#include "utils.hpp"

struct Strings {
	std::string path, dir, base;
};

static Strings strs[] = {
  	{ "", ".", "." },
	{ "/", "/", "." },
	{ "///", "//", "." },
	{ ".", ".", "." },
	{ "/hello", "/", "hello" },
	{ "/hello/", "/hello", "." },
	{ "/hello//", "/hello/", "." },
	{ "///hello/", "///hello", "." },
	{ "/hello/there", "/hello", "there" },
	{ "/hello/.", "/hello", "." },
	{ "hello", ".", "hello" },
	{ "hello/there", "hello", "there" },
	{ "hello///there", "hello//", "there" },
	{ "hello///there///", "hello///there//", "." },
	{ "hello/.", "hello", "." },
	{ "././.", "./.", "." },};

bool test_basename() {
	bool ok = true;
	for (unsigned int i = 0; i < sizeof(strs) / sizeof(strs[0]); i++) {
		std::string b = basename(strs[i].path);
		if (b == strs[i].base)
			continue;
		testpr("path [%s]: expected [%s], got [%s]\n", strs[i].path.c_str(),
			strs[i].base.c_str(), b.c_str());
		ok = false;
	}
	return ok;
}

bool test_dirname() {
	bool ok = true;
	for (unsigned int i = 0; i < sizeof(strs) / sizeof(strs[0]); i++) {
		std::string d = dirname(strs[i].path);
		if (d == strs[i].dir)
			continue;
		testpr("path [%s]: expected [%s], got [%s]\n", strs[i].path.c_str(),
			strs[i].dir.c_str(), d.c_str());
		ok = false;
	}
	return ok;
}