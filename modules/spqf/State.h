/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */


enum class State : uint
{
	PENDING      = 0,
	PREJUDICED,
	PASSED,
	CANCELED,
	VETOED,
	QUORUM,
	PLURALITY,
};


inline
bool passed(const State &state)
{
	switch(state)
	{
		case State::PASSED:
			return true;

		default:
			return false;
	}
}


inline
bool failed(const State &state)
{
	switch(state)
	{
		case State::PENDING:
		case State::PREJUDICED:
		case State::PASSED:
			return false;

		default:
			return true;
	}
}


inline
bool pending(const State &state)
{
	switch(state)
	{
		case State::PENDING:
		case State::PREJUDICED:
			return true;

		default:
			return false;
	}
}


static
const char *reflect(const State &state)
{
	switch(state)
	{
		case State::PENDING:     return "PENDING";
		case State::PREJUDICED:  return "PREJUDICED";
		case State::PASSED:      return "PASSED";
		case State::CANCELED:    return "CANCELED";
		case State::VETOED:      return "VETOED";
		case State::QUORUM:      return "QUORUM";
		case State::PLURALITY:   return "PLURALITY";
		default:                 return "??????";
	};
}


static
const char *describe(const State &state)
{
	switch(state)
	{
		case State::PENDING:     return "Result is still pending.";
		case State::PREJUDICED:  return "Still pending but effects applied prejudicially.";
		case State::PASSED:      return "Approved successfully.";
		case State::CANCELED:    return "Canceled by speaker.";
		case State::VETOED:      return "Rejected due to veto.";
		case State::QUORUM:      return "Failed to meet required participation.";
		case State::PLURALITY:   return "Failed to meet required majority.";
		default:                 return "No description available.";
	};
}
