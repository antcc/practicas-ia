/*
 * GriffinBot.h
 *
 * Author: Antonio Coín Castro
 */

#include "Bot.h"
#include <queue>
#include <array>
#include <unordered_map>
#include <chrono>

#ifndef GRIFFINBOT_H_
#define GRIFFINBOT_H_

#define NUM_PITS 6

struct Node;
struct NodeInfo;
struct NodeComp;
struct NodeHash;

using Bound = int;
using NodeList = array<Node*, NUM_PITS>;
using TTable = std::unordered_map<Node*, NodeInfo, NodeHash, NodeComp>;
using BoundAndMove = std::pair<Bound, Move>;

/**
 * Data structure that represents a state in the search space.
 */
struct Node {
  /**
   * Data structure that represents an (heuristic) evaluation
   * function nodes.
   */
  class Heuristic {
    private:
      static Player player;  // Whether our bot is playing as J1 or J2

    public:
      Heuristic();
      Heuristic(Player p);

      Bound heuristic(Node * n) const __attribute__((optimize("-O2")));
  };

  static Heuristic h;      // Evaluation function chosen for nodes
  GameState board;         // Current board state
  Move prev_move;          // Movement performed to get to this node
  Bound h_value;           // Heuristic value of node
  bool is_maximizing;      // Whether this is a MAX or a MIN node

  Node(const GameState& board, Move prev_move, bool maximizing_player, const Heuristic& h);
  NodeList children() __attribute__((optimize("-O2")));
  bool hasExtraTurn(Node * parent);
};

/**
 * Custom ordering for nodes
 */
struct NodeOrder {
  bool is_parent_maximizing;

  NodeOrder(bool is_parent_maximizing);
  bool operator()(Node * n1, Node * n2) const __attribute__((optimize("-O2")));
};

/**
 * Custom comparator of equality for nodes.
 * It is independent of the order defined by NodeOrder
 */
struct NodeComp {
  bool operator()(Node * n1, Node * n2) const __attribute__((optimize("-O2")));
};

/**
 * Custom hash function for nodes.
 */
struct NodeHash {
  std::size_t operator()(Node * n) const __attribute__((optimize("-O2")));
};

/**
 * Essential information of a node to store in
 * transposition table for reusing.
 */
struct NodeInfo {
  int depth;
  int age;
  bool is_lowerbound;  // Whether the bound is a lower bound or an upper bound
  Bound value;
  Move best_move;
};

/**
 * Data structure that represents an agent playing the game
 */
class GriffinBot : Bot {
  public:
    GriffinBot();
    ~GriffinBot();

    void initialize();
	  string getName();
	  Move nextMove(const vector<Move>& adversary, const GameState& state) __attribute__((optimize("-O2")));

  private:
    TTable table;
    int num_moves;
    std::chrono::high_resolution_clock::time_point begin;

    BoundAndMove alphaBetaWithMemory(Node * node, int depth, int alpha, int beta) __attribute__((optimize("-O2")));
    BoundAndMove mtdf(Node * root, Bound first_guess, int depth) __attribute__((optimize("-O2")));
};

#endif /* GRIFFINBOT_H_ */
