#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <regex>
#include <sstream>
#include <string_view>

using namespace std;
class SuggestItem {
public:
    string term = "";
    int distance = 0;
    int64_t count = 0;

    bool operator==(const SuggestItem& other) const {
        return term == other.term;
    }

    bool operator<(const SuggestItem& other) const {
        return distance < other.distance || (distance == other.distance && count > other.count);
    }
};
enum class Verbosity
{
    /// <summary>Top suggestion with the highest term frequency of the suggestions of smallest edit distance found.</summary>
    Top,
    /// <summary>All suggestions of smallest edit distance found, suggestions ordered by term frequency.</summary>
    Closest,
    /// <summary>All suggestions within maxEditDistance, suggestions ordered by edit distance
    /// , then by term frequency (slower, no early termination).</summary>
    All
};

class LinSpell {
public:
    unordered_map<string, long> dictionaryLinear;

    vector<string> ParseWords(const string& text) {
        regex word_regex(R"(['’\w-[_]]+)");
        auto words_begin = sregex_iterator(text.begin(), text.end(), word_regex);
        auto words_end = sregex_iterator();

        vector<string> matches;
        for (sregex_iterator i = words_begin; i != words_end; ++i) {
            matches.push_back(i->str());
        }
        return matches;
    }

    bool LoadDictionary(const string& corpus, const string& language, int termIndex, int countIndex) {
        ifstream file(corpus);
        if (!file.is_open()) return false;

        string line;
        while (getline(file, line)) {
            istringstream lineStream(line);
            vector<string> lineParts((istream_iterator<string>(lineStream)), istream_iterator<string>());
            if (lineParts.size() >= 2) {
                string key = lineParts[termIndex];
                int64_t count;
                if (istringstream(lineParts[countIndex]) >> count) {
                    dictionaryLinear[language + key] = min(INT64_MAX, count);
                }
            }
        }
        return true;
    }
    
    bool CreateDictionary(const string& corpus, const string& language) {
        ifstream file(corpus);
        if (!file.is_open()) return false;

        string line;
        while (getline(file, line)) {
            for (const string& key : ParseWords(line)) {
                auto it = dictionaryLinear.find(language + key);
                if (it != dictionaryLinear.end()) {
                    if (it->second < INT64_MAX) {
                        it->second += 1;
                    }
                }
                else {
                    dictionaryLinear[language + key] = 1;
                }
            }
        }
        return true;
    }

    vector<SuggestItem> LookupLinear(const string& input, Verbosity verbose , const string& language, int editDistanceMax) {
        vector<SuggestItem> suggestions;

        int editDistanceMax2 = editDistanceMax;

        auto it = dictionaryLinear.find(language + input);
        if (verbose != Verbosity::All && it != dictionaryLinear.end()) {
            SuggestItem si;
            si.term = input;
            si.count = it->second;
            si.distance = 0;
            suggestions.push_back(si);

            return suggestions;
        }

        for (const auto& kv : dictionaryLinear) {
            if (abs(static_cast<int>(kv.first.length()) - static_cast<int>(input.length())) > editDistanceMax2) continue;

            if (verbose == Verbosity::Top && !suggestions.empty() && suggestions[0].distance == 1 && kv.second <= suggestions[0].count) continue;

            int distance = DamerauLevenshteinDistance(input, kv.first, editDistanceMax2);

            if (distance >= 0 && distance <= editDistanceMax) {
                if (verbose != Verbosity::All  && !suggestions.empty() && distance > suggestions[0].distance) continue;

                if (verbose != Verbosity::All) editDistanceMax2 = distance;

                if (verbose != Verbosity::All && !suggestions.empty() && suggestions[0].distance > distance) {
                    suggestions.clear();
                }
                SuggestItem si;
                si.term = kv.first;
                si.count = kv.second;
                si.distance = distance;
                suggestions.push_back(si);
            }

        }

        sort(suggestions.begin(), suggestions.end());
        if (verbose == Verbosity::Top && suggestions.size() > 1) {
            suggestions.resize(1);
        }

        return suggestions;
    }
    int DamerauLevenshteinDistance(string_view string1, string_view string2, int maxDistance) {
        if (string1.empty()) return string2.length();
        if (string2.empty()) return string1.length();

        if (string1.length() > string2.length()) {
            swap(string1, string2);
        }

        int sLen = string1.length();
        int tLen = string2.length();

        while ((sLen > 0) && (string1[sLen - 1] == string2[tLen - 1])) {
            sLen--;
            tLen--;
        }

        int start = 0;
        if ((string1[0] == string2[0]) || (sLen == 0)) {
            while ((start < sLen) && (string1[start] == string2[start])) start++;
            sLen -= start;
            tLen -= start;

            if (sLen == 0) return tLen;

            string2 = string2.substr(start, tLen);
        }

        int lenDiff = tLen - sLen;
        if ((maxDistance < 0) || (maxDistance > tLen)) {
            maxDistance = tLen;
        }
        else if (lenDiff > maxDistance) return -1;

        vector<int> v0(tLen);
        vector<int> v2(tLen);

        int j;
        for (j = 0; j < maxDistance; j++) v0[j] = j + 1;
        for (; j < tLen; j++) v0[j] = maxDistance + 1;

        int jStartOffset = maxDistance - (tLen - sLen);
        bool haveMax = maxDistance < tLen;
        int jStart = 0;
        int jEnd = maxDistance;
        char sChar = string1[0];
        int current = 0;

        for (int i = 0; i < sLen; i++) {
            char prevsChar = sChar;
            sChar = string1[start + i];
            char tChar = string2[0];
            int left = i;
            current = left + 1;
            int nextTransCost = 0;
            jStart += (i > jStartOffset) ? 1 : 0;
            jEnd += (jEnd < tLen) ? 1 : 0;

            for (j = jStart; j < jEnd; j++) {
                int above = current;
                int thisTransCost = nextTransCost;
                nextTransCost = v2[j];
                v2[j] = current = left;
                left = v0[j];
                char prevtChar = tChar;
                tChar = string2[j];

                if (sChar != tChar) {
                    if (left < current) current = left;
                    if (above < current) current = above;
                    current++;
                    if ((i != 0) && (j != 0) && (sChar == prevtChar) && (prevsChar == tChar)) {
                        thisTransCost++;
                        if (thisTransCost < current) current = thisTransCost;
                    }
                }
                v0[j] = current;
            }
            if (haveMax && (v0[i + lenDiff] > maxDistance)) return -1;
        }
        return (current <= maxDistance) ? current : -1;

    };

};



