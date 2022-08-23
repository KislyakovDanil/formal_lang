#include <iostream>
#include <vector>
#include <stack>
#include <queue>
#include <memory>
using namespace std;

bool error = false;

struct Node;

struct Edge{ // ребро
    Edge(shared_ptr<Node> t, char s): to(t), symbol(s){};
    shared_ptr<Node> to;
    char symbol;
};

struct Node { // вершина
    Node():terminal(false), children(0, nullptr){};
    bool terminal;
    vector<shared_ptr<Edge>> children; // потомки
};

struct FSA { // НКА
    FSA(): root(make_shared<Node>(Node())),  terminals(0, nullptr){};
    FSA(string reg_exp);
    shared_ptr<Node> root; // стартовая вершина
    vector<shared_ptr<Node>> terminals; // терминальные вершины
};

shared_ptr<FSA> kleene_star(shared_ptr<FSA> fsa) {
    for (shared_ptr<Node> node: fsa->terminals){ // добавляем рёбра из терминальных вершин в стартовую
        if (node != fsa->root) {
            node->children.push_back(make_shared<Edge>(Edge(fsa->root, '1')));
        }
    }
    if (!fsa->root->terminal){
        fsa->root->terminal = true;
        fsa->terminals.push_back(fsa->root);
    }
    return  fsa;
}

shared_ptr<FSA> concatenation(shared_ptr<FSA> first_language, shared_ptr<FSA> second_language) {
    for (shared_ptr<Node> node: first_language->terminals) { // провели рёбра из терминальных вершин первого в потомков корня второго
        for (shared_ptr<Edge> edge: second_language->root->children) {
            node->children.push_back(edge);
        }
    }
    if (!second_language->root->terminal){
        for (shared_ptr<Node> node: first_language->terminals){
            node->terminal = false;
        }
        first_language->terminals.resize(0);
    }
    for (shared_ptr<Node> node: second_language->terminals) { // теперь все терминальные вершины второго являются терминальными вершинами первого
        first_language->terminals.push_back(node);
    }
    return first_language;
}

shared_ptr<FSA> addition(shared_ptr<FSA> first_language, shared_ptr<FSA> second_language) {
    for (shared_ptr<Edge> edge: second_language->root->children){ // подвесили потомков корня второго к корню первого
        first_language->root->children.push_back(edge);
    }
    if (second_language->root->terminal){
        first_language->root->terminal = true;
    }
    for (shared_ptr<Node> node: second_language->terminals){ // сделали терминальные вершины второго терминальными вершинами первого
        if (node != second_language->root){
            first_language->terminals.push_back(node);
        }
    }
    return first_language;
}

FSA::FSA(string reg_exp){
    stack<shared_ptr<FSA>> parser;
    shared_ptr<FSA> first_language = nullptr;
    shared_ptr<FSA> second_language = nullptr;
    for (char symbol: reg_exp) {
        if (symbol == '1'){
            shared_ptr<FSA> fsa = make_shared<FSA>(FSA());
            fsa->terminals.push_back(fsa->root); //сделали корень терминалом
            fsa->root->terminal = true;
            parser.push(fsa);
        } if (symbol == 'a' || symbol == 'b' || symbol == 'c'){
            shared_ptr<FSA> fsa = make_shared<FSA>(FSA());
            fsa->root->children.push_back(make_shared<Edge>(Edge(make_shared<Node>(Node()), symbol))); // провели ребро из корня в новую вершину
            fsa->root->children[0]->to->terminal = true;
            fsa->terminals.push_back(fsa->root->children[0]->to); // и сделали новую вершину терминальной
            parser.push(fsa);
        } else if (symbol == '*'){
            first_language = parser.top();
            parser.pop();
            parser.push(kleene_star(first_language));
        } else {
            if (parser.size() < 2) {
                error = true;
                return;
            }
            second_language = parser.top();
            parser.pop();
            first_language = parser.top();
            parser.pop(); // особенность хранения на стеке
            if (symbol == '.'){
                parser.push(concatenation(first_language, second_language));
            } else if (symbol == '+'){
                parser.push(addition(first_language, second_language));
            } else {
                error = true;
                return;
            }
        }
    }
    if (parser.size() != 1){
        error = true;
        return;
    }
    shared_ptr<FSA> fca = nullptr;
    fca = parser.top();
    root = fca->root;
    for (shared_ptr<Node> node: fca->terminals){
        terminals.push_back(node);
    }
}

void find_next(queue<shared_ptr<Node>>& currents_nodes, shared_ptr<Node> node, char symbol, int& new_currents_node_num){
    for (shared_ptr<Edge> edge: node->children){
        if (edge->symbol == symbol){
            currents_nodes.push(edge->to);
            ++new_currents_node_num;
        } else if(edge->symbol == '1') { // глубина не превосходит одного по построению
            find_next(currents_nodes, edge->to, symbol, new_currents_node_num);
        }
    }
}

void print_max_suff_length(shared_ptr<Node> root, const string& word){
    for (int first_symb = 0; first_symb < word.size(); ++first_symb){
        queue<shared_ptr<Node>> currents_nodes;
        currents_nodes.push(root);
        int currents_node_num = 1;
        for (int symb = first_symb; symb < word.size(); ++symb){
            int new_currents_node_num = 0;
            for (int i = 0; i < currents_node_num; ++i){
                find_next(currents_nodes, currents_nodes.front(), word[symb], new_currents_node_num);
                currents_nodes.pop();
            }
            if (new_currents_node_num == 0){
                break;
            } else {
                currents_node_num = new_currents_node_num;
            }
            if (symb == word.size() - 1){
                while(!currents_nodes.empty()){
                    if (currents_nodes.front()->terminal){
                        cout << word.size() - first_symb;
                        return;
                    }
                    currents_nodes.pop();
                }
            }
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
        print_max_suff_length(fsa.root, word);
    }
    return 0;
}