/*
 * GriffinBot.h
 *
 * Autor: Antonio Coín Castro
 */

#include "GriffinBot.h"
#include <string>
#include <iostream>
#include <algorithm>
#include <unordered_set>

using namespace std;
using Heuristic = Node::Heuristic;

#define DEBUG 0
#define ITERATIVE_DEEPENING 1
#define MAX_DEPTH 13

/*****

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

* EN EL CUTOFF-TEST HAY QUE TENER EN CUENTA
* EL TIEMPO MÁXIMO Y LOS NODOS quiescent (tener en cuenta que es mejor si eres tú el último que decide) --> MAX_DEPTH impar

* Iterative deepening: In case your program has a strong oscillation in the values it finds for odd and even search depths, you might be better off by feeding MTD(f) its return value of two plies ago, not one, as the above code does. MTD(f) works best with a stable Principal Variation (doesn't every program?) Although the transposition table greatly reduces the cost of doing a re-search, it is still a good idea to not re-search excessively. As a rule, in the deeper iterations of quiet positions in good programs MTD(f) typically performs between 5 and 15 passes before it finds the minimax value.

* Tabla hash. considerar guardar el orden de los nodos para no tener que volver
* a ordenar los hijos ya explorados.

* Añadir H3 a la heurística. Modificarla para que la raíz siempre sea max,
* pero la heurística tenga en cuenta J1 y J2

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

*******/

Player Node::Heuristic::player;
Heuristic Node::h;

Heuristic::Heuristic(){
}

Heuristic::Heuristic(Player p) {
  player = p;
}

Bound Heuristic::heuristic(Node * n) const {
  Player rival = player == J1 ? J2 : J1;
  int rival_score = n->board.getScore(rival);
  int h1 = n->board.getScore(player) - rival_score;
  int h3 = 25 - rival_score;
  return 0.85 * h1 + 0.15 * h3;
}

namespace {

  /**
   * Delete a NodeQueue
   *
   */
  void deleteQueue(NodeQueue& q) {
    while (!q.empty()) {
      auto value = q.top();
      q.pop();
      delete value;
    }
  }

}

Node::Node(const GameState& board, Move prev_move, bool maximizing_player, const Heuristic& h) {
  this->h = h;
  this->board = board;
  this->prev_move = prev_move;
  this->maximizing_player = maximizing_player;
  this->h_value = h.heuristic(this);
}

NodeQueue Node::children() const {
  NodeQueue q;
  for (int i = 1; i <= 6; i++) {
    GameState new_board = board.simulateMove((Move) i);
    bool maximizing = new_board.getCurrentPlayer() == board.getCurrentPlayer()
                      ? maximizing_player : !maximizing_player;
    Node* node = new Node(new_board, (Move) i, maximizing, h);
    q.push(node);
  }

  return q;
}

GriffinBot::GriffinBot() {
}

GriffinBot::~GriffinBot() {
}

void GriffinBot::initialize() {
}

string GriffinBot::getName() {
	return "GriffinBot";
}

/**
 * Implementation of memory-enhanced alpha-beta pruning
 *
 */
pair<Bound, Move> GriffinBot::alphaBetaWithMemory(Node * node, int depth, int alpha, int beta) {
  if (depth == 0 || node->board.isFinalState())
    return make_pair(node->h_value, M_NONE);

  Bound best_bound;
  Move best_move;
  NodeQueue children = node->children();

  if (node->maximizing_player) {
    best_bound = - (1 << 10);  // - infinity

    while (!children.empty()) {
      auto child = children.top();
      auto child_bound = alphaBetaWithMemory(child, depth - 1, alpha, beta).first;
      children.pop();

#if DEBUG == 1

      cerr << "MAX " << ", Depth " << depth << ", Move " << child->prev_move << ", Heuristic "
           << child->h_value << endl;

#endif

      if (child_bound > best_bound) {
        best_bound = child_bound;
        best_move = child->prev_move;
      }

      alpha = max(alpha, best_bound);

      // DELETE PROVISIONAL
      delete child;

      if (beta <= alpha) {
        deleteQueue(children);
        break;
      }
    }

    return make_pair(best_bound, best_move);
  }

  else {  // minimizing_player
    best_bound = 1 << 10;  // + infinity

    while (!children.empty()) {
      auto child = children.top();
      auto child_bound = alphaBetaWithMemory(child, depth - 1, alpha, beta).first;
      children.pop();

#if DEBUG == 1

      cerr << "MIN " << ", Depth " << depth << ", Move " << child->prev_move << ", Heuristic "
           << child->h_value << endl;

#endif

      if (child_bound < best_bound) {
        best_bound = child_bound;
        best_move = child->prev_move;
      }

      beta = min(beta, best_bound);

      // DELETE PROVISIONAL
      delete child;

      if (beta <= alpha) {
        deleteQueue(children);
        break;
      }
    }

    return make_pair(best_bound, best_move);
  }
}

/**
 * Implementation of MTD-f search algorithm
 *
 */
pair<Bound, Move> GriffinBot::mtdf(Node * root, Bound first_guess, int depth) {
  pair<Bound, Move> best_action;
  Bound best_bound = first_guess;
  int upper_bound = 1 << 10;   // + infinity
  int lower_bound = - (1 << 10);  // - infinity

  while (lower_bound < upper_bound) {
    int beta = max(best_bound, lower_bound + 1);
    best_action = alphaBetaWithMemory(root, depth, beta - 1, beta);
    best_bound = best_action.first;

    // Update current bounds
    if (best_bound < beta)
      upper_bound = best_bound;
    else
      lower_bound = best_bound;
  }

  return best_action;
}

Move GriffinBot::nextMove(const vector<Move>& adversary, const GameState& state) {
  static Heuristic heuristic(getPlayer());
  Move next_move;
  Node* root = new Node(state, M_NONE, true, heuristic);
  Bound first_guess = 0;

#if ITERATIVE_DEEPENING == 1

  for (int d = 1; d <= MAX_DEPTH; d += 2) {
    auto solution = mtdf(root, first_guess, d);
    first_guess = solution.first;
    next_move = solution.second;
  }

#else

  next_move = mtdf(root, first_guess, MAX_DEPTH).second;

#endif

  delete root;

	return next_move;
}
