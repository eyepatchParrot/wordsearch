#include "words.h"
#include "util.h"
#include "file.h"

Words::Words(std::string& fileName) : headsByLen(NULL), wordPosByLen(NULL),
    maxLen(0) {
    /*
     * 1. Build a vector of words.
     * 2. Create an array of string pointers to index the words.
     * 3. Sort the array.
     * 4. Create a vector of vectors of string pointers to index the words
     *    by occurrence.
     * 5. For each occurrence vector, construct a balanced binary tree.
     */

    File f(fileName);
    if (0 == f.getSize()) return;

    // read in the file
    char* s = f.getPtr();
    size_t sz = f.getSize();
    while (findWord(s, 

    // map to the given file
    FileMap file(fileName);

    if (file.getSz() == 0) {
        return;
    }

    // 4 byte index for every word
    // build len-based trees for every unique word

    // Partition words by length
    maxLen = findMaxWordLength(file.getStr(), file.getSz()) + 1;
    size_t* nWordsByLen = new size_t[maxLen];
    countNumWordsInEachLength(file.getStr(), file.getSz(), maxLen, nWordsByLen);
    wordPosByLen = new uint32_t*[maxLen];
    WordIx** wordsByLen = new WordIx*[maxLen];

    for (size_t len = 0; len < maxLen; len++) {
        wordsByLen[len] = nWordsByLen[len] > 0 ?
            new WordIx[nWordsByLen[len]] : NULL;
    }
    fillWordsByLen(file.getStr(), file.getSz(), maxLen, wordsByLen);

    // make space to index trees by length
    headsByLen = new BinNode*[maxLen];
    memset(headsByLen, 0, maxLen * sizeof(BinNode*));

    // create sorted lists from the partitions and construct the trees
    for (size_t len = 0; len < maxLen; len++) {
        size_t nWords = nWordsByLen[len];
        wordPosByLen[len] = NULL;
        if (nWordsByLen[len] > 0) {
            // create sorted list of unique words
            WordIx* words = wordsByLen[len];
            qsort_r((void*)words, nWords, sizeof(WordIx), WordIx::qsortCompare, (void*)len);
            size_t nUniqueWords = countUniqueWords(words, nWords, len);
            wordPosByLen[len] = new uint32_t[nWords + nUniqueWords + 1];
            WordIx* uniqueWords = new WordIx[nUniqueWords];

            // fill the unique words and add entries for other occurrences
            fillUniqueWords(wordsByLen[len], nWords, uniqueWords, wordPosByLen[len], len);

            // populate the binary tree at this key length
            headsByLen[len] = BinNode::newBinNode(uniqueWords, nUniqueWords, len);

            delete[] uniqueWords;
            delete[] wordsByLen[len];
        }
    }
    delete[] wordsByLen;
    delete[] nWordsByLen;
}

Words::~Words() {
    if (headsByLen != NULL) {
        for (size_t len = 0; len < maxLen; len++) {
            if (headsByLen[len] != NULL) {
                headsByLen[len]->~BinNode();
            }
        }
        delete[] headsByLen;
    }
    if (wordPosByLen != NULL) {
        for (size_t len = 0; len < maxLen; len++) {
            if (wordPosByLen[len] != NULL) {
                delete[] wordPosByLen[len];
            }
        }
        delete[] wordPosByLen;
    }
}

int Words::locate(std::string& word, uint32_t n) {
    assert(n > 0);
    size_t len = word.size();
    if (headsByLen != NULL && len < maxLen 
            && headsByLen[len] != NULL) {
        uint32_t entry = headsByLen[len]->get(word);
        if (n <= wordPosByLen[len][entry]) {
            return wordPosByLen[len][entry + n];
        }
    }

    return NOT_FOUND;
}

size_t Words::findMaxWordLength(char* str, size_t sz) {
    size_t wordStart = 0;
    size_t wordEnd = 0;
    size_t maxLen = 0;
    while (findWord(str, wordStart, sz, wordStart, wordEnd)) {
        maxLen = std::max(wordEnd - wordStart, maxLen);

        wordStart = wordEnd;
    }
    return maxLen;
}

void Words::countNumWordsInEachLength(char* str, size_t sz, size_t maxLen, size_t* nWordsByLen) {
    memset(nWordsByLen, 0, sizeof(size_t) * maxLen);
    size_t wordStart = 0;
    size_t wordEnd = 0;
    while (findWord(str, wordStart, sz, wordStart, wordEnd)) {
        size_t len = wordEnd - wordStart;
        assert(len < maxLen);
        nWordsByLen[len]++;
        wordStart = wordEnd;
    }
}

void Words::fillWordsByLen(char* str, size_t sz, size_t maxLen, WordIx** wordsByLen) {
    size_t wordStart = 0;
    size_t wordEnd = 0;
    uint32_t allIx = 1;

    size_t* wordIx = new size_t[maxLen];
    memset(wordIx, 0, sizeof(size_t) * maxLen);
    while (findWord(str, wordStart, sz, wordStart, wordEnd)) {
        size_t len = wordEnd - wordStart;
        size_t i = wordIx[len];
        wordsByLen[len][i] = WordIx(str + wordStart, allIx);
        allIx++;
        wordIx[len]++;

        wordStart = wordEnd;
    }
    delete[] wordIx;
}

size_t Words::countUniqueWords(WordIx* words, size_t nWords, size_t len) {
    if (nWords == 0) {
        return 0;
    }
    size_t nUniqueWords = 1;
    for (size_t i = 1; i < nWords; i++) {
        if (0 != words[i].compare(&words[i-1], len)) {
            nUniqueWords++;
        }
    }
    return nUniqueWords;
}

// entries include # of word positions and then n word positions  
void Words::fillUniqueWords(WordIx* words, size_t nWords, WordIx* uniqueWords, uint32_t* wordPos, size_t len) {
    wordPos[0] = 0;
    if (nWords == 0) {
        return;
    }
    size_t wordPosSz = 1;
    size_t uniqueIx = 0;
    size_t seqLen = 1;
    for (size_t i = 0; i < nWords; i++) {
        if (i == 0 || 0 != words[i].compare(&words[i-1], len)) {
            if (i != 0) {
                // finalize the last entry
                wordPos[uniqueWords[uniqueIx - 1].val] = seqLen;
                seqLen = 1;
            }
            // ready the new entry
            uniqueWords[uniqueIx++] = WordIx(words[i].str, wordPosSz);
            wordPosSz++;
        } else {
            seqLen++;
        }
        wordPos[wordPosSz++] = words[i].val;
    }
    wordPos[uniqueWords[uniqueIx - 1].val] = seqLen;
}
