/*
 * GriffinBot.h
 *
 * Autor: Antonio Coín Castro
 */

#include "GriffinBot.h"
#include <string>
#include <iostream>
#include <algorithm>

using namespace std;
using Heuristic = Node::Heuristic;

#define INFINITY 1 << 10
#define DEBUG 0
#define ITERATIVE_DEEPENING 1
#define MAX_DEPTH 12
#define MOVE_ORDERING 0

/*****
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

* EN EL CUTOFF-TEST HAY QUE TENER EN CUENTA
* EL TIEMPO MÁXIMO.

* USAR NEGASCOUT????

* QUIESCENCE SEARCH (https://en.wikipedia.org/wiki/Quiescence_search)

* Tener en cuenta en la heurística que en la práctica con 25 fichas has ganado.

* LOS DELETES!!

* Tabla hash. considerar guardar el orden de los nodos para no tener que volver
* a ordenar los hijos ya explorados. (best_move)
The move-ordering sort was based on four factors (in order of precedence): transposition-table suggestions,
extra turns, captures, and right-to-left default ordering

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*******/

namespace {
  void print_table(const TTable& t) {
    NodeHash hash;
    for (auto entry : t) {
      cerr << "Hash=" << hash(entry.first) << ", Bound=" << entry.second.value
           << ", Age=" << entry.second.age << endl;
    }
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
  for (int i = 1; i <= NUM_PITS; i++) {
    GameState new_board = board.simulateMove((Move) i);
    bool maximizing = new_board.getCurrentPlayer() == board.getCurrentPlayer()
                      ? is_maximizing : !is_maximizing;
    l[i-1] = new Node(new_board, (Move) i, maximizing, h);
  }

#if MOVE_ORDERING == 1

  NodeOrder comp;
  sort(l.begin(), l.end(), comp);

#endif

  return l;
}

/**
 * Delete remaining children of current node
 */

void Node::deleteChildren(NodeList& children, int begin) {
  for (int i = begin; i < NUM_PITS; i++) {
    delete children[i];
    children[i] = 0;
  }
}

/**
 * Custom ordering for nodes.
 * The ordering is determined by the heuristic value of the node.
 */
bool NodeOrder::operator()(Node * n1, Node * n2) const {
  return n1->h_value > n2->h_value;
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

  for (int i = 0; i <= NUM_PITS; i++) {
    node_str += n->board.getSeedsAt(p, (Position) i);
    node_str += n->board.getSeedsAt(other, (Position) i);
  }

  return hash<string>()(node_str);
}

/*************************************************
*                    GRIFFINBOT                  *
**************************************************/

/**
 * Implementation of memory-enhanced alpha-beta pruning
 */
pair<Bound, Move>
GriffinBot::alphaBetaWithMemory(Node * node, int depth, int alpha, int beta) {
  Bound best_bound;
  Move best_move;
  NodeInfo entry;
  bool table_hit = false;
  auto entry_it = table.find(node);

  if (entry_it != table.end()) { // hit
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

    // Move ordering
    if (table_hit && entry.best_move != M_NONE) {
      int i = 0;
      while (children[i]->prev_move != entry.best_move)
        i++;

      if (i > 0)
        swap(children[0], children[i]);
    }

    for (int i = 0; i < NUM_PITS; i++) {
      if (best_bound >= beta)
        break;

      auto child = children[i];
      auto child_bound = alphaBetaWithMemory(child, depth - 1, a, beta).first;

#if DEBUG == 1

      cerr << "MAX " << ", Depth " << depth << ", Move " << child->prev_move << ", Heuristic "
           << child->h_value << endl;

#endif

      if (child_bound > best_bound) {
        best_bound = child_bound;
        best_move = child->prev_move;
      }

      a = max(a, best_bound);
    }
  }

  else {// minimizing
    best_bound = INFINITY;
    int b = beta;
    auto children = node->children();

    // Move ordering
    if (table_hit && entry.best_move != M_NONE) {
      int i = 0;
      while (children[i]->prev_move != entry.best_move)
        i++;

      if (i > 0)
        swap(children[0], children[i]);
    }

    for (int i = 0; i < NUM_PITS; i++) {
      if (best_bound <= alpha)
        break;

      auto child = children[i];

      auto child_bound = alphaBetaWithMemory(child, depth - 1, alpha, b).first;

#if DEBUG == 1

      cerr << "MAX " << ", Depth " << depth << ", Move " << child->prev_move << ", Heuristic "
           << child->h_value << endl;

#endif

      if (child_bound < best_bound) {
        best_bound = child_bound;
        best_move = child->prev_move;
      }

      b = min(b, best_bound);
    }
  }

  if (best_bound <= alpha) {
    if (table_hit)
      best_move = entry.best_move;
    table[node] = {depth, num_moves, false, best_bound, best_move};
  }

  if (best_bound >= beta) {
    table[node] = {depth, num_moves, true, best_bound, best_move};
  }

  return make_pair(best_bound, best_move);
}

/**
 * Implementation of standard alpha-beta pruning
 */
pair<Bound, Move>
GriffinBot::alphaBeta(Node * node, int depth, int alpha, int beta) {
  if (depth == 0 || node->board.isFinalState())
    return make_pair(node->h_value, M_NONE);

  Bound best_bound;
  Move best_move;
  auto children = node->children();

  if (node->is_maximizing) {
    best_bound = -INFINITY;

    for (int i = 0; i < NUM_PITS; i++) {
      auto child = children[i];
      auto child_bound = alphaBeta(child, depth - 1, alpha, beta).first;

#if DEBUG == 1

      cerr << "MAX " << ", Depth " << depth << ", Move " << child->prev_move << ", Heuristic "
           << child->h_value << endl;

#endif

      if (child_bound > best_bound) {
        best_bound = child_bound;
        best_move = child->prev_move;
      }

      alpha = max(alpha, best_bound);
      delete child;

      if (beta <= alpha) {
        node->deleteChildren(children, i+1);
        break;
      }
    }

    return make_pair(best_bound, best_move);
  }

  else {  // minimizing_player
    best_bound = INFINITY;

    for (int i = 0; i < NUM_PITS; i++) {
      auto child = children[i];
      auto child_bound = alphaBeta(child, depth - 1, alpha, beta).first;

#if DEBUG == 1

      cerr << "MIN " << ", Depth " << depth << ", Move " << child->prev_move << ", Heuristic "
           << child->h_value << endl;

#endif

      if (child_bound < best_bound) {
        best_bound = child_bound;
        best_move = child->prev_move;
      }

      beta = min(beta, best_bound);
      delete child;

      if (beta <= alpha) {
        node->deleteChildren(children, i+1);
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
pair<Bound, Move>
GriffinBot::mtdf(Node * root, Bound first_guess, int depth) {
  pair<Bound, Move> best_action;
  Bound best_bound = first_guess;
  int upper_bound = INFINITY;
  int lower_bound = -INFINITY;

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

GriffinBot::GriffinBot() {
  num_moves = 0;
}

GriffinBot::~GriffinBot() {
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

#if ITERATIVE_DEEPENING == 1

  for (int d = 2; d <= MAX_DEPTH; d += 2) {
    auto solution = mtdf(root, first_guess, d);
    first_guess = solution.first;
    next_move = solution.second;
  }

#else

  next_move = mtdf(root, first_guess, MAX_DEPTH).second;

#endif

  num_moves++;

	return next_move;
}
