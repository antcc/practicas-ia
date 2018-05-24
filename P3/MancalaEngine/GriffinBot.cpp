/*
 * GriffinBot.h
 *
 * Autor: Antonio Coín Castro
 */

#include "GriffinBot.h"
#include <string>
#include <iostream>
#include <algorithm>
#include <chrono>

using namespace std;
using namespace chrono;
using Heuristic = Node::Heuristic;

#define DEBUG 0
#define ITERATIVE_DEEPENING 1
#define MOVE_ORDERING 1

#define INFINITY 1024
#define TIMEOUT 666
#define MAX_DEPTH 40
#define MAX_TIME_SPAN 1.95 * 1e3  // milliseconds

int main() __attribute__((optimize("-O2")));

/*****
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

* Optimizar:
  - La heurística
  - No limpiar la tabla entre iteraciones???
*
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*******/

namespace {
  void delete_table(TTable& table) {
    for (auto entry : table)
      delete entry.first;
  }
}

/*************************************************
*                   HEURISTIC                    *
**************************************************/

Player Heuristic::player;

Heuristic::Heuristic() {
}

Heuristic::Heuristic(Player p) {
  player = p;
}

Bound Heuristic::heuristic(Node * n) const {
  Player rival = player == J1 ? J2 : J1;
  return n->board.getScore(player) - n->board.getScore(rival);
}

/*************************************************
*                      NODE                      *
**************************************************/

Heuristic Node::h;

Node::Node(const GameState& board, Move prev_move, bool is_maximizing, const Heuristic& h) {
  this->h = h;
  this->board = board;
  this->prev_move = prev_move;
  this->is_maximizing = is_maximizing;
  this->h_value = h.heuristic(this);
}

/**
 * Return an ordered list of children of the current node.
 * @see NodeOrder
 */
NodeList Node::children() {
  NodeList l;

  // Unroll loop
  GameState new_board = board.simulateMove((Move) 1);
  bool maximizing = new_board.getCurrentPlayer() == board.getCurrentPlayer()
                    ? is_maximizing : !is_maximizing;
  l[0] = new Node(new_board, (Move) 1, maximizing, h);

  new_board = board.simulateMove((Move) 2);
  maximizing = new_board.getCurrentPlayer() == board.getCurrentPlayer()
                    ? is_maximizing : !is_maximizing;
  l[1] = new Node(new_board, (Move) 2, maximizing, h);

  new_board = board.simulateMove((Move) 3);
  maximizing = new_board.getCurrentPlayer() == board.getCurrentPlayer()
                    ? is_maximizing : !is_maximizing;
  l[2] = new Node(new_board, (Move) 3, maximizing, h);

  new_board = board.simulateMove((Move) 4);
  maximizing = new_board.getCurrentPlayer() == board.getCurrentPlayer()
                    ? is_maximizing : !is_maximizing;
  l[3] = new Node(new_board, (Move) 4, maximizing, h);

  new_board = board.simulateMove((Move) 5);
  maximizing = new_board.getCurrentPlayer() == board.getCurrentPlayer()
                    ? is_maximizing : !is_maximizing;
  l[4] = new Node(new_board, (Move) 5, maximizing, h);

  new_board = board.simulateMove((Move) 6);
  maximizing = new_board.getCurrentPlayer() == board.getCurrentPlayer()
                    ? is_maximizing : !is_maximizing;
  l[5] = new Node(new_board, (Move) 6, maximizing, h);

#if MOVE_ORDERING == 1

  NodeOrder order(is_maximizing);
  sort(l.begin(), l.end(), order);

#endif

  return l;
}

/**
 * Returns true if the node produces an extra turn.
 */
bool Node::hasExtraTurn(Node * parent) {
  return parent->is_maximizing == is_maximizing;
}

/**
 * Constructor for NodeOrder
 */
NodeOrder::NodeOrder(bool is_parent_maximizing) {
  this->is_parent_maximizing = is_parent_maximizing;
}

/**
 * Custom ordering for nodes.
 * The ordering is determined by the heuristic value of the node.
 */
bool NodeOrder::operator()(Node * n1, Node * n2) const {
  int diff = n1->h_value - n2->h_value;

  if (is_parent_maximizing)
    return diff > 0;
  else
    return diff < 0;
}

/**
 * Custom comparator for nodes
 * Two nodes are considered equal if they represent the same
 * state in the game.
 */
bool NodeComp::operator()(Node * n1, Node * n2) const {
  Player p1 = n1->board.getCurrentPlayer();
  Player p2 = n2->board.getCurrentPlayer();

  if (p1 != p2)
    return false;

  Player other = p1 == J1 ? J2 : J1;

  for (int i = 0; i <= NUM_PITS; i++) {
    if (n1->board.getSeedsAt(p1, (Position) i)
        != n2->board.getSeedsAt(p1, (Position) i))
      return false;
    if (n1->board.getSeedsAt(other, (Position) i)
        != n2->board.getSeedsAt(other, (Position) i))
      return false;
  }

  return true;
}

/**
 * Custom hash function for nodes.
 */
size_t NodeHash::operator()(Node * n) const {
  Player p = n->board.getCurrentPlayer();
  Player other = p == J1 ? J2 : J1;
  string node_str = p == J1 ? "J1" : "J2";

  // Unroll loop
  node_str += n->board.getSeedsAt(p, (Position) 0);
  node_str += n->board.getSeedsAt(other, (Position) 0);
  node_str += n->board.getSeedsAt(p, (Position) 1);
  node_str += n->board.getSeedsAt(other, (Position) 1);
  node_str += n->board.getSeedsAt(p, (Position) 2);
  node_str += n->board.getSeedsAt(other, (Position) 2);
  node_str += n->board.getSeedsAt(p, (Position) 3);
  node_str += n->board.getSeedsAt(other, (Position) 3);
  node_str += n->board.getSeedsAt(p, (Position) 4);
  node_str += n->board.getSeedsAt(other, (Position) 4);
  node_str += n->board.getSeedsAt(p, (Position) 5);
  node_str += n->board.getSeedsAt(other, (Position) 5);
  node_str += n->board.getSeedsAt(p, (Position) 6);
  node_str += n->board.getSeedsAt(other, (Position) 6);

  return hash<string>{}(node_str);
}

/*************************************************
*                    GRIFFINBOT                  *
**************************************************/

/**
 * Implementation of memory-enhanced alpha-beta pruning
 */
BoundAndMove
GriffinBot::alphaBetaWithMemory(Node * node, int depth, int alpha, int beta) {
  Bound best_bound;
  Move best_move;
  NodeInfo entry;
  bool table_hit = false;

  // Check time
  auto end = high_resolution_clock::now();
  auto time_span = duration_cast<milliseconds> (end - begin);

  if (time_span.count() >= MAX_TIME_SPAN)
    return make_pair(TIMEOUT, M_NONE);

  auto entry_it = table.find(node);

  if (entry_it != table.end()) { // Table hit
    entry = (*entry_it).second;

    if (entry.depth >= depth && entry.age == num_moves) {
      table_hit = true;

      if (entry.is_lowerbound && entry.value > alpha)
        alpha = entry.value;
      else if (!entry.is_lowerbound && entry.value < beta)
        beta = entry.value;
      if (alpha >= beta)
        return make_pair(entry.value, entry.best_move);
    }
  }

  if (depth == 0 || node->board.isFinalState()) {
    best_bound = node->h_value;
    best_move = M_NONE;
  }

  else if (node->is_maximizing) {
    best_bound = -INFINITY;
    int a = alpha;
    auto children = node->children();

#if MOVE_ORDERING == 1

    if (table_hit && entry.best_move != M_NONE) {
      int i = 0;
      while (children[i]->prev_move != entry.best_move)
        i++;

      if (i > 0) {
        swap(children[0], children[i]);
        NodeOrder order(node->is_maximizing);
        sort(next(children.begin()), children.end(), order);
      }
    }

#endif

    for (int i = 0; i < NUM_PITS; i++) {
      auto child = children[i];
      auto child_bound = alphaBetaWithMemory(child, depth - 1, a, beta).first;

      if (child_bound == TIMEOUT)
        return make_pair(TIMEOUT, M_NONE);

      if (child_bound > best_bound) {
        best_bound = child_bound;
        best_move = child->prev_move;
      }

      a = max(a, best_bound);

      if (best_bound >= beta)
        break;
    }
  }

  else {  // minimizing
    best_bound = INFINITY;
    int b = beta;
    auto children = node->children();

#if MOVE_ORDERING == 1

    if (table_hit && entry.best_move != M_NONE) {
      int i = 0;
      while (children[i]->prev_move != entry.best_move)
        i++;

      if (i > 0) {
        swap(children[0], children[i]);
        NodeOrder order(node->is_maximizing);
        sort(next(children.begin()), children.end(), order);
      }
    }

#endif

    for (int i = 0; i < NUM_PITS; i++) {
      auto child = children[i];
      auto child_bound = alphaBetaWithMemory(child, depth - 1, alpha, b).first;

      if (child_bound == TIMEOUT)
        return make_pair(TIMEOUT, M_NONE);

      if (child_bound < best_bound) {
        best_bound = child_bound;
        best_move = child->prev_move;
      }

      b = min(b, best_bound);

      if (best_bound <= alpha)
        break;
    }
  }

  if (best_bound <= alpha) {
    table[node] = {depth, num_moves, false, best_bound, table_hit ? entry.best_move : M_NONE};
  }

  if (best_bound >= beta) {
    table[node] = {depth, num_moves, true, best_bound, best_move};
  }

  if (best_bound > alpha && best_bound < beta)
    delete node;

  return make_pair(best_bound, best_move);
}

/**
 * Implementation of MTD-f search algorithm
 *
 */
BoundAndMove
GriffinBot::mtdf(Node * root, Bound first_guess, int depth) {
  pair<Bound, Move> best_action;
  Bound best_bound = first_guess;
  int upper_bound = INFINITY;
  int lower_bound = -INFINITY;

  while (lower_bound < upper_bound) {
    int beta = max(best_bound, lower_bound + 1);
    best_action = alphaBetaWithMemory(root, depth, beta - 1, beta);
    best_bound = best_action.first;

    // Check time
    if (best_bound == TIMEOUT)
      return best_action;

    // Update current bounds
    if (best_bound < beta)
      upper_bound = best_bound;
    else
      lower_bound = best_bound;
  }

  return best_action;
}

GriffinBot::GriffinBot() {
  num_moves = 0;
}

GriffinBot::~GriffinBot() {
  delete_table(table);
}

void GriffinBot::initialize() {
}

string GriffinBot::getName() {
	return "GriffinBot";
}

Move GriffinBot::nextMove(const vector<Move>& adversary, const GameState& state) {
  static Heuristic heuristic(getPlayer());
  Move next_move;
  Bound first_guess = 0;
  Node* root = new Node(state, M_NONE, true, heuristic);

  begin = high_resolution_clock::now();

#if ITERATIVE_DEEPENING == 1

  for (int d = 1; d <= MAX_DEPTH; d++) {
    auto solution = mtdf(root, first_guess, d);

    // Check time
    if (solution.first == TIMEOUT)
      break;

    next_move = solution.second;
    first_guess = solution.first;

#if DEBUG == 1

    cerr << "Profundidad: " << d << "\nBound: " << solution.first << "\nMovimiento: " << solution.second << endl;

#endif

  }

#else

  next_move = mtdf(root, first_guess, MAX_DEPTH).second;

#endif

  auto end = high_resolution_clock::now();
  auto time_span = duration_cast<milliseconds> (end - begin);

  cerr << "Tiempo total del movimiento: " << time_span.count() / 1e3 << endl;

  delete root;
  num_moves++;

	return next_move;
}
