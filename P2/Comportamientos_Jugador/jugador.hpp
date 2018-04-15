#ifndef COMPORTAMIENTOJUGADOR_H
#define COMPORTAMIENTOJUGADOR_H

#include "comportamientos/comportamiento.hpp"
#include <stack>
#include <list>

// 0=Norte, 1=Este, 2=Sur, 3=Oeste
enum Orientation {NORTH, EAST, SOUTH, WEST};

struct estado {
  int fila;
  int columna;
  Orientation orientacion;

  bool operator==(const estado& otro) {
    return fila == otro.fila && columna == otro.columna;
  }

  bool operator !=(const estado& otro) {
    return !(*this == otro);
  }
};

struct node {
  int h;
  int g;
  estado pos;
  stack<Action> actions; // Actions performed to get from parent to this node
  node* parent;

  node(node * parent, const estado& pos, int h, stack<Action> actions, int cost) {
    if (parent)
      g = parent->g + cost;
    else
      g = 0;
    this->parent = parent;
    this->pos = pos;
    this->h = h;
    this->actions = actions;
  }
};

class ComportamientoJugador : public Comportamiento {
  public:

    void constructor() {
      // Inicializar Variables de Estado
      pos.fila = pos.columna = 99;
      pos.orientacion = NORTH;
      destino.fila = aldeano.fila = -1;
      destino.columna = aldeano.columna = -1;
      destino.orientacion = aldeano.orientacion = NORTH; // Indiferente
      ultimaAccion = actIDLE;
      hayPlan = tried_alternative = false;
      ignore_villager = true;
      total_cost = -1;
      wait = 0;
    }

    ComportamientoJugador(unsigned int size) : Comportamiento(size) {
      constructor();
    }
    ComportamientoJugador(std::vector< std::vector< unsigned char> > mapaR) : Comportamiento(mapaR) {
      constructor();
    }
    ComportamientoJugador(const ComportamientoJugador & comport) : Comportamiento(comport) {}
    ~ComportamientoJugador() {}

    Action think(Sensores sensores);
    int interact(Action accion, int valor);
    ComportamientoJugador * clone() {return new ComportamientoJugador(*this);}

  private:
    // Declarar Variables de Estado
    estado pos, destino, aldeano;
    bool hayPlan, tried_alternative, ignore_villager;
    int total_cost; // Coste total del camino actual si hay plan
    int wait; // Número de turnos sin hacer nada
    Action ultimaAccion;
    stack<Action> plan;

    void actualizaPos(estado& pos, Action next);
    bool pathFinding(const estado &origen, const estado &destino, stack<Action> &plan, int& cost);

    void VisualizaPlan(const estado &st, stack<Action> plan);
    void PintaPlan(stack<Action> plan);
    bool esTransitable(int fil, int col);
};

#endif
