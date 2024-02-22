// Решите загадку: Сколько чисел от 1 до 1000 содержат как минимум одну цифру 3?
// Напишите ответ здесь:

// Закомитьте изменения и отправьте их в свой репозиторий.

#include <algorithm>
#include <iostream>
#include <set>
#include <map>
#include <string>
#include <utility>
#include <vector>
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
    int      id;
    double   relevance;
};

class SearchServer {
public:
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }
    
    void AddDocument(int document_id, const string& document_content) {
        const vector<string> words = SplitIntoWordsNoStop(document_content);
        const set<string> words_set(words.begin(), words.end());
        
        int words_size = words.size();
        for(string word : words_set){
           
            double tf = ((double) count(words.begin(),words.end(), word)) / words_size ;
            word_to_documents_freq_[word][document_id] = tf;
        }
        ++document_count_;
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        const set<string> query_words = ParseQuery(raw_query);
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
    struct DocumentContent {
        int id = 0;
        vector<string> words;
    };

    map<string, map<int, double>> word_to_documents_freq_;
    int                           document_count_ = 0;
    set<string>                   stop_words_;

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

    set<string> ParseQuery(const string& text) const {
        set<string> query_words;
        for (const string& word : SplitIntoWordsNoStop(text)) {
            query_words.insert(word);
        }
        return query_words;
    }

    double idf(const string& word) const {
        return log((double) document_count_ / word_to_documents_freq_.at(word).size());
    }

    vector<Document> FindAllDocuments(const set<string>& query_words) const {
        map<int, double> document_to_relevance;

        set<string> query_minus_words;
        set<string> query_plus_words;
        const char minus = '-';

        for(string w : query_words) {
            if(w[0] ==  minus) {
                query_minus_words.insert(w.substr(1));
            } else {
                query_plus_words.insert(w);
            }
        }

        double idf;
        for(const string& query_word : query_plus_words) { 
            if(word_to_documents_freq_.count(query_word) > 0) {
                idf =
                    log((double) document_count_ / word_to_documents_freq_.at(query_word).size());
                for(const auto& [doc_id, tf] : word_to_documents_freq_.at(query_word)) { 
                    document_to_relevance[doc_id] += (double) idf * tf;
                }
            }
        }

        for(const string& minus_word : query_minus_words) {
            if(word_to_documents_freq_.count(minus_word) > 0){
                for(auto& [doc_id , tf] : word_to_documents_freq_.at(minus_word)) {
                    document_to_relevance.erase(doc_id);
                }
            }
        }

        vector<Document>  result;
        for(auto& [doc_id, relevance] : document_to_relevance) {
            result.push_back({doc_id, relevance});
        }

        return result;
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

    const string raw_query = ReadLine();
    for (const auto& [document_id, relevance] : search_server.FindTopDocuments(raw_query)) {
        cout << "{ document_id = "s << document_id << ", "
             << "relevance = "s << relevance << " }"s << endl;
    }
}
