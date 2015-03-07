#include <iostream>
#include <stdlib.h>
#include <algorithm>
#include <vector>
#include <ctime>
using namespace std;

#define UNDEF -1
#define TRUE 1
#define FALSE 0
#define NBTSTODECAY 1300

uint numVars;
uint numClauses;
vector<vector<int> > clauses;
vector<vector<int> > litAppearsIn;
vector<int> model;
vector<int> modelStack;
vector<int> VSIDS;
uint indexOfNextLitToPropagate;
uint decisionLevel;
int numDecisions;
int timeToDecay;


inline int refLit (int lit) {
    return (lit < 0 ? -lit+numVars+1 : lit);
}

void readClauses( ){
    // Skip comments
    char c = cin.get();
    while (c == 'c') {
        while (c != '\n') c = cin.get();
        c = cin.get();
    }  
    // Read "cnf numVars numClauses"
    string aux;
    cin >> aux >> numVars >> numClauses;
    clauses.resize(numClauses);
    litAppearsIn.resize((numVars+1)*2);
    VSIDS.resize(numVars+1,0);
    // Read clauses
    for (uint i = 0; i < numClauses; ++i) {
        int lit;
        while (cin >> lit and lit != 0) {
            litAppearsIn[refLit(lit)].push_back(i);
            clauses[i].push_back(lit);
            VSIDS[abs(lit)] += 10;
        }
    }    
}


int currentValueInModel(int lit){
    if (lit >= 0) return model[lit];
    else {
        if (model[-lit] == UNDEF) return UNDEF;
        else return 1 - model[-lit];
    }
}


void setLiteralToTrue(int lit){
    modelStack.push_back(lit);
    if (lit > 0) model[lit] = TRUE;
    else model[-lit] = FALSE;       
}


bool propagateGivesConflict () {
    while ( indexOfNextLitToPropagate < modelStack.size() ) {
        int litToPropagate = modelStack[indexOfNextLitToPropagate];
        ++indexOfNextLitToPropagate;
        int r = refLit(-litToPropagate);
        for (const int clauseToCheck : litAppearsIn[r]) {
            bool someLitTrue = false;
            int numUndefs = 0;
            int litUndef = 0;
            uint sizeClause = clauses[clauseToCheck].size();
            for (uint k = 0; not someLitTrue and k < sizeClause; ++k) {
                int val = currentValueInModel(clauses[clauseToCheck][k]);
                if (val == TRUE) someLitTrue = true;
                else if (val == UNDEF) {
                    ++numUndefs;
                    litUndef = clauses[clauseToCheck][k];
                }
            }
            if (not someLitTrue and numUndefs == 1) setLiteralToTrue(litUndef);
            else if (not someLitTrue and numUndefs == 0) {
                for (uint k = 0; k < sizeClause; ++k) {
                    VSIDS[abs(clauses[clauseToCheck][k])] += 100;
                }
                return true;
            }
        }
    }
    return false;
}


void backtrack(){
    uint i = modelStack.size() -1;
    int lit = 0;
    while (modelStack[i] != 0){ // 0 is the DL mark
        lit = modelStack[i];
        model[abs(lit)] = UNDEF;
        modelStack.pop_back();
        --i;
    }
    // at this point, lit is the last decision
    modelStack.pop_back(); // remove the DL mark
    --decisionLevel;
    indexOfNextLitToPropagate = modelStack.size();
    setLiteralToTrue(-lit);  // reverse last decision
    --timeToDecay;
}

inline void decayScores () {
    for (int& x : VSIDS) x >>= 1;
}

// Heuristic for finding the next decision literal:
int getNextDecisionLiteral(){
    int mx = -1, lit = 0;
    for (uint i = 1; i <= numVars; ++i) {
        if (currentValueInModel(i) == UNDEF and VSIDS[i] > mx) {
            mx = VSIDS[i];
            lit = i;
        }
    }
    if (litAppearsIn[lit].size() < litAppearsIn[refLit(-lit)].size()) lit = -lit;
    return lit; // reurns 0 when all literals are defined
}

void checkmodel(){
    for (uint i = 0; i < numClauses; ++i){
        bool someTrue = false;
        for (uint j = 0; not someTrue and j < clauses[i].size(); ++j)
            someTrue = (currentValueInModel(clauses[i][j]) == TRUE);
        if (not someTrue) {
            cout << "Error in model, clause is not satisfied:";
            for (int j = 0; j < (int)clauses[i].size(); ++j) cout << clauses[i][j] << " ";
            cout << endl;
            exit(1);
        }
    }  
}

int printResults (bool b, clock_t s) {
    clock_t end = clock();
    cout.setf(ios::fixed);
    cout.precision(6);
    if (b) cout << "s SATISFIABLE" << endl;
    else cout << "s UNSATISFIABLE" << endl;
    cout << "c " << numDecisions << " decisions" << endl;
    double elapsed = double(end - s) / CLOCKS_PER_SEC;
    cout << "c " << elapsed << " seconds total run time" << endl;
    return (b ? 20:10);
}

int main(){ 
    readClauses(); // reads numVars, numClauses and clauses
    model.resize(numVars+1,UNDEF);
    indexOfNextLitToPropagate = 0;  
    decisionLevel = 0;
    numDecisions = 0;
    timeToDecay = NBTSTODECAY;
    clock_t begin = clock();
    
    // Take care of initial unit clauses, if any
    for (uint i = 0; i < numClauses; ++i){
        if (clauses[i].size() == 1) {
            int lit = clauses[i][0];
            int val = currentValueInModel(lit);
            if (val == FALSE) return printResults(false,begin);
            else if (val == UNDEF) setLiteralToTrue(lit);
        }
    }

    // DPLL algorithm
    while (true) {
        while ( propagateGivesConflict() ) {
            if (decisionLevel == 0) return printResults(false,begin);
            backtrack();
            if (timeToDecay <= 0) {
                decayScores(); 
                timeToDecay = NBTSTODECAY;
            }
        }
        int decisionLit = getNextDecisionLiteral();
        if (decisionLit == 0) {
            checkmodel();
            return printResults(true,begin);
        }
        // start new decision level:
        modelStack.push_back(0);  // push mark indicating new DL
        ++indexOfNextLitToPropagate;
        ++decisionLevel;
        setLiteralToTrue(decisionLit);    // now push decisionLit on top of the mark
        ++numDecisions;
    }
}  
