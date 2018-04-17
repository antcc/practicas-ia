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

  bool operator<(const estado& otro) const {
    if (fila != otro.fila)
      return fila < otro.fila;
    return columna < otro.columna;
  }

  bool operator==(const estado& otro) const {
    return !(*this < otro || otro < *this);
  }

  bool operator !=(const estado& otro) const {
    return !(*this == otro);
  }
};

struct node {
  int h;
  int g;
  estado pos;
  stack<Action> actions; // Actions performed to get from parent to this node
  node* parent;

  node(node * parent, const estado& pos, int h, stack<Action> actions);
};

class ComportamientoJugador : public Comportamiento {
  public:
    void constructor() {
      // Inicializar Variables de Estado
      pos.fila = pos.columna = 99;
      pos.orientacion = NORTH;
      destino.fila = -1;
      destino.columna = -1;
      destino.orientacion = NORTH; // Indiferente
      ultimaAccion = actIDLE;
      hayPlan = false;
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
    estado pos, destino;
    bool hayPlan;
    int total_cost; // Coste total del camino actual si hay plan
    Action ultimaAccion;
    stack<Action> plan;

    void actualizaPos(estado& pos, Action next);
    bool pathFinding(const estado &origen, const estado &destino, stack<Action> &plan, int& cost);

    void VisualizaPlan(const estado &st, stack<Action> plan);
    void PintaPlan(stack<Action> plan);
    bool esTransitable(int fil, int col) const;
};

#endif
