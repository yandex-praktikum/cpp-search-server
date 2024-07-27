#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <cmath>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}

struct Document {
    int id;
    double relevance;
};

class SearchServer {
public:
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document) {
        const vector<string> words = SplitIntoWordsNoStop(document);
        document_count_ += 1;
        int N = words.size();
        for (string word: words) {
            word_to_document_TF_[word][document_id] += 1.0 / N;
        }
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        const vector<string> query_words = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query_words);

        sort(matched_documents.begin(), matched_documents.end(),
             [](const Document& lhs, const Document& rhs) {
                 return lhs.relevance > rhs.relevance;
             });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

private:

    map<string, map<int, double>> word_to_document_TF_;
    int document_count_ = 0;
    set<string> stop_words_;

    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }

    vector<string> ParseQuery(const string& text) const {
        vector<string> query_words;
        for (const string& word : SplitIntoWordsNoStop(text)) {
            query_words.push_back(word);
        }
        return query_words;
    }
    
    map <string, double> IDF(const vector<string>& text) const {
        map<string, double> map_idf;
        for (auto& word: text) {
            if (word_to_document_TF_.count(word) > 0) {
                int docs_count = word_to_document_TF_.at(word).size();
                map_idf[word] = log(static_cast<double>(document_count_) / docs_count);
            }
        }
        return map_idf;
    }
    
    vector<Document> FindAllDocuments(const vector<string>& query_words) const {
        map<int, double> document_to_relevance;
        vector<Document> result;
        map<string, double> document_IDF = IDF(query_words);
        set<string> minus_words = MinusSlova(query_words);
        vector<int> id_minus_words = IdMinusSlova(minus_words);
        for (auto& [word, IDF]: document_IDF) {
            if (minus_words.count(word) > 0) {continue;}
            for (auto& [id, TF]: word_to_document_TF_.at(word)) {
                    document_to_relevance[id] += (IDF * TF);
           }
        }
        for (int id : id_minus_words) {
            document_to_relevance.erase(id);
        }
        for (const auto& relevance : document_to_relevance) {
            result.push_back(Document{relevance.first, relevance.second});
        }
     return result;
    }
    
    vector<int> IdMinusSlova(set<string> text) const {
        vector<int> number;
        for (string word: text) {
            if (word_to_document_TF_.count(word) > 0) {
                for (auto& [id, reiting]: word_to_document_TF_.at(word)) {
                number.push_back(id);
            }
            }
        }
        return number;
    }
    
    set<string> MinusSlova(const vector<string>& query_words) const {
        set<string> minus_slova;
        for (string word: query_words) {
            if (word.at(0) == '-') {
                minus_slova.insert(word.substr(1));
            }
        }
        return minus_slova;
    }
};

SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());

    const int document_count = ReadLineWithNumber();
    for (int document_id = 0; document_id < document_count; ++document_id) {
        search_server.AddDocument(document_id, ReadLine());
    }

    return search_server;
}

int main() {
    const SearchServer search_server = CreateSearchServer();

    const string query = ReadLine();
    for (const auto& [document_id, relevance] : search_server.FindTopDocuments(query)) {
        cout << "{ document_id = "s << document_id << ", "
             << "relevance = "s << relevance << " }"s << endl;
    }
}
/*
is are was a an in the with near at
3
a colorful parrot with green wings and red tail is lost
a grey hound with black ears is found at the railway station
a white cat with long furry tail is found near the red square
white cat long tail -кфшдцфн
*/
