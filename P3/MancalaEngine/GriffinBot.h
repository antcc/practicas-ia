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
  static Player self;  // Stores whether we are J1 or J2
  GameState board;
  Move prev_move;  // Movement performed to get to this node
  bool maximizing_player;
  int h_value;

  Node(const GameState& board, Move prev_move, bool maximizing_player);
  static void setPlayer(Player p);
  NodeQueue children() const;
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
    
  private:
    std::pair<Bound, Move> alphaBetaWithMemory(Node * node, int depth, int alpha, int beta);
    std::pair<Bound, Move> mtdf(Node * root, Bound first_guess, int depth);
};

#endif /* GRIFFINBOT_H_ */
