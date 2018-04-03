#include "motorlib.hpp"

int main(int argc, char ** argv) {
  if (argc < 2){
    cout << "Faltan parametros..." << endl;
    cout << "Se necesita indicar: " << endl;
    cout << "1) Semilla" << endl;
  }
  else {
    srand(atoi(argv[1]));
    lanzar_motor_grafico(argc, argv);
  }

  exit(EXIT_SUCCESS);
}
