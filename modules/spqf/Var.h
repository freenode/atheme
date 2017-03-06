/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */


enum class Var : uint
{
	ENABLE,
	DURATION,
	EFFECTIVE,
	AUDIBLES,
	PREJUDICE,
	QUICK,
	MODE,
	ACCESS,
	BALLOT,
	QUORUM,
	PLURALITY,
	TURNOUT,
	BALLOTS,
	USER,
	TYPE,
	CHAN,
	RETRY_QUORUM,
	RETRY_VETOED,
	RETRY_PLURALITY,
	TALLY,
	YEAS,
	NAYS,
	VETOES,
	PENDING,
	YEA,
	NAY,
	THRESHOLD,
	STARTED,
	EXPIRED,
	CANCELED,
	PASSED,
	FAIL_QUORUM,
	FAIL_PLURALITY,
	FAIL_VETOED,


	_NUM_
};


inline
const char *reflect(const Var &var)
{
	switch(var)
	{
		case Var::ENABLE:                 return "ENABLE";
		case Var::DURATION:               return "DURATION";
		case Var::EFFECTIVE:              return "EFFECTIVE";
		case Var::AUDIBLES:               return "AUDIBLES";
		case Var::PREJUDICE:              return "PREJUDICE";
		case Var::QUICK:                  return "QUICK";
		case Var::MODE:                   return "MODE";
		case Var::ACCESS:                 return "ACCESS";
		case Var::BALLOT:                 return "BALLOT";
		case Var::QUORUM:                 return "QUORUM";
		case Var::PLURALITY:              return "PLURALITY";
		case Var::TURNOUT:                return "TURNOUT";
		case Var::BALLOTS:                return "BALLOTS";
		case Var::USER:                   return "USER";
		case Var::TYPE:                   return "TYPE";
		case Var::CHAN:                   return "CHAN";
		case Var::RETRY_QUORUM:           return "RETRY_QUORUM";
		case Var::RETRY_VETOED:           return "RETRY_VETOED";
		case Var::RETRY_PLURALITY:        return "RETRY_PLURALITY";
		case Var::TALLY:                  return "TALLY";
		case Var::YEAS:                   return "YEAS";
		case Var::NAYS:                   return "NAYS";
		case Var::VETOES:                 return "VETOES";
		case Var::PENDING:                return "PENDING";
		case Var::YEA:                    return "YEA";
		case Var::NAY:                    return "NAY";
		case Var::THRESHOLD:              return "THRESHOLD";
		case Var::STARTED:                return "STARTED";
		case Var::EXPIRED:                return "EXPIRED";
		case Var::CANCELED:               return "CANCELED";
		case Var::PASSED:                 return "PASSED";
		case Var::FAIL_QUORUM:            return "FAIL_QUORUM";
		case Var::FAIL_PLURALITY:         return "FAIL_PLURALITY";
		case Var::FAIL_VETOED:            return "FAIL_VETOED";
		default:                          return "??????";
	}
}


inline
const char *describe(const Var &var)
{
	switch(var)
	{
		case Var::ACCESS:       return "Required channel access flags.";
		case Var::MODE:         return "Required user modes in a channel.";
		case Var::BALLOT:       return "y/n/v/a ballot character or string.";
		default:                return "No description available.";
	}
}
