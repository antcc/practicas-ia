#include "../Comportamientos_Jugador/jugador.hpp"
#include "motorlib/util.h"

#include <iostream>
#include <cmath>
#include <vector>
#include <set>
#include <map>
using namespace std;

#define DEBUG 1
#define MAX_G 100000

/***************** ESTRUCTURAS AUXILIARES ******************/

struct comp_pos {
  bool operator()(node * n1, node * n2) const {
    return n1->pos < n2->pos;
  }
};

map <node*, int, comp_pos> f;

/***************** FUNCIONES AUXILIARES ******************/

namespace {

Orientation operator+(const Action& act, const Orientation& ori) {
  Orientation new_ori;
  int move;

  if (act == actTURN_L)
    move = -1;
  else if (act == actTURN_R)
    move = 1;

  switch(act) {
    case actFORWARD:
    case actIDLE:
      new_ori = ori;
      break;
    case actTURN_L:
    case actTURN_R:
      new_ori = static_cast<Orientation>((ori + move + 4) % 4);
      break;
  }
  return new_ori;
}

Orientation operator+(const Orientation& ori, const Action& act) {
  return act + ori;
}

int manhattan_distance(const estado& origen, const estado& destino) {
  return abs(origen.fila - destino.fila) + abs(origen.columna - destino.columna);
}

node * mknode(const estado& pos, const estado& destino, node * parent,
              const stack<Action>& act) {
  node* n = new node(parent, pos, manhattan_distance(pos, destino), act);
  return n;
}

template <typename T>
void delete_set(set<node*, T> l) {
  for (auto it = l.begin(); it != l.end(); ++it) {
    delete *it;
  }
}

void printn(node * n) {
  cout << "(" << n->pos.fila << "," << n->pos.columna << "): g="
       << n->g << ", h=" << n->h << ", f=" << f[n] << endl;
}

void reconstruct_path(stack<Action>& plan, node * goal) {
  node* current = goal;

  while (current) {
    stack<Action> partial_path = current->actions;
    while (!partial_path.empty()) {
      plan.push(partial_path.top());
      partial_path.pop();
    }
    current = current->parent;
  }
}

void AnularMatriz(vector<vector<unsigned char> >& m){
	for (int i=0; i<m[0].size(); i++){
		for (int j=0; j<m.size(); j++){
			m[i][j]=0;
		}
	}
}

void clear_stack(stack<Action>& st) {
  while (!st.empty())
    st.pop();
}

}

/***************** IMPLEMENTACIÓN DE MÉTODOS  *******************/

node::node(node * parent, const estado& pos, int h, stack<Action> actions) {
  this->g = MAX_G;
  this->parent = parent;
  this->pos = pos;
  this->h = h;
  this->actions = actions;
}

void ComportamientoJugador::PintaPlan(stack<Action> plan) {
	while (!plan.empty()){
    Action act = plan.top();
		if (act == actFORWARD){
			cout << "A ";
		}
		else if (act == actTURN_R){
			cout << "D ";
		}
		else if (act == actTURN_L){
			cout << "I ";
		}
		else {
			cout << "- ";
		}
		plan.pop();
	}
	cout << endl;
}

void ComportamientoJugador::actualizaPos(estado& pos, Action next) {
  if (next == actFORWARD) {
    switch(pos.orientacion) {
      case NORTH:
        pos.fila--;
        break;
      case SOUTH:
        pos.fila++;
        break;
      case EAST:
        pos.columna++;
        break;
      case WEST:
        pos.columna--;
        break;
    }
  }
  pos.orientacion = next + pos.orientacion;
}

bool ComportamientoJugador::esTransitable(int fil, int col) const {
  bool terreno = mapaResultado[fil][col] == 'S'
                 || mapaResultado[fil][col] == 'T'
                 || mapaResultado[fil][col] == 'K';
  return terreno;
}

/**
 * A* algorithm
 */
bool ComportamientoJugador::pathFinding(const estado& origen,
                                        const estado& destino,
                                        stack<Action>& plan,
                                        int& cost) {
  // Custom comparator for nodes
  struct comp_f {
    bool operator()(node * n1, node * n2) const {
      int diff = f[n1] - f[n2];
      if (n1->pos == n2->pos)
        return false;
      if (diff != 0)
        return diff < 0;
      return n1->pos < n2->pos;
    }
  };

  bool found = false;
  int cont = 0;
  set<node*, comp_f> open_set;
  set<node*, comp_pos> closed_set;
  node* neighbors[4];
  int next_cost[4];
  stack<Action> act;

#if DEBUG == 1
  cout << "Destino -> (" << destino.fila << "," << destino.columna << ")" << endl;
#endif

  // Start node
  node* current = mknode(origen, destino, NULL, act);
  current->g = 0;
  f[current] = current->h;
  open_set.insert(current);

  // Main loop
  while (!open_set.empty() && !found) {
    auto current_it = open_set.begin();
    current = *current_it;

#if DEBUG == 1
    cout << "Open -> ";
    printn(current);
#endif

    if (current->h == 0) { // h is the distance to the goal
      found = true;
      break;
    }

    closed_set.insert(current);
    open_set.erase(current_it);

    // Evaluate neighbors
    int valid = 0;
    estado pos = {current->pos.fila, current->pos.columna, current->pos.orientacion};
    estado tmp = pos;

    // Move forward
    actualizaPos(tmp, actFORWARD);
    if (esTransitable(tmp.fila, tmp.columna)) {
      act.push(actFORWARD);
      neighbors[valid] = mknode(tmp, destino, current, act);
      next_cost[valid++] = 1;
      clear_stack(act);
    }
    tmp = pos;

    // Move east
    actualizaPos(tmp, actTURN_R);
    actualizaPos(tmp, actFORWARD);
    if (esTransitable(tmp.fila, tmp.columna)) {
      act.push(actTURN_R);
      act.push(actFORWARD);
      neighbors[valid] = mknode(tmp, destino, current, act);
      next_cost[valid++] = 2;
      clear_stack(act);
    }
    tmp = pos;

    // Move west
    actualizaPos(tmp, actTURN_L);
    actualizaPos(tmp, actFORWARD);
    if (esTransitable(tmp.fila, tmp.columna)) {
      act.push(actTURN_L);
      act.push(actFORWARD);
      neighbors[valid] = mknode(tmp, destino, current, act);
      next_cost[valid++] = 2;
      clear_stack(act);
    }
    tmp = pos;

    // Move backwards (only useful for the first move)
    if (cont == 0) {
      actualizaPos(tmp, actTURN_R);
      actualizaPos(tmp, actTURN_R);
      actualizaPos(tmp, actFORWARD);
      if (esTransitable(tmp.fila, tmp.columna)) {
        act.push(actTURN_R);
        act.push(actTURN_R);
        act.push(actFORWARD);
        neighbors[valid] = mknode(tmp, destino, current, act);
        next_cost[valid++] = 3;
        clear_stack(act);
      }
      tmp = pos;
    }

    for (int i = 0; i < valid; i++) {
      if (!closed_set.count(neighbors[i])) {
        auto it = open_set.find(neighbors[i]);
        bool found_in_open = it != open_set.end();
        int tentative_g = current->g + next_cost[i];
        bool better = !found_in_open || tentative_g < (*it)->g;

        if (better) {
          neighbors[i]->g = tentative_g;
          f[neighbors[i]] = neighbors[i]->g + neighbors[i]->h;
        }

#if DEBUG == 1
      if (better) {
        cout << "Neighbor -> ";
        printn(neighbors[i]);
      }
#endif

        if (better) { // Update (or insert) node in open set
          if (found_in_open)
            open_set.erase(it);
          open_set.insert(neighbors[i]);
        }
        else {
          delete neighbors[i];
        }
      }
      else { // Already in closed set
        delete neighbors[i];
      }
    }
  cont++;
  }

  if (found) {
    cont++;  // Take into account the expansion of the goal node

#if DEBUG == 1
    cout << "---------- PATH FOUND ----------\n" << endl;
    cout << "Nodos abiertos: " << cont << endl;
#endif

    // Generate actual path and print it
    reconstruct_path(plan, current);
    VisualizaPlan(origen, plan);
    cost = current->g;
  }
  else {
    cost = -1;
  }

  // Free memory
  delete_set(open_set);
  delete_set(closed_set);

  return found;
}

Action ComportamientoJugador::think(Sensores sensores) {
  Action next_move;

  if (sensores.mensajeF != -1) {
    pos.fila = sensores.mensajeF;
    pos.columna = sensores.mensajeC;
  }

  actualizaPos(pos, ultimaAccion);

  if (hayPlan && (sensores.destinoF != destino.fila || sensores.destinoC != destino.columna)) {
    cout << "\nObjetivo cambiado." << endl;
    hayPlan = false;
    clear_stack(plan);
  }

  // Plan itinerary the first time
  if (!hayPlan) {
    destino.fila = sensores.destinoF;
    destino.columna = sensores.destinoC;
    if (!(hayPlan = pathFinding(pos, destino, plan, total_cost)))
      cout << "Error: no hay camino posible al destino." << endl;
    else
      cout << "Coste total del camino: " << total_cost << endl;
  }

  if (hayPlan && !plan.empty()) {
    next_move = plan.top();
    if (next_move == actFORWARD && sensores.superficie[2] == 'a')
      next_move = actIDLE;
    else
      plan.pop();
  }
  else {
    next_move = actIDLE;
  }

  ultimaAccion = next_move;

  return next_move;
}

void ComportamientoJugador::VisualizaPlan(const estado &st, stack<Action> plan){
  AnularMatriz(mapaConPlan);
	estado cst = st;
	while (!plan.empty()){
    Action next = plan.top();
		actualizaPos(cst, next);
    mapaConPlan[cst.fila][cst.columna] = 1;
		plan.pop();
	}
}

int ComportamientoJugador::interact(Action accion, int valor) {
  return false;
}
