#include <algorithm>
#include <iostream>
#include <map>
#include <math.h>
#include <set>
#include <string>
#include <vector>
#include <utility>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;


struct Document
{
    int id;
    double relevance;
};

struct QueryWords
{
    set<string> minus_word;
    set<string> plus_word;
};

string ReadLine(){
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber(){
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text){
    vector<string> words;
    string word;
    for (const char c:text){
        if (c == ' '){
            if(!word.empty()){
                words.push_back(word);
                word.clear();
            } 
        } else {
            word +=c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }
    return words;
}

class SearchServer{
    
public:

    void SetStopWords(const string& text){
        for (const string& word:SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document){
        const vector<string> words = SplitIntoWordsNoStop(document);
        int count_words_documents = words.size();
        double tf_document = 1./count_words_documents;
        for (const auto& word: words){
            word_to_document_freqs_[word][document_id]+=tf_document;    
        }
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        const QueryWords query_words = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query_words);
        sort(matched_documents.begin(), matched_documents.end(),
                [](const Document& left, const Document& right){
                    return left.relevance > right.relevance;
                }
            );
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }

        return matched_documents;
    }
    void SetCountDocuments(int count_documents){
        count_documents_ = count_documents;
    }
private:
    map<string, map<int, double>> word_to_document_freqs_;
    set<string> stop_words_;
    int count_documents_;
    
    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }
    
    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word: SplitIntoWords(text)){
            if (!IsStopWord(word)){
                words.push_back(word);
            }
        }
        return words;
    }

    QueryWords ParseQuery(const string& text) const {
        QueryWords query_words;
        for (const string& word:SplitIntoWordsNoStop(text)){
            if (word[0] != '-') {
                query_words.plus_word.insert(word);
            }
            else {
                query_words.minus_word.insert(word.substr(1));
            }
        }
        return query_words;
    }

    double CalculateIdf(const string& plus_word) const {
        return log(count_documents_ / static_cast<double>(word_to_document_freqs_.at(plus_word).size()));
    }

    vector<Document> FindAllDocuments(const QueryWords& query_words) const {
        map<int, double> document_to_relevance; 
        vector<Document> matched_documents;
        for (const string& plus_word : query_words.plus_word){
            if (word_to_document_freqs_.count(plus_word) != 0){
                double idf_word = CalculateIdf(plus_word);
                for(const auto& [id_document, tf_document] : word_to_document_freqs_.at(plus_word)) { 
                    document_to_relevance[id_document] += static_cast<double>(idf_word * tf_document);
                }              
            }           
        }
        for (const string& no_word: query_words.minus_word){
            if (word_to_document_freqs_.count(no_word) != 0){
                for(const auto& [id_documents,_]:word_to_document_freqs_.at(no_word)) {
                    document_to_relevance.erase(id_documents);  
                }
            }
        }
        for (const auto& [id_documents, relevance_document]:document_to_relevance){
            matched_documents.push_back({id_documents, relevance_document});
        }
        return matched_documents;
    }
};

SearchServer CreateSearchServer(){
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());
    const int document_count = ReadLineWithNumber();
    search_server.SetCountDocuments(document_count);
    for (int document_id = 0; document_id < document_count; ++document_id){
        search_server.AddDocument(document_id, ReadLine());
    }
    return search_server;
}

int main()
{
    const SearchServer search_server = CreateSearchServer();
    const string query = ReadLine();
    search_server.FindTopDocuments(query);
    for (const auto& [document_id, relevance]:search_server.FindTopDocuments(query)){
        cout << "{ document_id = "s << document_id << ", relevance = "s << relevance << " }"s << endl;
    };
    return 0;
}
