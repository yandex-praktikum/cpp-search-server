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
         document_count_ += 1; // добавили количество документов
        const vector<string> words = SplitIntoWordsNoStop(document);
        tf_count_doc_words_[document_id] = words.size(); //посчитали количество слов в документе
        for (const string& word : words) { //гоняет по словам
            tf_count_words_indoc_[word][document_id] += 1;
                       documents_[word].insert(document_id);
        }
        //documents_.push_back({document_id, words});
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        const QueryWords query_words = ParseQuery(raw_query);
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
    map<int, int> tf_count_doc_words_; // ключ это айди документа, значение это количество слов в документе
    map<string, map<int , int>> tf_count_words_indoc_; // количество слов запросчиков в документе
    int document_count_ = 0;
  /*  struct DocumentContent {
        int id = 0;
        vector<string> words;
    };*/
    
    struct QueryWords {
        set<string> plus_word;
        set<string> minus_word;
    };
    
    map<string, set<int>> documents_;
    

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

    QueryWords ParseQuery(const string& text) const {
        QueryWords query_words;
        for (const string& word : SplitIntoWordsNoStop(text)) {
            if (word[0] == '-') {
                query_words.minus_word.insert(word.substr(1, word.size()));
            } else {
                query_words.plus_word.insert(word);
            }
        }
        return query_words;
    }

    vector<Document> FindAllDocuments(const QueryWords& query_words) const {
        vector<Document> matched_documents;
            const map<int, double> relevance = MatchDocument(documents_,
                                                             query_words,
                                                             tf_count_doc_words_,
                                                             tf_count_words_indoc_,
                                                             document_count_);
        for (const auto& rel : relevance){
            //if (rel.second > 0) {
                matched_documents.push_back({rel.first, rel.second});
            //}
        }
        return matched_documents;
    }

    static map<int, double> MatchDocument(const map<string, set<int>>& content, //сюда походу доки отправляем
                                          const QueryWords& query_words, 
                                          const map<int, int>& count_docs,
                                          const map<string, map<int,int>>& words_indoc,
                                          const int& document_count_) { // тут походу запрос + -
      
        map<int, double> rrr; // возвращаем по итогу
        if (query_words.plus_word.empty()) {
            return rrr;
        }
        for (const string& plus_s : query_words.plus_word) { //хапаем слово из запроса и кружим его
            if (content.count(plus_s) == 0) { //убедились что хапанули не пустоту
                continue;
            } else {
                for (const int& id_doc : content.at(plus_s)) {  // загружаем ид относительно слова запроса
                rrr[id_doc] += static_cast<double>(words_indoc.at(plus_s).at(id_doc)) / count_docs.at(id_doc) * log(static_cast<double>(document_count_)/content.at(plus_s).size()); //тут посчитаем TFIDF
                    if (rrr[id_doc] <= 0) {
                        rrr.erase(id_doc);
                    }
                }
            }
        }
        if (!query_words.minus_word.empty()) { //ну тут минус слова обнуляем по релевантности
            for (const string& minus_s : query_words.minus_word) {
                if (content.count(minus_s) == 0) {
                    continue;
                } else {
                    for (const int& id_doc : content.at(minus_s)) {
                        rrr.erase(id_doc);
                    }
                }
            }
        }
        return rrr;
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
    //    if (relevance != 0) {
        cout << "{ document_id = "s << document_id << ", "
             << "relevance = "s << relevance << " }"s << endl;
        //}
    }
}
