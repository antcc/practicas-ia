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
};

struct node {
  int h;
  int g;
  estado pos;
  stack<Action> actions; // Actions performed to get from parent to this node
  node* parent;

  node(node * parent, int g_parent, const estado& pos) {
    this->parent = parent;
    g = g_parent + 1;
    this->pos = pos;
  }
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
      pasos = 0;
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
    int pasos;
    stack<Action> plan;

    void actualizaPos(estado& pos, Action next);
    bool pathFinding(const estado &origen, const estado &destino, stack<Action> &plan);
    void PintaPlan(stack<Action> plan);
};

#endif
