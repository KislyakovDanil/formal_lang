#include <iostream>
#include <vector>
#include <stack>
#include <queue>
using namespace std;

bool error = false;

struct FSA { // НКА
    FSA(string reg_exp);
    vector<vector<char>> relations_table; // from, to, edge
    vector<bool> terminals; // терминальные вершины
};

FSA::FSA(string reg_exp){
    relations_table.resize(reg_exp.size(), vector<char>(reg_exp.size(), '0'));
    terminals.resize(reg_exp.size(), false);
    int current_max_vert = 0;
    stack<pair<int,int>> parser;
    pair<int,int> first_current_vertices;
    pair<int,int> second_current_vertices;
    for (char symbol: reg_exp) {
        if (symbol == '1'){
            parser.push(make_pair(current_max_vert, current_max_vert));
            ++current_max_vert;
        } if (symbol == 'a' || symbol == 'b' || symbol == 'c'){
            parser.push(make_pair(current_max_vert, current_max_vert + 1));
            relations_table[current_max_vert][current_max_vert+1] = symbol;
            terminals[current_max_vert + 1] = true;
            current_max_vert += 2;
        } else if (symbol == '*'){
            first_current_vertices.first = parser.top().first;
            first_current_vertices.second = parser.top().second;
            for (int i = first_current_vertices.first + 1; i <= first_current_vertices.second; ++i){ // добавляем рёбра из терминальных вершин в стартовую
                if (terminals[i]) {
                    relations_table[i][first_current_vertices.first] = '1';
                }
            }
            if (!terminals[first_current_vertices.first]){
                terminals[first_current_vertices.first] = true;
            }
        } else {
            if (parser.size() < 2) {
                error = true;
                break;
            }
            second_current_vertices.first = parser.top().first;
            second_current_vertices.second = parser.top().second;
            parser.pop();
            first_current_vertices.first = parser.top().first;
            first_current_vertices.second = parser.top().second;
            parser.pop(); // особенность хранения на стеке
            if (symbol == '.'){
                for (int i = first_current_vertices.first; i <= first_current_vertices.second; ++i) { // провели рёбра из терминальных вершин первого в потомков корня второго
                    for (int j = second_current_vertices.first + 1; j <= second_current_vertices.second; ++j) {
                        if (terminals[i]){
                            relations_table[i][j] = relations_table[second_current_vertices.first][j];
                        }
                    }
                }
                if (!terminals[second_current_vertices.first]){
                    for (int i = first_current_vertices.first; i <= first_current_vertices.second; ++i) {
                        terminals[i] = false;
                    }
                }
                parser.push(make_pair(first_current_vertices.first, second_current_vertices.second));
            } else if (symbol == '+'){
                for (int i = second_current_vertices.first + 1; i <= second_current_vertices.second; ++i){ // подвесили потомков корня второго к корню первого
                    relations_table[first_current_vertices.first][i] = relations_table[second_current_vertices.first][i];
                }
                if (terminals[second_current_vertices.first]){
                    terminals[first_current_vertices.first] = true;
                }
                parser.push(make_pair(first_current_vertices.first, second_current_vertices.second));
            } else {
                error = true;
                break;
            }
        }
    }
    if (parser.size() != 1){
        error = true;
    }
}

bool find(const FSA& fsa, int node, const string& word, int symbol_pos_in_word){
    if (symbol_pos_in_word == word.size()){
        return true;
    }
    for (int i = 0; i < fsa.relations_table[node].size(); ++i){
        if (fsa.relations_table[node][i] == word[symbol_pos_in_word]){
            return find(fsa, i, word, symbol_pos_in_word + 1);
        } else if(fsa.relations_table[node][i] == '1') { // глубина не превосходит одного по построению
            return find(fsa, i, word, symbol_pos_in_word);
        }
    }
    return false;
}

void print_max_suff_length(const FSA& fsa, const string& word){
    for (int first_symb = 0; first_symb < word.size(); ++first_symb){
        if (find(fsa, 0, word, first_symb)){
            cout << word.size() - first_symb;
            return;
        }
    }
    cout << 0;
    return;
}

int main () {
    string reg_exp, word;
    cin >> reg_exp;
    cin >> word;
    FSA fsa = FSA(reg_exp);
    if (error){
        cout << "ERROR";
    } else {
        print_max_suff_length(fsa, word);
    }
    return 0;
} // 29 строк на }