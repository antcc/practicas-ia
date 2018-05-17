/*
 * GriffinBot.h
 *
 * Autor: Antonio Coín Castro
 */

#include "Bot.h"
#include <queue>
#include <vector>

#ifndef GRIFFINBOT_H_
#define GRIFFINBOT_H_

struct Node;
struct NodeComp;

using Bound = int;
using NodeQueue = std::priority_queue<Node*, std::vector<Node*>, NodeComp>;

struct Node {
  GameState board;
  Move prev_move;  // Movement performed to get to this node
  bool maximizing_player;
  int h_value;

  Node(const GameState& board, Move prev_move, bool maximizing_player);
  NodeQueue children();
};

struct NodeComp {
  bool operator()(Node * n1, Node * n2) {
    return n1->h_value > n2->h_value;
  }
};

class GriffinBot : Bot {
  public:
    GriffinBot();
    ~GriffinBot();

    void initialize();
	  string getName();
	  Move nextMove(const vector<Move>& adversary, const GameState& state);
};

#endif /* GRIFFINBOT_H_ */
