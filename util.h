#ifndef UTIL_H
#define UTIL_H

template <bool complement>
bool legalChar(char c) {
    // use complement to reverse the return value
    return (isalnum(c) || c == '\'') != complement;
}

char* findChar(const char* str, const char* end, charPred fn) {
    // find the first character that matches the predicate
    while (start < end && !fn(str[start])) {
        start++;
    }
    return start;
}

bool findWord(const char* str, size_t len, char*& wordStart, char*& wordEnd) {
    // look for the first character that matches the predicate
    char* end = str + len;
    wordStart = findChar(str, end, legalChar<false>);

    // finish the word by the first character that doesn't match the predicate
    wordEnd = findChar(wordStart, end, legalChar<true>);

    // return if this is the last word in the string.
    return wordStart < end;
}

#endif
