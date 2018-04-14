#include "../Comportamientos_Jugador/jugador.hpp"
#include "motorlib/util.h"

#include <iostream>
#include <cmath>
#include <queue>
#include <vector>

using namespace std;

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

int f(node* n) {
  return n->h + n->g;
}

int manhattan_distance(const estado& origen, const estado& destino) {
  return abs(origen.fila - destino.fila) + abs(origen.columna - destino.columna);
}

node * mknode(const estado& pos, const estado& destino, node * parent) {
  int parent_g;
  if (parent)
    parent_g = parent->g;
  else
    parent_g = -1;

  node* n = new node(parent, parent_g, pos);
  n->h = manhattan_distance(pos, destino);
  return n;
}

void delete_list(list<node*> l) {
  for (auto it = l.begin(); it != l.end(); ++it)
    delete *it;
}

void reconstruct_path(stack<Action>& plan, node * destination) {
  node* current = destination;

  while (current) {
    stack<Action> partial_path = current->actions;
    while (!partial_path.empty()) {
      plan.push(partial_path.top());
      partial_path.pop();
    }
    current = current->parent;
  }
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

/**
 * A* algorithm
 */
bool ComportamientoJugador::pathFinding(const estado& origen,
                                        const estado& destino,
                                        stack<Action>& plan) {
  // Custom comparator for nodes
  struct comp_nodes {
    bool operator()(node * n1, node * n2) {
      return f(n1) < f(n2);
    }
  };

  bool found = false;
  priority_queue<node*, vector<node*>, comp_nodes> open_set;
  list<node*> closed_set;

  // Start node
  open_set.push(mknode(origen, destino, NULL));


  // if(llegado a destino) found=true



  // Generate actual path
  reconstruct_path(plan, closed_set.front());

  // Free memory
  while (!open_set.empty()) {
    delete open_set.top();
    open_set.pop();
  }
  delete_list(closed_set);

  return found;
}

Action ComportamientoJugador::think(Sensores sensores) {
  /**
   * Asumo que el juego se detiene al llegar al destino.
   *
   */
  Action next_move;

  if (pasos == 0) {
    pos.fila = sensores.mensajeF;
    pos.columna = sensores.mensajeC;
    destino.fila = sensores.destinoF;
    destino.columna = sensores.destinoC;
  }

  // Plan itinerary the first time
  if (plan.empty()) {
    if (!pathFinding(pos, destino, plan))
      cout << "Error: no hay camino posible al destino." << endl;
  }

  //next_move = plan.top();
  //plan.pop();

  actualizaPos(pos, next_move);
  pasos++;

  return actIDLE;
}

void AnularMatriz(vector<vector<unsigned char> > &m){
	for (int i=0; i<m[0].size(); i++){
		for (int j=0; j<m.size(); j++){
			m[i][j]=0;
		}
	}
}

void ComportamientoJugador::VisualizaPlan(const estado &st, const list<Action> &plan){
  AnularMatriz(mapaConPlan);
	estado cst = st;

	auto it = plan.begin();
	while (it!=plan.end()){
		if (*it == actFORWARD){
			switch (cst.orientacion) {
				case 0: cst.fila--; break;
				case 1: cst.columna++; break;
				case 2: cst.fila++; break;
				case 3: cst.columna--; break;
			}
			mapaConPlan[cst.fila][cst.columna]=1;
		}
		else if (*it == actTURN_R){
			cst.orientacion = (cst.orientacion+1)%4;
		}
		else {
			cst.orientacion = (cst.orientacion+3)%4;
		}
		it++;
	}
}

int ComportamientoJugador::interact(Action accion, int valor) {
  return false;
}
