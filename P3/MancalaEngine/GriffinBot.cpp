/*
 * GriffinBot.h
 *
 * Autor: Antonio Coín Castro
 */

#include "GriffinBot.h"
#include <string>
#include <cstdlib>
#include <iostream>
#include <unordered_set>

using namespace std;

#define ITERATIVE_DEEPENING 1
#define MAX_DEPTH 11

/*****

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

* EN EL CUTOFF-TEST HAY QUE TENER EN CUENTA
  EL TIEMPO MÁXIMO Y LOS NODOS quiescent.  Consider storing leaf and quiescence nodes in the table.

* Tabla hash

* Añadir H3 a la heurística. Modificarla para que la raíz siempre sea max,
* pero la heurística tenga en cuenta J1 y J2

* Iterative deepening: considerar guardar el orden de los nodos para no tener que volver
* a ordenar los hijos ya explorados.

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

*******/

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

  /**
   * Heuristic function to evaluate the score of a board
   *
   */
  int heuristic(Node * n) {
    return n->board.getScore(J1) - n->board.getScore(J2);
  }

  /**
   * Implementation of memory-enhanced alpha-beta pruning
   *
   */
  pair<Bound, Move> alphaBetaWithMemory(Node * node, int depth, int alpha, int beta) {
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
  pair<Bound, Move> mtdf(Node * root, Bound first_guess, int depth) {
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

}

Node::Node(const GameState& board, Move prev_move, bool maximizing_player) {
  this->board = board;
  this->prev_move = prev_move;
  this->maximizing_player = maximizing_player;
  this->h_value = heuristic(this);
}

NodeQueue Node::children() {
  NodeQueue q;
  for (int i = 1; i <= 6; i++) {
    GameState new_board = board.simulateMove((Move) i);
    bool maximizing = new_board.getCurrentPlayer() == board.getCurrentPlayer()
                      ? maximizing_player : !maximizing_player;
    Node* node = new Node(new_board, (Move) i, maximizing);
    q.push(node);
  }

  return q;
}

GriffinBot::GriffinBot() {
	// Inicializar las variables necesarias para ejecutar la partida
}

GriffinBot::~GriffinBot() {
	// Liberar los recursos reservados
}

void GriffinBot::initialize() {
	// Inicializar el bot antes de jugar una partida
}

string GriffinBot::getName() {
	return "GriffinBot";
}

Move GriffinBot::nextMove(const vector<Move>& adversary, const GameState& state) {
  Move next_move;
  Node* root = new Node(state, M_NONE, getPlayer() == J1);
  Bound first_guess = 0;

#if ITERATIVE_DEEPENING == 1

  for (int d = 1; d <= MAX_DEPTH; d++) {
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
