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
  class Heuristic {
    private:
      static Player player;

    public:
      Heuristic();
      Heuristic(Player p);
      Bound heuristic(Node * n) const;
  };

  static Heuristic h;
  GameState board;
  Move prev_move;  // Movement performed to get to this node
  Bound h_value;
  bool maximizing_player;

  Node(const GameState& board, Move prev_move, bool maximizing_player, const Heuristic& h);
  NodeQueue children() const;
};

struct NodeComp {
  bool operator()(Node * n1, Node * n2) {
    return n1->h_value < n2->h_value;
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
