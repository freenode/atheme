/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */


enum class Type : uint
{
	DEFAULT      = 0,
	CONFIG,
	OPINE,
	TOPIC,
	INVITE,
	KICK,
	MODE,
	OP,
	DEOP,
	VOICE,
	DEVOICE,
	BAN,
	UNBAN,
	QUIET,
	UNQUIET,
	EXEMPT,
	UNEXEMPT,
	INVEX,
	UNINVEX,
	FLAGS,

	_NUM_            // indicates the number of elements in this enum
};


static
const char *reflect(const Type &type)
{
	switch(type)
	{
		case Type::DEFAULT:     return "DEFAULT";
		case Type::CONFIG:      return "CONFIG";
		case Type::OPINE:       return "OPINE";
		case Type::TOPIC:       return "TOPIC";
		case Type::INVITE:      return "INVITE";
		case Type::KICK:        return "KICK";
		case Type::MODE:        return "MODE";
		case Type::OP:          return "OP";
		case Type::DEOP:        return "DEOP";
		case Type::VOICE:       return "VOICE";
		case Type::DEVOICE:     return "DEVOICE";
		case Type::BAN:         return "BAN";
		case Type::UNBAN:       return "UNBAN";
		case Type::QUIET:       return "QUIET";
		case Type::UNQUIET:     return "UNQUIET";
		case Type::EXEMPT:      return "EXEMPT";
		case Type::UNEXEMPT:    return "UNEXEMPT";
		case Type::INVEX:       return "INVEX";
		case Type::UNINVEX:     return "UNINVEX";
		case Type::FLAGS:       return "FLAGS";
		default:                return "??????";
	};
}


static
const char *describe(const Type &type)
{
	switch(type)
	{
		case Type::DEFAULT:     return "Null vote type (seeing this is an error).";
		case Type::CONFIG:      return "Motion to change the voting configuration itself.";
		case Type::OPINE:       return "General opinion poll for the channel.";
		case Type::TOPIC:       return "Change the topic of the channel.";
		case Type::INVITE:      return "Invite a nickname to the channel.";
		case Type::KICK:        return "Kick a nickname from the channel.";
		case Type::MODE:        return "Change a channel's mode.";
		case Type::OP:          return "Grant operator to a nickname.";
		case Type::DEOP:        return "Remove operator from a nickname.";
		case Type::VOICE:       return "Grand voice to a nickname.";
		case Type::DEVOICE:     return "Remove voice from a nickname.";
		case Type::BAN:         return "Add items to the channel ban list.";
		case Type::UNBAN:       return "Remove items from the channel ban list.";
		case Type::QUIET:       return "Add items to the channel quiet list.";
		case Type::UNQUIET:     return "Remove items from the channel quiet list.";
		case Type::EXEMPT:      return "Add items to the ban exemption list.";
		case Type::UNEXEMPT:    return "Remove items from the ban exemption list.";
		case Type::INVEX:       return "Add items to the invite exemption list.";
		case Type::UNINVEX:     return "Remove items from the invite exemption list.";
		case Type::FLAGS:       return "Manipulate the channel's access flags list.";
		default:                return "No description available.";
	};
}
