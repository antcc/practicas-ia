/*
 * GriffinBot.h
 *
 * Autor: Antonio Coín Castro
 */

#include "Bot.h"

#ifndef GRIFFINBOT_H_
#define GRIFFINBOT_H_

class GriffinBot : Bot {
public:
	GriffinBot();
	~GriffinBot();

	void initialize();
	string getName();
	Move nextMove(const vector<Move>& adversary, const GameState& state);
};

#endif /* GRIFFINBOT_H_ */
