/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */


enum class Doc : uint
{
	VOTE,
	LIMIT,
	QUORUM,
	ENFRANCHISE,
	QUALIFY,
	SPEAKER,
	VETO,
	WEIGHT,
	VISIBLE,
	VERBOSE,

	_NUM_
};


inline
const char *reflect(const Doc &doc)
{
	switch(doc)
	{
		case Doc::VOTE:          return "VOTE";
		case Doc::LIMIT:         return "LIMIT";
		case Doc::QUORUM:        return "QUORUM";
		case Doc::ENFRANCHISE:   return "ENFRANCHISE";
		case Doc::QUALIFY:       return "QUALIFY";
		case Doc::SPEAKER:       return "SPEAKER";
		case Doc::VETO:          return "VETO";
		case Doc::WEIGHT:        return "WEIGHT";
		case Doc::VISIBLE:       return "VISIBLE";
		case Doc::VERBOSE:       return "VERBOSE";
		default:                 return "?????";
	}
}


inline
const char *describe(const Doc &doc)
{
	switch(doc)
	{
		case Doc::VOTE:          return "General voting motion parameters.";
		case Doc::LIMIT:         return "Limitations for the voting system.";
		case Doc::QUORUM:        return "Minimum participation requirements.";
		case Doc::ENFRANCHISE:   return "Who is allowed to vote.";
		case Doc::QUALIFY:       return "Who is allowed to vote in situ.";
		case Doc::SPEAKER:       return "Who can create a vote.";
		case Doc::VETO:          return "Who can veto a vote.";
		case Doc::WEIGHT:        return "The amount each ballot adjusts the effects.";
		case Doc::VISIBLE:       return "Aspects of voting that are public or secret.";
		case Doc::VERBOSE:       return "Levels and directions of output.";
		default:                 return "No description available.";
	}
}
