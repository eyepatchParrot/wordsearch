//
// File: wl.h
// 
//  Description: Load a text file into a datastructure composed of many BSTs.
//  Student Name: Peter Van Sandt
//  UW Campus ID: 9071889357
//  enamil: vansandt@wisc.edu

/**
 * @brief takes a character and returns whether or not it is legal under some
 * definition.
 *
 * @param char character to test
 *
 * @return is the character legal?
 */
typedef bool (*charPred)(char);

/**
 * @brief is c in [A-z0-9']?
 *
 * @tparam complement reverse result complement == true means illegal,
 * complement == false means legal
 * @param c character to process
 *
 * @return is c in [A-z0-9']?
 */
template <bool complement>
bool legalChar(char c);

/**
 * @brief Find index of first character in str for which fn(c) == true.
 *
 * @param str string to search
 * @param start index to start searching in the string
 * @param end index to stop searching in the string
 * @param fn function to test legality of characters
 *
 * @return index of first character in str for which fn(c) == true.
 */
size_t findChar(const char* str, size_t start, size_t end, charPred fn);

/**
 * @brief Set wordStart and wordEnd such that wordStart is at the first
 * character that is legal, and wordEnd is the first adjacent character that is
 * illegal.
 *
 * @param str string to search
 * @param strStart where to start searching
 * @param strEnd index to finish searching
 * @param wordStart output index of the start of the word
 * @param wordEnd output index of the end of the word
 *
 * @return if this is the last legal word in the file.
 */
bool findWord(const char* str, size_t strStart, size_t strEnd, size_t& wordStart, size_t& wordEnd);

/**
 * @brief Creates an object that holds a map to the file while it's needed. 
 */
class FileMap {
    public:
        /**
         * @brief Constructor that loads a given file. Expects a valid file.
         *
         * @param fileName The name of the file to be loaded.
         */
        FileMap(std::string& fileName);

        /**
         * @brief Release the memory map to the file
         */
        ~FileMap();

        /**
         * @brief Returns a pointer to the file.
         *
         * @return char* to the file
         */
        char* getStr() {
            return file;
        };

        /**
         * @brief Returns the size of the file
         *
         * @return size of the file in bytes
         */
        size_t getSz() {
            return sz;
        };

    private:
        /**
         * @brief The pointer to the file given by mmap. NULL if no map
         * successful.
         */
        char* file;
        /**
         * @brief The file descriptor of the file.
         */
        int fd;
        /**
         * @brief The size of the file in bytes.
         */
        size_t sz;
};

/**
 * @brief Pointer to the start of a word combined with a linked integer value.
 */
struct WordIx {
    /**
     * @brief Create with everything blank.
     */
    WordIx() : str(NULL), val(0) { };
    /**
     * @brief Initialize str, val with provided values.
     *
     * @param newStr character pointer to beginning of word
     * @param newVal index of the word in the file
     */
    WordIx(char* newStr, uint32_t newVal) : str(newStr), val(newVal) { };
    /**
     * @brief character pointer to beginning of word
     */
    char* str;
    /**
     * @brief index of the word in the file
     */
    uint32_t val;

    /**
     * @brief Compare with another Word. Negative if less, 0 if equal, positive
     * is greater.
     *
     * @param r the other word
     * @param len the length of the word
     *
     * @return Relative comparison.
     */
    int compare(const WordIx* r, size_t len) {
        return compare(this, r, len);
    };

    /**
     * @brief Special compare format for qsort. (case insentive)
     *
     * @param l word on left
     * @param r word on right
     * @param len length of word
     *
     * @return negative if less than, 0 if equal, positive if greater
     */
    static int qsortCompare(const void* l, const void* r, void* len) {
        return compare((const WordIx*)l, (const WordIx*)r, (size_t)len);
    };

    /**
     * @brief Static method of comparison. (case insensitive)
     *
     * @param l word on left
     * @param r word on right
     * @param len length of the word
     *
     * @return negative if less than, 0 if equal, positive if greater
     */
    static int compare(const WordIx* l, const WordIx* r, size_t len) {
        return memCompare(l->str, r->str, len);
    };

    /**
     * @brief Do case insensitive comparsion of c-strings.
     *
     * @param l string on left
     * @param r string on right
     * @param sz length of string
     *
     * @return negative if less than, 0 if equal, positive if greater
     */
    static int memCompare(const char* l, const char* r, const size_t sz);
};

/**
 * @brief Balanced binary tree stored in an array composed of words of
 * the same length, generated from a sorted array of words.
 */
class BinNode {
    public:
        /**
         * @brief The words was not found.
         */
        const static int NOT_FOUND = 0;

        /**
         * @brief Allocate memory for a new node. New can't be used since the
         * size of a binary node varies.
         *
         * @param words sorted array of words to initialize from
         * @param nWords number of words in the array
         * @param keySz length of a key in the array
         *
         * @return a pointer to the new node.
         */
        static BinNode* newBinNode(const WordIx* words, size_t nWords, size_t keySz);
        /**
         * @brief Destroy this node from memory.
         */
        ~BinNode() { void* mem = this; ::operator delete(mem); };
        
        /**
         * @brief recurseively locate a key in this set of nodes.
         *
         * @param testKey the key to look for
         * @param i index of the node to start at
         *
         * @return the entry index of this pair
         */
        uint32_t get(std::string& testKey, size_t i = 0);
        /**
         * @brief Debug method to output the keys and values of the nodes of this tree
         *
         * @param wordPos the vector of entries to read from
         * @param keySz the size of the keys of this tree
         * @param nodeIx the index from which to recurse
         */
        void dump(uint32_t* wordPos, size_t keySz, size_t nodeIx = 0);

    private:
        /**
         * @brief The number of nodes in this tree.
         */
        uint32_t nNodes;
        /**
         * @brief a binary search tree of keys followed by an array of values. Every
         * sub-node has a key and a value. Keys are defined why the list that
         * this is resident in, and values are always 4 bytes.  In the case
         * where this is a rep node, then the value will index into the node
         * where that entry's value list starts.
         */
        char keyVal[0];

        /**
         * @brief Construct the binary tree.
         *
         * @param words Sorted list of words from which to make the BST.
         * @param nWords Number of words in the list
         * @param szNode Number of nodes in the BST
         * @param keySz Length of the key
         */
        BinNode(const WordIx* words, size_t nWords, size_t szNode, size_t keySz);
        /**
         * @brief Recursively initialize a new node.
         *
         * @param words Sorted list of words from which to make the BST.
         * @param keySz Length of the key
         * @param i Left end of the list to recurse on.
         * @param j Right end of the list to recurse on.
         * @param nodeIx Index of the node to recurse on.
         */
        void newNode(const WordIx* words, size_t keySz, size_t i, size_t j,
                size_t nodeIx = 0);
        /**
         * @brief Returns a pointer to the key for node i.
         *
         * @param i the node to return
         * @param keySz the size of the key
         *
         * @return pointer to a key
         */
        char* getKey(size_t i, size_t keySz);
        /**
         * @brief Returns a pointer to the value for node i.
         *
         * @param i node to return
         * @param keySz size of the key
         *
         * @return pointer to a value
         */
        uint32_t* getVal(size_t i, size_t keySz);
        /**
         * @brief Returns the index value of which child of i.
         *
         * @param i parent node
         * @param which select child
         *
         * @return index of a node
         */
        size_t getKid(size_t i, size_t which);
};
