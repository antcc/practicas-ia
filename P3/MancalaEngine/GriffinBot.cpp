/*
 * GriffinBot.h
 *
 * Autor: Antonio Coín Castro
 */

#include "GriffinBot.h"
#include <string>
#include <cstdlib>
#include <iostream>

using namespace std;

GriffinBot::GriffinBot() {
	// Inicializar las variables necesarias para ejecutar la partida
}

GriffinBot::~GriffinBot() {
	// Liberar los recursos reservados (memoria, ficheros, etc.)
}

void GriffinBot::initialize() {
	// Inicializar el bot antes de jugar una partida
}

string GriffinBot::getName() {
	return "GriffinBot"; // Sustituir por el nombre del bot
}

Move GriffinBot::nextMove(const vector<Move>& adversary, const GameState& state) {

	Move movimiento = M_NONE;

	return movimiento;
}
