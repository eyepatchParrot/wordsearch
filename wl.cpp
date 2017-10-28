//
// File: wl.cpp
// 
//  Description: Load a file 
//  Student Name: Peter Van Sandt
//  UW Campus ID: 9071889357
//  enamil: vansandt@wisc.edu
//
//  TODO Design Report
//  TODO Turn in wl.cpp, wl.h, deisgn report
//  TODO test on terminating whitespace
//
//  60% correctness, 30% design report, 10% style
//
//  Design report:
//  1. Explain your choice of the data structure that you implemented. Did you
//  consider any other data structures besides the one that you implemented?
//  How did you arrive at your final choice of the data structure?
//  2. What is the best, average, and worst case complexity of your
//  implementation of the locate command in terms of the number of words in the
//  file that you are querying? (you need to provide all three - best, average,
//  and worst-case analysis). For the complexity, I am only interested in the
//  big-Oh analysis.
//  3. What is the average case space complexity of your data structure in
//  terms of the number of words in the input file? In other words, using the
//  big-Oh notation what is the expected average size of your data structure in
//  terms of the number of words.
//
//  1) name the project directory: <lastname>_<firstname>_P1 (e.g. Musk_Elon_P1)
//  2) run: make clean from inside the directory (so that the submitted file size is small)
//  3) run: tar -czvf <lastname>_<firstname>_P1.tar.gz /path-to-the-project/lastname_firstname_P1
//  4) submit the tar file 
//   
//   As a check, you can uncompress the tar file (using the command:  tar - xzvf <lastname>_<firstname>_P1.tar.gz). You should see the Makefile and all the source code files (e.g. wc.cpp and wc.h) inside the directory. If you do not adhere to this standard, 5 points will be deducted.

#include <algorithm>
#include <assert.h>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdint.h>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "wl.h"

int WordIx::memCompare(const char* l, const char* r, const size_t sz) {
    // compare case insensitively
    int cmp = 0;
    for (size_t i = 0; cmp == 0 && i < sz; i++) {
        cmp = tolower(l[i]) - tolower(r[i]);
    }
    return cmp;
}

BinNode* BinNode::newBinNode(const WordIx* words, size_t nWords, size_t keySz) {
    // full binary tree has 2^n - 1 nodes. So find a value like that that
    // for which all nodes fit.
    size_t szNode = 1;
    while (szNode <= nWords) {
        szNode <<= 1;
    }
    szNode--;

    // since the size of a node depends on the number of keys, we don't know
    // the size in advance, so we have to handle memory ourselves.
//    char* mem = new char[sizeof(BinNode) + szNode * (sizeof(uint32_t) + keySz)];
    void* mem = (BinNode*)::operator new(sizeof(BinNode) + szNode * (sizeof(uint32_t) + keySz));
    BinNode* n = new (mem) BinNode(words, nWords, szNode, keySz);
    assert(mem == (void*)n);
    return n;
}

BinNode::BinNode(const WordIx* words, size_t nWords, size_t szNode, size_t keySz) : nNodes(szNode) {
    newNode(words, keySz, 0, nWords);
}

void BinNode::newNode(const WordIx* words, size_t keySz, size_t i, size_t j, size_t nodeIx) {
    assert(i != j);

    size_t m = i + (j - i) / 2;
    const WordIx& word = words[m];
    char* key = getKey(nodeIx, keySz);
    for (size_t k = 0; k < keySz; k++) {
        key[k] = tolower(word.str[k]);
    }
    *getVal(nodeIx, keySz) = word.val;
    if (m - i >= 1) {
        newNode(words, keySz, i, m, getKid(nodeIx, 0));
    } else if (getKid(nodeIx, 0) < nNodes) {
        *getVal(getKid(nodeIx, 0), keySz) = NOT_FOUND;
    }
    if (j - (m + 1) >= 1) {
        newNode(words, keySz, m+1, j, getKid(nodeIx, 1));
    } else if (getKid(nodeIx, 1) < nNodes) {
        *getVal(getKid(nodeIx, 1), keySz) = NOT_FOUND;
    }
}

char* BinNode::getKey(size_t i, size_t keySz) {
    return keyVal + i * keySz;
}

uint32_t* BinNode::getVal(size_t i, size_t keySz) {
    return (uint32_t*)(keyVal + nNodes * keySz + sizeof(uint32_t) * i);
}

uint32_t BinNode::get(std::string& testKey, size_t nodeIx) {
    size_t keySz = testKey.size();
    uint32_t val = *getVal(nodeIx, keySz);
    if (val == NOT_FOUND) return NOT_FOUND;

    int cmp = memcmp(testKey.data(), getKey(nodeIx, keySz), keySz);
    if (0 == cmp) {
        return *getVal(nodeIx, keySz);
    }
    size_t kid = getKid(nodeIx, cmp > 0);
    return kid < nNodes ? get(testKey, kid) : NOT_FOUND;
}

size_t BinNode::getKid(size_t i, size_t which) {
    return i * 2 + 1 + which;
}

void BinNode::dump(uint32_t* wordPos, size_t keySz, size_t nodeIx) {
    if (nodeIx >= nNodes) return;
    uint32_t val = *getVal(nodeIx, keySz);
    if (val == 0) return;
    char* key = getKey(nodeIx, keySz);
    for (size_t i = 0; i < keySz; i++) {
        std::cerr << key[i];
    }
    uint32_t nVal = wordPos[val];
    for (size_t i = 1; i <= nVal; i++) {
        std::cerr << ' ' << wordPos[val + i] << ' ';
    }
    dump(wordPos, keySz, getKid(nodeIx, 0));
    dump(wordPos, keySz, getKid(nodeIx, 1));
};

int main()
{
    // valid commands and pre-programmed responses.
    const std::string CMD_END = "end";
    const std::string CMD_LOAD = "load";
    const std::string CMD_LOCATE = "locate";
    const std::string CMD_NEW = "new";
    const std::string ERROR_INVALID = "ERROR: Invalid command\n";
    const std::string MSG_NOTFOUND = "No matching entry\n";

    // Words stores the database of words.
    Words* curWords = NULL;
    bool done = false;
    while (!done) {
        std::cout << ">";

        std::string bfr;
        std::getline(std::cin, bfr);

        std::stringstream ss(bfr);
        std::string cmdName;
        ss >> cmdName;
        // lowercase the command since we treat commands case-insensitive
        for (size_t i = 0; i < cmdName.size(); i++) {
            cmdName[i] = tolower(cmdName[i]);
        }

        // parse the given command and make sure that it's in a valid form
        // types of errors:
        // incorrect command name, too few arguments, too many arguments,
        // illegal word
        // command types: load, locate, new, end, error
        enum Command { Load, Locate, New, End, Error };
        Command cmd = Error;
        std::string word;
        std::string fileName;
        int n;
        if (0 == CMD_LOAD.compare(cmdName)) {
            // load <fileName>
            // verify that a fileName was given, no other arguments were given,
            // and the file exists.
            if (ss.good()) {
                ss >> fileName;
                if (ss.eof() && !ss.fail()) {
                    // verify the file name
                    std::ifstream file(fileName.c_str());
                    cmd = file.good() ? Load : cmd;
                }
            }
        } else if (0 == CMD_LOCATE.compare(cmdName)) {
            // locate <word> <n>
            // verify that <word> and <n> were given, no other arguments were
            // given, n > 0, and the word given is a legal word.
            if (ss.good()) {
                ss >> word;
                if (ss.good()) {
                    ss >> n;
                    if (n > 0 && ss.eof() && !ss.fail()) {
                        for (size_t i = 0; i < word.size(); i++) {
                            word[i] = tolower(word[i]);
                        }
                        if (word.size() == findChar(word.data(), 0, word.size(), legalChar<true>)) {
                            cmd = Locate;
                        }
                    }
                }
            }
        } else if (0 == CMD_NEW.compare(cmdName)) {
            // new
            // verify no arguments were given
            cmd = ss.eof() && !ss.fail() ? New : cmd;
        } else if (0 == CMD_END.compare(cmdName)) {
            // end
            // verify no arguments were given
            cmd = ss.eof() && !ss.fail() ? End : cmd;
        }

        // act on the given command
        switch (cmd) {
            case Load:
                // delete the database which clears up memory. (using deletion
                // to be more RAII). Create a new one from the given file.
                if (curWords != NULL) {
                    delete curWords;
                }
                curWords = new Words(fileName);
                break;
            case Locate:
                // Locate the word in Words.
                // Search Words if it exists. If the word if found, print where
                // if not, print that it wasn't found.
                {
                    int wordN = 0;
                    if (curWords != NULL) {
                        wordN = curWords->locate(word, n);
                    }
                    if (wordN != Words::NOT_FOUND) {
                        std::cout << wordN << std::endl;
                    } else {
                        std::cout << MSG_NOTFOUND;
                    }
                    break;
                }
            case New:
                // This command resets the word list to the original (empty) state.
                // Delete Words if it exists which frees up the memory.
                if (curWords != NULL) {
                    delete curWords;
                    curWords = NULL;
                }
                break;
            case End:
                // This command terminates the program.
                done = true;
                break;
            case Error:
                // Some error state has been encountered.
                std::cout << ERROR_INVALID;
                break;
            default:
                // Should always hit an error state or one of the other states.
                std::cerr << "impossible state?\n";
                assert(0);
        }
    }

    if (curWords != NULL) {
        delete curWords;
    }

    return 0;
}
