#include "../Comportamientos_Jugador/jugador.hpp"
#include "motorlib/util.h"

#include <iostream>
#include <cmath>
#include <vector>
#include <set>

using namespace std;

#define DEBUG 1
#define MAX_WAIT 10
#define DELTA 0.25

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

node * mknode(const estado& pos, const estado& destino, node * parent, const stack<Action>& act, int cost) {
  node* n = new node(parent, pos, manhattan_distance(pos, destino), act, cost);
  return n;
}

template<typename T>
void delete_set(set<node*, T> l) {
  for (auto it = l.begin(); it != l.end(); ++it) {
    delete *it;
  }
}

int f(node * n) {
  return n->g + n->h;
}

void printn(node * n) {
  cout << "(" << n->pos.fila << "," << n->pos.columna << "): g=" << n->g << ", h=" << n->h << ", f=" << f(n) << endl;
}

int reconstruct_path(stack<Action>& plan, node * goal) {
  node* current = goal;
  int cost = 0;

  while (current) {
    stack<Action> partial_path = current->actions;
    while (!partial_path.empty()) {
      plan.push(partial_path.top());
      partial_path.pop();
    }
    current = current->parent;
    cost++;
  }

  return cost;
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

/***************** MÉTODOS DE LA CLASE *******************/

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

bool ComportamientoJugador::esTransitable(int fil, int col) {
  bool terreno = mapaResultado[fil][col] == 'S'
                || mapaResultado[fil][col] == 'T'
                || mapaResultado[fil][col] == 'K';

  if (aldeano != estado{-1, -1})
    return terreno && aldeano != estado{fil, col};
  else
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
  struct comp_pos {
    bool operator()(node * n1, node * n2) {
      if (n1->pos.fila < n2->pos.fila)
        return true;
      else if (n1->pos.fila > n2->pos.fila)
        return false;

      return n1->pos.columna < n2->pos.columna;
    }
  };

  struct comp_f {
    bool operator()(node * n1, node * n2) {
      comp_pos p;
      int diff = f(n1) - f(n2);
      if (diff == 0)
        return p(n1, n2);
      else
        return diff < 0;
    }
  };

  bool found = false;
  int cont = 0;
  set<node*, comp_f> open_set;
  set<node*, comp_pos> closed_set;
  node* neighbors[4];
  stack<Action> act;

#if DEBUG == 1
  cout << "Destino -> (" << destino.fila << "," << destino.columna << ")" << endl;
#endif

  // Start node
  node* current = mknode(origen, destino, NULL, act, 0);
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
      neighbors[valid++] = mknode(tmp, destino, current, act, 1);
      clear_stack(act);
    }
    tmp = pos;

    // Move east
    actualizaPos(tmp, actTURN_R);
    actualizaPos(tmp, actFORWARD);
    if (esTransitable(tmp.fila, tmp.columna)) {
      act.push(actTURN_R);
      act.push(actFORWARD);
      neighbors[valid++] = mknode(tmp, destino, current, act, 2);
      clear_stack(act);
    }
    tmp = pos;

    // Move west
    actualizaPos(tmp, actTURN_L);
    actualizaPos(tmp, actFORWARD);
    if (esTransitable(tmp.fila, tmp.columna)) {
      act.push(actTURN_L);
      act.push(actFORWARD);
      neighbors[valid++] = mknode(tmp, destino, current, act, 2);
      clear_stack(act);
    }
    tmp = pos;

    for (int i = 0; i < valid; i++) {
      if (closed_set.find(neighbors[i]) == closed_set.end()) {
        auto it = open_set.find(neighbors[i]);

  #if DEBUG == 1
        cout << "Neighbor -> ";
        printn(neighbors[i]);
  #endif

        if (it == open_set.end()) {
          open_set.insert(neighbors[i]);
        }
        else {
          if (neighbors[i]->g < (*it)->g) {
            open_set.erase(it);
            open_set.insert(neighbors[i]);
          }
        }
      }
      else {
        delete neighbors[i];
      }
    }
  cont++;
  }

  if (found) {

#if DEBUG == 1
    cout << "---------- PATH FOUND ----------\n" << endl;
    cout << "Nodos abiertos: " << cont << endl;
#endif

    // Generate actual path and print it
    cost = reconstruct_path(plan, current);
    VisualizaPlan(origen, plan);
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
  stack<Action> plan_alternativo;
  int cost;
  bool waiting = false;

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
  }

  /**
   * Comportamiento reactivo:
   *
   * - Si hay un aldeano delante, me quedo quieto.
   * - Si tras un número fijo de movimientos (MAX_WAIT) sigue ahí, planifico
   * nueva ruta.
   *
   * - Si la nueva ruta aumenta el coste anterior más de un factor fijado
   * (DELTA), la descarto y sigo quieto hasta que se vaya el aldeano
   */

  // Hay un aldeano delante
  if (hayPlan && !plan.empty() && sensores.superficie[2] == 'a') {
    wait++;
    next_move = actIDLE;

    if (wait >= MAX_WAIT && !tried_alternative) {
      tried_alternative = true;
      aldeano.fila = pos.fila;
      aldeano.columna = pos.columna;
      if ((hayPlan = pathFinding(pos, destino, plan_alternativo, cost))
          && cost < (1 + DELTA) * total_cost) {
        plan.swap(plan_alternativo);
        clear_stack(plan_alternativo);
        ignore_villager = false;
      }
      aldeano.fila = -1;
      aldeano.columna = -1;
    }
  }

  if (hayPlan && !plan.empty()
      && (!ignore_villager || sensores.superficie[2] != 'a')) {
    tried_alternative = false;
    ignore_villager = true;
    wait = 0;
    next_move = plan.top();
    plan.pop();
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
