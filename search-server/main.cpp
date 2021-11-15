#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
  string s;
  getline(cin, s);
  return s;
}

int ReadLineWithNumber() {
  int result;
  cin >> result;
  ReadLine();
  return result;
}

vector<string> SplitIntoWords(const string &text) {
  vector<string> words;
  string word;
  for (const char symbol : text) {
    if (symbol == ' ') {
      if (!word.empty()) {
        words.push_back(word);
        word.clear();
      }
    } else {
      word += symbol;
    }
  }
  if (!word.empty()) {
    words.push_back(word);
  }

  return words;
}

struct Document {
  int    id;
  double relevance;
  int    rating;
};

enum class DocumentStatus {
  ACTUAL,
  IRRELEVANT,
  BANNED,
  REMOVED,
};

class SearchServer {
public:
  void SetStopWords(const string &text) {
    for (const string &word : SplitIntoWords(text)) {
      stop_words_.insert(word);
    }
  }

  void AddDocument(int document_id, const string &document,
                   DocumentStatus status, const vector<int> &ratings) {
    const vector<string> words = SplitIntoWordsNoStop(document);
    const double inv_word_count = 1.0 / words.size();
    for (const string &word : words) {
      word_to_document_freqs_[word][document_id] += inv_word_count;
    }
    documents_.emplace(document_id,
                       DocumentData{ComputeAverageRating(ratings), status});
  }

  template <typename Lambda>
  vector<Document> FindTopDocuments(const string &raw_query,
                                    Lambda lambda) const {
    const Query query = ParseQuery(raw_query);
    auto matched_documents = FindAllDocuments(query, lambda);
    sort(matched_documents.begin(), matched_documents.end(),
         [](const Document &lhs, const Document &rhs) {
           if (abs(lhs.relevance - rhs.relevance) < 1e-6) {
             return lhs.rating > rhs.rating;
           } else {
             return lhs.relevance > rhs.relevance;
           }
         });
    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
        matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }
    return matched_documents;
  }

  vector<Document> FindTopDocuments(const string &raw_query) const {
      return FindTopDocuments(raw_query, [](int document_id, DocumentStatus status_,
          int rating) { return status_ == DocumentStatus::ACTUAL; });
  }

  vector<Document> FindTopDocuments(const string &raw_query,
                                    DocumentStatus status) const {
    return FindTopDocuments(raw_query,
                            [status](int document_id, DocumentStatus status_,
                                     int rating) { return status_ == status; });
  }

  size_t GetDocumentCount() const { return documents_.size(); }

  tuple<vector<string>, DocumentStatus> MatchDocument(const string &raw_query,
                                                      int document_id) const {
    const Query query = ParseQuery(raw_query);
    vector<string> matched_words;
    for (const string &word : query.plus_words) {
      if (word_to_document_freqs_.count(word) == 0) {
        continue;
      }
      if (word_to_document_freqs_.at(word).count(document_id)) {
        matched_words.push_back(word);
      }
    }
    for (const string &word : query.minus_words) {
      if (word_to_document_freqs_.count(word) == 0) {
        continue;
      }
      if (word_to_document_freqs_.at(word).count(document_id)) {
        matched_words.clear();
        break;
      }
    }
    return tie(matched_words, documents_.at(document_id).status);
  }

private:
  struct DocumentData {
    int            rating;
    DocumentStatus status;
  };

  set<string>                   stop_words_;
  map<string, map<int, double>> word_to_document_freqs_;
  map<int, DocumentData>        documents_;

  vector<string> SplitIntoWordsNoStop(const string &text) const {
    vector<string> words;
    for (const string &word : SplitIntoWords(text)) {
      if (stop_words_.count(word) == 0) {
        words.push_back(word);
      }
    }
    return words;
  }

  static int ComputeAverageRating(const vector<int> &ratings) {
    if (ratings.empty()) {
      return 0;
    }
    int rating_sum = 0;
    for (const int rating : ratings) {
      rating_sum += rating;
    }
    return rating_sum / static_cast<int>(ratings.size());
  }

  struct Query {
    set<string> plus_words;
    set<string> minus_words;
  };

  Query ParseQuery(const string &query) const {
    Query query_;
    for (auto const &word : SplitIntoWordsNoStop(query)) {
      if (word.empty()) {
        continue;
      }
      if (word[0] == '-') {
        query_.minus_words.insert(word.substr(1));
      } else {
        query_.plus_words.insert(word);
      }
    }
    return query_;
  }

  double ComputeWordInverseDocumentFreq(const string &word) const {
    return log(GetDocumentCount() * 1.0 /
               word_to_document_freqs_.at(word).size());
  }
  template <typename Lambda>
  vector<Document> FindAllDocuments(const Query &query, Lambda lambda) const {
    map<int, double> document_to_relevance;
    for (const string &word : query.plus_words) {
      if (word_to_document_freqs_.count(word) == 0) {
        continue;
      }
      const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
      for (const auto [document_id, term_freq] :
           word_to_document_freqs_.at(word)) {
        document_to_relevance[document_id] += term_freq * inverse_document_freq;
      }
    }

    for (const string &word : query.minus_words) {
      if (word_to_document_freqs_.count(word) == 0) {
        continue;
      }
      for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
        document_to_relevance.erase(document_id);
      }
    }
    vector<Document> matched_documents;
    for (const auto [document_id, relevance] : document_to_relevance) {
        if (lambda(document_id, documents_.at(document_id).status,
            documents_.at(document_id).rating)) {
            matched_documents.push_back(
                { document_id, relevance, documents_.at(document_id).rating });
        }
    }
    return matched_documents;
  }
};

void PrintDocument(const Document &document) {
  cout << "{ "s
       << "document_id = "s << document.id << ", "s
       << "relevance = "s << document.relevance << ", "s
       << "rating = "s << document.rating << " }"s << endl;
}
int main() { return 0; }
