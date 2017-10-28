#ifndef WORDS_H
#define WORDS_H

/**
 * @brief Database that holds a list of binary trees indexed by key length.
 */
class Words {
    public:
        /**
         * @brief Definition for key not found.
         */
        const static int NOT_FOUND = BinNode::NOT_FOUND;

        /**
         * @brief load a file as a new words database
         *
         * @param filename file to load
         */
        Words(std::string& filename);
        /**
         * @brief destroy the database
         */
        ~Words();

        /**
         * @brief Return 0 if not found. Expects n > 0 since input should be
         * sanitized. If n and the length are in range, index by length and
         * repetition, and search the BST.
         *
         * @param word word to search for
         * @param n nth occurrence
         *
         * @return location of nth occurrence of word
         */
        int locate(std::string& word, uint32_t n);
        
    private:
        /**
         * @brief List of binary trees indexed by key length.
         */
        BinNode** headsByLen;
        /**
         * @brief List of entry arrays for bsts to link to.
         */
        uint32_t** wordPosByLen;
        /**
         * @brief the largest length key
         */
        uint32_t maxLen;

        /**
         * @brief Find the length of the largest word in str
         *
         * @param str string to read through
         * @param sz length of the string
         *
         * @return length of the word
         */
        size_t findMaxWordLength(char* str, size_t sz);
        /**
         * @brief Populate nWordsByLen with the number of words of each length.
         *
         * @param str string to read
         * @param sz size of string
         * @param maxLen maximum length word
         * @param nWordsByLen output array for number of words
         */
        void countNumWordsInEachLength(char* str, size_t sz, size_t maxLen,
                size_t* nWordsByLen);
        /**
         * @brief Fill a 2d array with words based on their length.
         *
         * @param str string to read
         * @param sz length of string
         * @param maxLen maximum length of word
         * @param wordsByLen output 2d array
         */
        void fillWordsByLen(char* str, size_t sz, size_t maxLen, WordIx** wordsByLen);
        /**
         * @brief Count the number of different words.
         *
         * @param words list of words
         * @param nWords number of words in the list
         * @param len length of each word
         *
         * @return number of unique words
         */
        size_t countUniqueWords(WordIx* words, size_t nWords, size_t len);
        /**
         * @brief Fill uniqueWords with each word at exactly once. Link the
         * words in unique words to entries in wordPos and fill wordPos with
         * the positions of each occurrence.
         *
         * @param words list of words
         * @param nWords number of words
         * @param uniqueWords list of unique words
         * @param wordPos list of entries to fill with occurrences
         * @param len length of word
         */
        void fillUniqueWords(WordIx* words, size_t nWords, WordIx* uniqueWords, uint32_t* wordPos, size_t len);
};

#endif // WORDS_H
