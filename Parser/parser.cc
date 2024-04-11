#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include "lexer.h"

using namespace std;

vector<pair<string, vector<string>>> ruleList;  // list of rules, where pair.first = varaiable and pair.second = rules for production
vector<string> terminals;
vector<string> nonterminals;
unordered_set<string> terms;
unordered_set<string> nonterms;

LexicalAnalyzer lexer;
Token token;

// Function to parse the individual rule
void parseRule(int index) { 
    
    // ID ARROW RHS STAR
    string ruleName;
    vector<string> rules;

    token = lexer.GetToken();
    if (token.token_type == ID) {
        string var = token.lexeme;
        //cout << "ID: " << var;
        token = lexer.GetToken();

        if (token.token_type == ARROW) {
            ruleName = var;
            //cout << " -> ";
            token = lexer.GetToken();
            if (token.token_type == STAR) {
                // Rule -> * (epsilon)
                rules.push_back("#");
            }
            else {
                while (token.token_type != STAR) {
                    if (token.token_type == ID) {
                        rules.push_back(token.lexeme);
                        //ruleList[index].second.push_back(token.lexeme);
                        //cout << token.lexeme << " ";
                    }
                    token = lexer.GetToken();
                }
            }
            
        }
    }
    else if (token.token_type == HASH) return;
    else {
        cout << "SYNTAX ERROR !!!" << endl;
        exit(1);
    }

    if (!rules.empty()) {
        pair<string, vector<string>> p = {ruleName, rules};
        ruleList.push_back(p);
        if (nonterms.find(p.first) == nonterms.end()) {     // do not include duplicate nonterminals!
            nonterminals.push_back(p.first);
        }
        nonterms.insert(ruleName);
    }
}

// Function to parse the whole rule list
void parseRuleList() {
    int index = 0;
    while (lexer.peek(1).token_type != END_OF_FILE) {
        parseRule(index);
        index++;
    }
}

// Function to get terminals in the order they appear in the list of rules
void getTerminals() {
    for (int index = 0; index < ruleList.size(); index++) {
        for (int i = 0; i < ruleList[index].second.size(); i++) {
            auto item = ruleList[index].second[i];
            if (nonterms.find(item) == nonterms.end()) {   // if you can't find in list of nonterminals, must be terminal
                if (terms.find(item) == terms.end()) {     // make sure not to duplicate
                    terms.insert(item);
                    //terms[item] += 1;
                    terminals.push_back(item);
                }
            }
        }
    }

    // reorder terminals
    for (auto it = terminals.begin(); it != terminals.end(); it++) {
        if (*it == "#") {
            terminals.erase(it);
            terminals.insert(terminals.begin(), "#");
            break;
        }
    }
}

// Function to get non-terminals in the order they appear in the list of rules
vector<string> getNonTerminalsInOrder() {
    unordered_map<string, int> seen;
    vector<string> ordered;
    ordered.push_back(ruleList[0].first);  // this will always be the first non-terminal seen
    seen[ruleList[0].first] += 1;

    
    for (int index = 0; index < ruleList.size(); index++) {
        auto first = ruleList[index].first;
        // check variable to the left of arrow in the rule
        if (terms.find(first) == terms.end()) {     // if not a terminal
            if (seen.find(first) == seen.end()) {   // and not seen yet
                seen[first] += 1;
                ordered.push_back(first);
            }
        }

        for (int i = 0; i < ruleList[index].second.size(); i++) {
            auto second = ruleList[index].second[i];
            if (terms.find(second) == terms.end()) {      // make sure it's a non-terminal
                if (seen.find(second) == seen.end()) {    // don't add duplicates to list of non-terminals
                    seen[second] += 1;
                    ordered.push_back(second);
                }
            }
        }
    }
    return ordered;
}

// Function to find generating symbols
void findGeneratingSymbols(unordered_set<string> &generating) {
    // start off with generating = terms
    // iterate through rule to find if rhs = only terminals
    // if rhs is only terminals, add lhs to generating set
    // recursive call

    bool changed = false;

    for (auto rule : ruleList) {
        string lhs = rule.first;
        vector<string> rhs = rule.second;

        bool allGenerating = true;
        for (string symbol : rhs) {
            if (generating.find(symbol) == generating.end()) {
                allGenerating = false;
                break;
            }
        }

        if (allGenerating && generating.find(lhs) == generating.end()) {
            generating.insert(lhs);
            changed = true;
        }
    }

    if (changed) findGeneratingSymbols(generating);
}

// Function to find non-generating symbols
unordered_set<string> findNonGeneratingSymbols(unordered_set<string> &generating) {
    unordered_set<string> nongenerating;
    bool changed = false;

    for (string symbol : nonterminals) {
        if (generating.find(symbol) == generating.end()) {
            nongenerating.insert(symbol);
        }
    }
    
    return nongenerating;
}

void removeNonGeneratingSymbols(unordered_set<string> &nongenerating) {
    for (auto it = ruleList.begin(); it != ruleList.end(); ) {
        bool allGenerating = true;
        if (nongenerating.find(it->first) != nongenerating.end()) {
            allGenerating = false;
        }
        else {
            for (string symbol : it->second) {
                if (nongenerating.find(symbol) != nongenerating.end()) {
                    allGenerating = false;
                    break;
                }
            }
        }
        if (!allGenerating) {
            it = ruleList.erase(it);
        }
        else it++;
    }
}

void markReachable(string symbol, unordered_set<string> &reachable) {
    // exit condition
    if (reachable.find(symbol) != reachable.end()) return;

    reachable.insert(symbol);

    for (auto rule : ruleList) {
        if (rule.first == symbol) {
            for (string rhsSymbol : rule.second) {
                markReachable(rhsSymbol, reachable);
            }
        }
    }
}

unordered_set<string> findUnreachableSymbols(unordered_set<string> &unreachable) {
    unordered_set<string> reachable;
    string startSymbol = ruleList[0].first;

    markReachable(startSymbol, reachable);
    
    for (auto rule : ruleList) {
        string symbol = rule.first;
        if (reachable.find(symbol) == reachable.end()) {
            unreachable.insert(symbol);
        }
    }

    return unreachable;
}

void removeUnreachableSymbols(unordered_set<string> &unreachable) {
    for (auto it = ruleList.begin(); it != ruleList.end(); ) {
        bool remove = false;
        if (unreachable.find(it->first) != unreachable.end()) {
            remove = true;
        }
        else {
            for (string symbol : it->second) {
                if (unreachable.find(symbol) != unreachable.end()) {
                    remove = true;
                    break;
                }
            }
        }

        if (remove) it = ruleList.erase(it);
        else it++;
    }
}

bool allDeriveEpsilon(map<string, vector<string>> &firstSet, vector<string> &rhs, int index) {
    for (int i = index; i < rhs.size(); i++) {
        if (find(firstSet[rhs[i]].begin(), firstSet[rhs[i]].end(), "#") == firstSet[rhs[i]].end()) return false;
    }
    return true;
}

void findFirstSets(map<string, vector<string>> &firstSet) {
    bool changed;
    do {
        changed = false;
        for (auto rule : ruleList) {
            string lhs = rule.first;
            vector<string> rhs = rule.second;

            // check if rule has epsilon
            if (rhs[0] == "#") {
                if (find(firstSet[lhs].begin(), firstSet[lhs].end(), "#") == firstSet[lhs].end()) { // if epsilon is not in FIRST(lhs)
                    firstSet[lhs].push_back("#");
                    changed = true;
                }
            }
            else if (terms.find(rhs[0]) != terms.end()) {   // is it a terminal
                if (find(firstSet[lhs].begin(), firstSet[lhs].end(), rhs[0]) == firstSet[lhs].end()) {
                    firstSet[lhs].push_back(rhs[0]);
                    changed = true;
                }
            }
            else {  // not epsilon or terminal, must be nonterminal
                bool hasEpsilon = false;
                for (string rhsSymbol : rhs) {
                    hasEpsilon = false;     // reset hasEpsilon for each symbol on rhs of rule
                    if (nonterms.find(rhsSymbol) != nonterms.end()) {   // if non-terminal
                        // add FIRST(rhsSymbol) to FIRST(lhs)
                        for (string symbol : firstSet[rhsSymbol]) {
                            if (symbol == "#") hasEpsilon = true;
                            else if (find(firstSet[lhs].begin(), firstSet[lhs].end(), symbol) == firstSet[lhs].end()) {
                                firstSet[lhs].push_back(symbol);
                                changed = true;
                            }
                        }
                        if (!hasEpsilon) break;     // if FIRST(rhsSymbol) doesn't have epsilon, break
                    }
                    else {
                        if (terms.find(rhsSymbol) != terms.end() && find(firstSet[lhs].begin(), firstSet[lhs].end(), rhsSymbol) == firstSet[lhs].end()) {
                            firstSet[lhs].push_back(rhsSymbol);
                            changed = true;
                        }
                        break;
                    }
                }

                if (hasEpsilon) {   // if all symbols on RHS have epsilon in their FIRST set...
                    if (find(firstSet[lhs].begin(), firstSet[lhs].end(), "#") == firstSet[lhs].end()) {
                        firstSet[lhs].push_back("#");
                        changed = true;
                    }
                }
            }
        }
    }
    while (changed);
}

void findFollowSets(map<string, vector<string>> &firstSet, map<string, vector<string>> &followSet) {
    // add $ to FOLLOW(startSymbol)
    followSet[ruleList[0].first].push_back("$");

    bool changed;

    do {
        changed = false;

        for (auto rule : ruleList) {
            string lhs = rule.first;
            vector<string> rhs = rule.second;

            for (int i = 0; i < rhs.size(); i++) {
                string symbol = rhs[i];

                // if symbol == non-terminal
                if (nonterms.find(symbol) != nonterms.end()) {
                    bool hasEpsilon = false;
                    // Case 1: A -> αBβ, add FIRST(β) to FOLLOW(B)
                    if (i < rhs.size() - 1) {   // if not last symbol in rule
                        string nextSymbol = rhs[i + 1]; // get follow symbol
                        if (nonterms.find(nextSymbol) != nonterms.end()) {  // if follow symbol is non-terminal, FOLLOW(symbol) += FIRST(nextSymbol)
                            for (string s : firstSet[nextSymbol]) {
                                if (s != "#") {
                                    if (find(followSet[symbol].begin(), followSet[symbol].end(), s) == followSet[symbol].end()) {
                                        followSet[symbol].push_back(s);
                                        changed = true;
                                    }
                                }
                                else hasEpsilon = true;
                            }
                            if (hasEpsilon) {       // if nextSymbol derives epsilon, we need to add FIRST(nextNextSymbol) to FOLLOW(symbol), must account for if nextNextSymbol is terminal or non-terminal
                                if (i + 2 < rhs.size()) {
                                    if (nonterms.find(rhs[i + 2]) != nonterms.end()) {  // if nextNextSymbol is non-terminal
                                        for (string s : firstSet[rhs[i + 2]]) {
                                            if (s != "#") {
                                                if (find(followSet[symbol].begin(), followSet[symbol].end(), s) == followSet[symbol].end()) {
                                                    followSet[symbol].push_back(s);
                                                    changed = true;
                                                }
                                            }
                                        }
                                    }
                                    else if (terms.find(rhs[i + 2]) != terms.end()) {   // if nextNextSymbol is terminal
                                        if (rhs[i + 2] != "#") {
                                            if (find(followSet[symbol].begin(), followSet[symbol].end(), rhs[i + 2]) == followSet[symbol].end()) {
                                                followSet[symbol].push_back(rhs[i + 2]);
                                                changed = true;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        else if (terms.find(nextSymbol) != terms.end()) {   // if follow symbol is terminal, FOLLOW(symbol) += terminal
                            if (nextSymbol != "#") {    // redundant
                                if (find(followSet[symbol].begin(), followSet[symbol].end(), nextSymbol) == followSet[symbol].end()) {
                                    followSet[symbol].push_back(nextSymbol);
                                    changed = true;
                                }
                            }
                        }
                    }
                    // Case 2: A -> αB or A -> αBβ where β derives epsilon, add FOLLOW(A) to FOLLOW(B)
                    if (i == rhs.size() - 1 || (i < rhs.size() - 1 && allDeriveEpsilon(firstSet, rhs, i + 1))) {        // <--- can't use 'else if' here because it will never apply Case 2 AND Case 1. We need to apply both!
                        for (auto entry : followSet[lhs]) {
                            if (find(followSet[symbol].begin(), followSet[symbol].end(), entry) == followSet[symbol].end()) {
                                followSet[symbol].push_back(entry);
                                changed = true;
                            }
                        }
                    }
                }
            }
        }
    }
    while (changed);
}

vector<pair<string, vector<string>>> reorderFirstSets(map<string, vector<string>> firstSet) {
    vector<string> nonterminalOrdered = getNonTerminalsInOrder();
    vector<pair<string, vector<string>>> orderedFirstSet(nonterminalOrdered.size());

    int i = 0;
    for (string symbol : nonterminalOrdered) {
        orderedFirstSet[i].first = symbol;
        // check if firstSet(symbol) has epsilon
        if (find(firstSet[symbol].begin(), firstSet[symbol].end(), "#") != firstSet[symbol].end()) {
            orderedFirstSet[i].second.push_back("#");
        }
        for (string fSymbol : firstSet[symbol]) {
            if (fSymbol != "#") {
                orderedFirstSet[i].second.push_back(fSymbol);
            }
        }
        i++;
    }

    return orderedFirstSet;
}

vector<string> getFirst(vector<string> rhs, map<string, vector<string>> &firstSet) {
    vector<string> first;
    for (string symbol : rhs) {
        bool hasEpsilon = false;
        if (nonterms.find(symbol) != nonterms.end()) {
            hasEpsilon = find(firstSet[symbol].begin(), firstSet[symbol].end(), "#") != firstSet[symbol].end();
            for (string s : firstSet[symbol]) {
                if (s != "#") {
                    if (find(first.begin(), first.end(), s) == first.end())
                        first.push_back(s);
                }
            }
            if (!hasEpsilon) break;
        }
        else if (terms.find(symbol) != terms.end()) {
            if (find(first.begin(), first.end(), symbol) == first.end()) {
                first.push_back(symbol);
            }
            break;
        }
    }

    // check all derive epsilon
    bool includeEpsilon = allDeriveEpsilon(firstSet, rhs, 0);
    if (includeEpsilon) first.push_back("#");

    return first;
}

bool hasIntersection(vector<string> s1, vector<string> s2) {
    for (string s : s1) {
        if (find(s2.begin(), s2.end(), s) != s2.end()) return true;
    }
    return false;
}

void printRuleList() {
    int index = 0;
    while (index < ruleList.size()) {
        cout << ruleList[index].first << " -> ";
        for (int i = 0; i < ruleList[index].second.size(); i++) {
            cout << ruleList[index].second[i] << " ";
        }
        cout << '\n';
        index++;
    }
}

// read grammar
void ReadGrammar() {
    parseRuleList();
}

// Task 1
void printTerminalsAndNoneTerminals() {
    getTerminals();
    vector<string> nonterminalsOrdered = getNonTerminalsInOrder();

    for (int i = 0; i < terminals.size(); i++) {
        if (terminals[i] != "#") cout << terminals[i] << " ";
    }
    
    for (int i = 0; i < nonterminalsOrdered.size(); i++) {
        cout << nonterminalsOrdered[i] << " ";
    }
}


void printing(unordered_set<string> toPrint) {
    for (string s : toPrint) {
        cout << s << " ";
    }
    cout << '\n';
}

// Task 2
void RemoveUselessSymbols() {
    unordered_set<string> generating;
    unordered_set<string> nongenerating;
    unordered_set<string> unreachable;
    
    parseRuleList();    // this gets non-terminals as well
    string startSymbol = ruleList[0].first;

    // get terminals
    getTerminals();
    generating = terms;

    // find generating symbols
    findGeneratingSymbols(generating);
    if (generating.find(startSymbol) == generating.end()) return;   // if the start symbol is non-generating, then all symbols are useless!
    
    // find Non-Generating Symbols
    nongenerating = findNonGeneratingSymbols(generating);

    // remove non-generating symbols
    removeNonGeneratingSymbols(nongenerating);

    // find unreachable symbols
    unreachable = findUnreachableSymbols(unreachable);

    // remove all unreachable rules
    removeUnreachableSymbols(unreachable);
    
    printRuleList();
}

// Task 3
void CalculateFirstSets() {
    map<string, vector<string>> firstSet;
    vector<string> nonterminalsOrdered;

    parseRuleList();
    getTerminals();
    nonterminalsOrdered = getNonTerminalsInOrder();
    
    findFirstSets(firstSet);
    
    // reorder first sets
    //orderedFirstSet = reorderFirstSets(firstSet);

    // print first sets
    for (string entry : nonterminalsOrdered) {
        cout << "FIRST(" << entry << ") = { ";
        int i = 0;
        for (string t : terminals) {
            if (find(firstSet[entry].begin(), firstSet[entry].end(), t) != firstSet[entry].end()) {
                i++;
                if (i < firstSet[entry].size())
                    cout << t << ", ";
                else cout << t;
            }
        }
        cout << " }\n";
    }
}

// Task 4
void CalculateFollowSets() {
    map<string, vector<string>> firstSet;
    map<string, vector<string>> followSet;
    vector<string> nonterminalsOrdered;

    parseRuleList();
    getTerminals();
    nonterminalsOrdered = getNonTerminalsInOrder();

    findFirstSets(firstSet);
    findFollowSets(firstSet, followSet);

    for (string entry : nonterminalsOrdered) {
        int i = 0;
        cout << "FOLLOW(" << entry << ") = { ";

        // check for "$" in FOLLOW(entry)
        bool hasDollar = find(followSet[entry].begin(), followSet[entry].end(), "$") != followSet[entry].end();

        if (hasDollar) {
            i++;
            cout << "$";
            if (followSet[entry].size() > 1)
                cout << ", ";
        }

        // print other terminals
        for (string t : terminals) {
            if (find(followSet[entry].begin(), followSet[entry].end(), t) != followSet[entry].end()) {
                i++;
                if (i < followSet[entry].size())
                    cout << t << ", ";
                else cout << t;
            }
        }
        cout << " }\n";
    }
}

// Task 5
void CheckIfGrammarHasPredictiveParser() {
    unordered_set<string> generating;
    unordered_set<string> nongenerating;
    unordered_set<string> unreachable;
    map<string, vector<string>> firstSet;
    map<string, vector<string>> followSet;
    vector<string> nonterminalsOrdered;

    bool hasUseless = false;
    
    parseRuleList();    // this gets non-terminals as well
    string startSymbol = ruleList[0].first;

    // get terminals
    getTerminals();
    generating = terms;

    nonterminalsOrdered = getNonTerminalsInOrder();

    // Find generating, non-generating, and unreachable symbols
    findGeneratingSymbols(generating);
    if (generating.find(startSymbol) == generating.end()) hasUseless = true; // if the start symbol is non-generating, then all symbols are useless!
    nongenerating = findNonGeneratingSymbols(generating);
    if (!nongenerating.empty()) hasUseless = true;
    removeNonGeneratingSymbols(nongenerating);
    unreachable = findUnreachableSymbols(unreachable);
    removeUnreachableSymbols(unreachable);  // remove all unreachable rules
    if (!unreachable.empty()) hasUseless = true;

    if (hasUseless) {
        cout << "NO";
        return;
    }


    // Doesn't contain useless symbols... Must calculate FIRST and FOLLOW sets
    findFirstSets(firstSet);
    findFollowSets(firstSet, followSet);

    // Calculate FIRST sets of each rule
    map<string, vector<vector<string>>> firstSetOfEachRule;
    for (auto rule : ruleList) {
        string lhs = rule.first;
        vector<string> rhs = rule.second;

        vector<string> first = getFirst(rhs, firstSet);
        firstSetOfEachRule[lhs].push_back(first);
    }

    // Check conditions for predictive parser
    for (auto set : firstSetOfEachRule) {
        string lhs = set.first;
        vector<vector<string>> rhs = set.second;
        int numSets = rhs.size();
        bool setsIntersect = false;
        bool hasEpsilon = false;

        if (find(firstSet[lhs].begin(), firstSet[lhs].end(), "#") != firstSet[lhs].end())
            hasEpsilon = true;

        if (numSets > 1) {
            for (int i = 0; i < numSets - 1; i++) {
                for (int j = 1; j < numSets; j++) {
                    setsIntersect = hasIntersection(rhs[i], rhs[j]);
                    if (setsIntersect) break;
                }
            }
        }
        if (!setsIntersect && (numSets == 1 || hasEpsilon)) {
            setsIntersect = hasIntersection(firstSet[lhs], followSet[lhs]);
        }

        if (setsIntersect) {
            cout << "NO";
            return;
        }
    }

    cout << "YES";
}
    
int main (int argc, char* argv[]) {
    int task;

    if (argc < 2) {
        cout << "Error: missing argument\n";
        return 1;
    }

    /*
       Note that by convention argv[0] is the name of your executable,
       and the first argument to your program is stored in argv[1]
     */

    task = atoi(argv[1]);
    
    ReadGrammar();  // Reads the input grammar from standard input

    switch (task) {
        case 1: printTerminalsAndNoneTerminals();
            break;

        case 2: RemoveUselessSymbols();
            break;

        case 3: CalculateFirstSets();
            break;

        case 4: CalculateFollowSets();
            break;

        case 5: CheckIfGrammarHasPredictiveParser();
            break;

        default:
            cout << "Error: unrecognized task number " << task << "\n";
            break;
    }
    return 0;
}