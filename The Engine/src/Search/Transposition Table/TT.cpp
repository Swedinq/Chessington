//
// Created by Noah Joubert on 30/07/2023.
//

#include "zobrist.h"

#ifndef SEARCH_ZOBRIST_H
#define SEARCH_ZOBRIST_H

typedef uint16_t Zob16; // used to just hold the last 16 bits of a zobrist key to save memory

// these flags are used to identify whether an evaluation is exact, or an alpha beta cut-off.
#define EXACT_EVAL 1
#define LOWER_EVAL 2
#define UPPER_EVAL 3

/* This struct is for the entry to a transposition table
 * We need to store basic information like the evaluation of the position, the best move found, and part of the zobrist key in order to identify it
 * But we also need to store less obvious information in order to optimise the space in the transposition table like depth, flag, age.
 * */
struct TTNode {
    Zob16 key = 0; // top half of the zobrist key, used to identify a chess position.
    Move move = 0; // the move code of the best move found
    U8 depth = 0; // the depth at which the position was searched
    U8 flag = 0; // holds whether the evaluation is exact, or an alpha-beta cut off
    U8 age = 0; // holds the moveNumber at which this search was done
    int16_t eval = 0; // evaluation of this node
};

/* The actual transposition table. It is essentially a wrapper around an array of TTNodes.
 * To find an entry using a Zobrist hash:
    * The array is indexed by the first part of the zobrist hash
    * The final 16 bits of the hash are stored in the TTNode to see if the same position is being stored
 * There are risks of collisions, where the program believes two entries represent the same position. But hopefully these are minimal.
 * We can reduce this risk by checking that the best move found is a legal move.
 * */
class TranspositionTable {
    // the array will be indexed using the first part of the zobrist key

    // TODO ( second part of the zobrist key will be stored to check for collisions) Is it?

    /* The transposition table will contain 2^n elements. The key will be the first n bits. */
    static const long TTSizePower2 = 15; // the table will be a power of 2 size (will need to be long if greater than 32)
    static const long TTsize = 1 << TTSizePower2; // size
    static const long TTKeyMask = TTsize - 1; // this mask is used to convert a zobrist hash to a key for the transposition table. it takes the first TTSizePower2 bits

    static const int replaceDepth = 1; // the extra depth needed to replace a node
    static const int replaceAge = 4; // the extra age needed to replace a node

    TTNode table[TTsize];  // array which holds the transposition table
public:
    /* These stats keep track of the access statistics */
    int totalProbeCalls = 0, totalProbeFound = 0, totalProbeExact = 0, totalProbeUpper = 0, totalProbeLower = 0; // the number of probes to the TT
    int totalSetCalls = 0; // total number of calls to add a search to the TT
    int totalUniqueNodes = 0; // the total number of nodes added to the TT
    int totalNodesSet = 0; // total number of nodes set. this includes new nodes, overwrites, and collisions
    int totalNewNodesSet = 0, totalOverwrittenNodesSet = 0, totalCollisionsSet = 0;

    TranspositionTable() {
        // init the table
        for (int i = 0; i < TTsize; i++) {
            TTNode node;
            table[i] = node;
        }
    }


    inline TTNode* find(Zobrist &key) {
        // returns the entry for a zobrist key
        int index = zobristToTTKey(key);

        return &table[index];
    }
    inline int zobristToTTKey(Zobrist &key) {
        // converts a zobrist key, to a TT key
        return key & TTKeyMask;
    }
    inline Zob16 toZob16(Zobrist &key) {
        // takes the final 16 bits of the zobrist key. this is stored in the transposition node, and used to check for collisions
        return (Zob16) (key >> 48);
    }
    inline TTNode* probe(Zobrist key, bool &found) {
        // this function gets the entry referenced by this zobrist hash, and checks whether the match is exact
        totalProbeCalls ++;

        // get the node
        TTNode* node = find(key);

        // compare the zobrist keys
        found = node->key == toZob16(key);

        if (found) {
            totalProbeFound += 1;
            if (node->flag == EXACT_EVAL) {
                totalProbeExact ++
                ;} else if (node->flag == UPPER_EVAL) {
                totalProbeUpper++;
            } else if (node->flag == LOWER_EVAL) {
                totalProbeLower ++;
            }
        }

        return node;
    }

    void set(Zobrist key, Move &move, int &depth, int &flag, short age, int &eval) {
        // takes in the results of a search and replaces the node if necessary

        TTNode* node = find(key);
        Zob16 shiftedKey = toZob16(key);

        totalSetCalls ++;

        if (    (   node->key == 0  ) ||
                (   (node->key == shiftedKey)  &&   (depth > node->depth)   ) ||
                (   (node->key != shiftedKey)  &&   ( (depth - replaceDepth > node->depth)   ||  (age - replaceAge > node->age))))
        {
            // the node will be overwritten if
            // a. It is empty
            // b. the node has the same zobrist key and the search is to a higher depth
            // c. the node has a different zobrist key and the search is at a higher depth, or is more recent
            totalNodesSet ++;

            if (node->key == 0) {
                totalNewNodesSet ++;
                totalUniqueNodes ++;
            } else if (node->key == shiftedKey) {
                totalOverwrittenNodesSet ++;
            } else {
                totalCollisionsSet ++;
            }

            node->key = shiftedKey;
            node->move = (Move) move;
            node->depth = (U8) depth;
            node->flag = (U8) flag;
            node->age = (U8) age;
            node->eval = (int16_t) eval;
        }
    }
    void clearTotals() {
        totalProbeCalls = 0, totalProbeFound = 0; // the number of probes to the TT
        totalSetCalls = 0; // total number of calls to add a search to the TT
        totalNodesSet = 0; // the total number of nodes added to the TT
        totalNewNodesSet = 0, totalOverwrittenNodesSet = 0, totalCollisionsSet = 0;
        totalProbeCalls = 0, totalProbeFound = 0, totalProbeExact = 0, totalProbeUpper = 0, totalProbeLower = 0; // the number of probes to the TT
    }

    int getSize() {return TTsize;}
};


#endif //SEARCH_ZOBRIST_H
