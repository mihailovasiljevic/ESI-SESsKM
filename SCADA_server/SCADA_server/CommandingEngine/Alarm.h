#pragma once
#ifndef ALRARM_STATE_H
#include "State.h"
#include  "TurnedOff.h"
#include "TurnedOn.h"

class AlarmState : public State {
	void alarm(Context *c);
};
#endif // !TURNED_ON_H