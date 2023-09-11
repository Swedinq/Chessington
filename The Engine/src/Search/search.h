//
// Created by Noah Joubert on 28/07/2023.
//

#ifndef SEARCH_CPP_SEARCH_H
#define SEARCH_CPP_SEARCH_H

#define EXACT_EVAL 1
#define LOWER_EVAL 2
#define UPPER_EVAL 0

#define INFIN 1000000
#define MATE 100000

struct SearchParameters {
    //TODO look into the setting of these
    struct TTParameters{
        int TTSizeMb = 0; // size of the TT in mb  (0 BY DEFAULT)
        int replaceDepth = 1; // the extra depth needed to overwrite a node (must be at least 1)
        int replaceAge = 7; // the extra age needed to overwrite a node (must be at least 1)

        bool useTT = false; // whether we are using the TT in regular search //todo dont forget this is turned off!
        bool useTTPruning = true; // whether we use the TT for pruning (move-ordering used by default)
        bool useTTInQSearch = true; // whether we are using the TT in the quiescence search
    };

    TTParameters ttParameters;

    /* Iterative deepening parameters */
    float minSearchTime = 1; // the minimum time of a search in the iterative deepening framework
    int startingDepth = 1; // the depth at which iterative deepening is started

    /* Quiescence parameters */
    bool useQuiescence = true; // whether we use a quiescence search
    bool useSEE = true; // whether we use SEE
    bool useDelta = true;
    int deltaMargin = 200; // the margin used for delta pruning
    int maxDepthForChecks = -2;

    /* Evaluation parameters */
    int stalemateEvaluation = -1000; // the evaluation of a stalemate position

    /* Main search parameters */
    bool useLMR = false;
    int useLMRDepth = 5; // the minimum depth we must be at for LMR
    int minMovesBeforeLMR = 3; // the minimum full searches needed before a LMR

    /* Multithreading parameters */
    int numThreads = 2;
};

struct SearchStats {
    int totalNodesSearched = 0;
    int totalQuiescenceSearched = 0;
    int totalNonCaptureQSearched = 0; // count how many quiescence nodes aren't captures (ie. checks/ promos)

    void clear(){
        totalNodesSearched = 0;
        totalQuiescenceSearched = 0;
        totalNonCaptureQSearched = 0;
    }
    void add(SearchStats s) {
        totalNodesSearched += s.totalNodesSearched;
        totalQuiescenceSearched += s.totalQuiescenceSearched;
        totalNonCaptureQSearched += s.totalNonCaptureQSearched;
    }
};

struct SearchResults {
    int evaluation;
    Move bestMove;
};

inline short getEvaluationType(int eval, int alpha, int beta) {
    if (eval <= alpha) {
        return UPPER_EVAL;
    } else if (eval >= beta) {
        return LOWER_EVAL;
    } else {
        return EXACT_EVAL;
    }
}


#endif //SEARCH_CPP_SEARCH_H
