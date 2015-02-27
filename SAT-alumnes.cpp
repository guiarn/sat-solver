#include <iostream>
#include <stdlib.h>
#include <algorithm>
#include <vector>
#include <ctime>
using namespace std;

#define UNDEF -1
#define TRUE 1
#define FALSE 0

uint numVars;
uint numClauses;
vector<vector<int> > clauses;
vector<int> model;
vector<vector<int> > litAppearsIn;
vector<int> modelStack;
uint indexOfNextLitToPropagate;
uint decisionLevel;
int numDecisions;


inline int refLit (int lit) {
	return lit + (lit < 0 ? numVars : 0);
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
    // Read clauses
    for (uint i = 0; i < numClauses; ++i) {
        int lit;
        while (cin >> lit and lit != 0) {
			litAppearsIn[refLit(lit)].push_back(i);
			clauses[i].push_back(lit);
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

//while there is variables to propagate
//checks all clauses, if all literals from clause are false -> keep backtracking
//if only one literal undef in clause and the other literals are false, set it to true and continue
//idea -> aux structure that only has active clauses
bool propagateGivesConflict () {
	while ( indexOfNextLitToPropagate < modelStack.size() ) {
		int litToPropagate = modelStack[indexOfNextLitToPropagate];
		int negatedLitToProp = -litToPropagate;
		++indexOfNextLitToPropagate;
		for (int clauseToCheck : litAppearsIn[refLit(negatedLitToProp)]) {
			bool someLitTrue = false;
			int numUndefs = 0;
			int lastLitUndef = 0;
			for (uint k = 0; not someLitTrue and k < clauses[clauseToCheck].size(); ++k) {
				int val = currentValueInModel(clauses[clauseToCheck][k]);
				if (val == TRUE) someLitTrue = true;
				else if (val == UNDEF) {
					++numUndefs;
					lastLitUndef = clauses[clauseToCheck][k];
				}
			}
			if (not someLitTrue and numUndefs == 1) setLiteralToTrue(lastLitUndef);
			else if (not someLitTrue and numUndefs == 0) return true;
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
	++numDecisions;
}


// Heuristic for finding the next decision literal:
int getNextDecisionLiteral(){
    for (uint i = 1; i <= numVars; ++i) // stupid heuristic:
        if (model[i] == UNDEF) return i;  // returns first UNDEF var, positively
    return 0; // reurns 0 when all literals are defined
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
