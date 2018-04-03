#include "comportamientos/comportamiento.hpp"

Comportamiento::Comportamiento(unsigned int size){
  vector< unsigned char> aux(size, '?');
  for(unsigned int i = 0; i < size; i++){
    mapaResultado.push_back(aux);
    mapaEntidades.push_back(aux);
  }
}

Comportamiento::Comportamiento(vector< vector< unsigned char> > mapaR) {
  vector< unsigned char> aux(mapaR.size(), '?');
  for(unsigned int i = 0; i < mapaR.size(); i++){
    mapaEntidades.push_back(aux);
  }

  mapaResultado = mapaR;
}

Action Comportamiento::think(Sensores sensores){
  return actIDLE;
}

int Comportamiento::interact(Action accion, int valor){
  return 0;
}
