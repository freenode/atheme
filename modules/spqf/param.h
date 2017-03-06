/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */

namespace param {


enum Spec : uint
{
	VAR,       // Apropos Var enum
	FORM,      // A character representing the (custom) type of value required for this Var
	LOW,       // For appropriate types, this is a lower bound on what the user can enter
	HIGH       // The upper bound on what the user can enter; if 0, usually ignored; for strings, maxlen
};

enum Form : char
{
	STRING     = 's',
	BOOLEAN    = 'b',
	UNSIGNED   = 'u',
	TIMESTR    = 't',
	CMODES     = 'M',
	ACSFLAGS   = 'F',
	BALLOT     = 'B',
};

using Param = std::tuple<Var,Form,int,int>;
using Params = std::initializer_list<Param>;


const Param nullspec;
std::array<Params,num_of<Doc>()> tree
{{
	// VOTE
	{
		Param { Var::ENABLE,              Form::BOOLEAN,    0, 0                         },
		Param { Var::DURATION,            Form::TIMESTR,    15, 60*60*24*30L             },
		Param { Var::EFFECTIVE,           Form::TIMESTR,    60, 60*60*24*30*6L           },
		Param { Var::PLURALITY,           Form::UNSIGNED,   0, 100                       },
		Param { Var::PREJUDICE,           Form::BOOLEAN,    0, 0                         },
		Param { Var::QUICK,               Form::BOOLEAN,    0, 0                         },
	},

	// LIMIT
	{
		Param { Var::TYPE,                Form::UNSIGNED,   0, 32    /* pending */       },
		Param { Var::USER,                Form::UNSIGNED,   0, 32    /* pending */       },
		Param { Var::CHAN,                Form::UNSIGNED,   0, 5000  /* total */         },
		Param { Var::RETRY_QUORUM,        Form::TIMESTR,    0, 60*60*24*30*12L           },
		Param { Var::RETRY_VETOED,        Form::TIMESTR,    0, 60*60*24*30*12L           },
		Param { Var::RETRY_PLURALITY,     Form::TIMESTR,    0, 60*60*24*30*12L           },
	},

	// QUORUM
	{
		Param { Var::BALLOTS,             Form::UNSIGNED,   0, 1000                      },
		Param { Var::YEAS,                Form::UNSIGNED,   0, 1000                      },
		Param { Var::TURNOUT,             Form::UNSIGNED,   0, 100                       },
	},

	// ENFRANCHISE
	{
		Param { Var::ACCESS,              Form::ACSFLAGS,   0, 0                         },
		Param { Var::MODE,                Form::CMODES,     0, 0                         },
	},

	// QUALIFY
	{
		// Not implemented yet
	},

	// SPEAKER
	{
		Param { Var::ACCESS,              Form::ACSFLAGS,   0, 0                         },
		Param { Var::MODE,                Form::CMODES,     0, 0                         },
		Param { Var::BALLOT,              Form::BALLOT,     0, 0                         },
	},

	// VETO
	{
		Param { Var::ACCESS,              Form::ACSFLAGS,   0, 0                         },
		Param { Var::MODE,                Form::CMODES,     0, 0                         },
		Param { Var::QUORUM,              Form::UNSIGNED,   0, 1000                      },
		Param { Var::QUICK,               Form::BOOLEAN,    0, 0                         },
	},

	// WEIGHT
	{
		Param { Var::YEA,                 Form::TIMESTR,    0, 60*60*24*30L              },
		Param { Var::NAY,                 Form::TIMESTR,    0, 60*60*24*30L              },
	},

	// VISIBLE
	{
		Param { Var::TALLY,               Form::BOOLEAN,    0, 0                         },
		Param { Var::PENDING,             Form::BOOLEAN,    0, 0                         },
		Param { Var::YEAS,                Form::BOOLEAN,    0, 0                         },
		Param { Var::NAYS,                Form::BOOLEAN,    0, 0                         },
		Param { Var::VETOES,              Form::BOOLEAN,    0, 0                         },
	},

	// VERBOSE
	{
		Param { Var::THRESHOLD,           Form::UNSIGNED,   0, 1000                      },
		Param { Var::STARTED,             Form::BOOLEAN,    0, 0                         },
		Param { Var::EXPIRED,             Form::BOOLEAN,    0, 0                         },
		Param { Var::CANCELED,            Form::BOOLEAN,    0, 0                         },
		Param { Var::PASSED,              Form::BOOLEAN,    0, 0                         },
		Param { Var::FAIL_QUORUM,         Form::BOOLEAN,    0, 0                         },
		Param { Var::FAIL_PLURALITY,      Form::BOOLEAN,    0, 0                         },
		Param { Var::FAIL_VETOED,         Form::BOOLEAN,    0, 0                         },
	},
}};


inline
const Param &find(const Doc &doc,
                  const Var &var)
{
	const auto lam([&var](const Param &param)
	{
		return get<Spec::VAR>(param) == var;
	});

	const auto &params(tree[uint(doc)]);
	const auto it(std::find_if(begin(params),end(params),lam));
	return it != end(params)? *it : nullspec;
}


inline
bool exists(const Doc &doc,
            const Var &var)
{
	return find(doc,var) != nullspec;
}


inline
bool valid_boolean(const Doc &doc,
                   const Var &var,
                   const char *const &value,
                   char *const &errstr = nullptr,
                   const size_t &errsz = 0)
{
	const auto &spec(find(doc,var));
	if(spec == nullspec)
	{
		mowgli_strlcpy(errstr,_("This parameter is not specified."),errsz);
		return false;
	}

	if(get<Spec::FORM>(spec) != Form::BOOLEAN)
	{
		mowgli_strlcpy(errstr,_("This parameter is specified as a different type."),errsz);
		return false;
	}

	switch(hash(value))
	{
		case hash("1"):
		case hash("0"):
		case hash("true"):
		case hash("false"):
			return true;

		default:
			mowgli_strlcpy(errstr,_("You must specify a boolean value \"1\", \"true\" or \"0\" \"false\" (without the quotes)."),errsz);
			return false;
	}
}


inline
bool valid_unsigned(const Doc &doc,
                    const Var &var,
                    const char *const &value,
                    char *const &errstr = nullptr,
                    const size_t &errsz = 0)
{
	const auto &spec(find(doc,var));
	if(spec == nullspec)
	{
		mowgli_strlcpy(errstr,_("This parameter is not specified."),errsz);
		return false;
	}

	if(get<Spec::FORM>(spec) != Form::UNSIGNED)
	{
		mowgli_strlcpy(errstr,_("This parameter is specified as a different type."),errsz);
		return false;
	}

	if(!isnumeric(value,16))
	{
		mowgli_strlcpy(errstr,_("You must specify a positive integer for this parameter."),errsz);
		return false;
	}

	const auto val(atoi(value));
	if(val < 0)
	{
		mowgli_strlcpy(errstr,_("This value requires a positive integer."),errsz);
		return false;
	}

	if(get<Spec::HIGH>(spec) && (val < get<Spec::LOW>(spec) || val > get<Spec::HIGH>(spec)))
	{
		snprintf(errstr,errsz,_("The integer for this parameter must be between %d and %d inclusively."),
		         get<Spec::LOW>(spec),
		         get<Spec::HIGH>(spec));
		return false;
	}

	return true;
}


inline
bool valid_timestr(const Doc &doc,
                   const Var &var,
                   const char *const &value,
                   char *const &errstr = nullptr,
                   const size_t &errsz = 0)
{
	const auto &spec(find(doc,var));
	if(spec == nullspec)
	{
		mowgli_strlcpy(errstr,_("This parameter is not specified."),errsz);
		return false;
	}

	if(get<Spec::FORM>(spec) != Form::TIMESTR)
	{
		mowgli_strlcpy(errstr,_("This parameter is specified as a different type."),errsz);
		return false;
	}

	const auto val(secs_cast(value));
	if(val < 0)
	{
		mowgli_strlcpy(errstr,_("Invalid time duration value. Must be an integer with an optional valid unit postfix. i.e 10m for ten minutes, or 3h for three hours, or 120 for two minutes."), errsz);
		return false;
	}

	if(get<Spec::HIGH>(spec) && (val < get<Spec::LOW>(spec) || val > get<Spec::HIGH>(spec)))
	{
		char lowbuf[32], highbuf[32];
		secs_cast(lowbuf,sizeof(lowbuf),get<Spec::LOW>(spec));
		secs_cast(highbuf,sizeof(highbuf),get<Spec::HIGH>(spec));
		snprintf(errstr,sizeof(errsz),_("The duration must be between %s and %s inclusively"),lowbuf,highbuf);
		return false;
	}

	return true;
}


inline
bool valid_cmodes(const Doc &doc,
                  const Var &var,
                  const char *const &value,
                  char *const &errstr = nullptr,
                  const size_t &errsz = 0)
{
	const auto &spec(find(doc,var));
	if(spec == nullspec)
	{
		mowgli_strlcpy(errstr,_("This parameter is not specified."),errsz);
		return false;
	}

	if(get<Spec::FORM>(spec) != Form::CMODES)
	{
		mowgli_strlcpy(errstr,_("This parameter is specified as a different type."),errsz);
		return false;
	}

	const auto len(strnlen(value,4));
	const bool valid_modes(std::all_of(value,value+len,[]
	(const char &c)
	{
		return c == 'o' || c == 'v' || c == 'h';
	}));

	if(!valid_modes || len >= 4)
	{
		mowgli_strlcpy(errstr,_("Invalid channel-user modes. Must be a string containing any or all of o, v, or h for operator, voice, or halfop respectively."), errsz);
		return false;
	}

	return true;
}


inline
bool valid_acsflags(const Doc &doc,
                    const Var &var,
                    const char *const &value,
                    char *const &errstr = nullptr,
                    const size_t &errsz = 0)
{
	const auto &spec(find(doc,var));
	if(spec == nullspec)
	{
		mowgli_strlcpy(errstr,_("This parameter is not specified."),errsz);
		return false;
	}

	if(get<Spec::FORM>(spec) != Form::ACSFLAGS)
	{
		mowgli_strlcpy(errstr,_("This parameter is specified as a different type."),errsz);
		return false;
	}

	const auto len(strnlen(value,64));
	const bool valid_flags(std::all_of(value,value+len,[]
	(const uint8_t &flag)
	{
		return chanacs_flags[flag].value;
	}));

	if(!valid_flags || len >= 64)
	{
		mowgli_strlcpy(errstr,_("Invalid channel access flags. Must be a string containing a chanserv access flag pattern (omit the +/-). See: /msg chanserv help flags."), errsz);
		return false;
	}

	return true;
}


inline
bool valid_ballot(const Doc &doc,
                  const Var &var,
                  const char *const &value,
                  char *const &errstr = nullptr,
                  const size_t &errsz = 0)
{
	const auto &spec(find(doc,var));
	if(spec == nullspec)
	{
		mowgli_strlcpy(errstr,_("This parameter is not specified."),errsz);
		return false;
	}

	if(get<Spec::FORM>(spec) != Form::BALLOT)
	{
		mowgli_strlcpy(errstr,_("This parameter is specified as a different type."),errsz);
		return false;
	}

	// Ballots default to ABSTAIN and the reflection gives safe enough program behavior,
	// but we can still hack on that here to inform the user of at least a grotesque mistype.
	const auto b(ballot(value));
	if(b != Ballot::ABSTAIN)
		return true;

	if(strnlen(value,8) > 0 && value[0] == 'a')
		return true;

	mowgli_strlcpy(errstr,_("This is not a valid ballot type."),errsz);
	return false;
}


inline
bool valid_string(const Doc &doc,
                  const Var &var,
                  const char *const &value,
                  char *const &errstr = nullptr,
                  const size_t &errsz = 0)
{
	const auto &spec(find(doc,var));
	if(spec == nullspec)
	{
		mowgli_strlcpy(errstr,_("This parameter is not specified."),errsz);
		return false;
	}

	if(get<Spec::FORM>(spec) != Form::STRING)
	{
		mowgli_strlcpy(errstr,_("This parameter is specified as a different type."),errsz);
		return false;
	}

	const auto len(strnlen(value,BUFSIZE));
	if(get<Spec::HIGH>(spec) && (len <= get<Spec::LOW>(spec) || len >= get<Spec::HIGH>(spec)))
	{
		snprintf(errstr,errsz,_("The length of this string must be longer than %d and shorter than %d characters."),
		         get<Spec::LOW>(spec),
		         get<Spec::HIGH>(spec));

		return false;
	}

	return true;
}


inline
bool valid(const Doc &doc,
           const Var &var,
           const char *const &val,
           char *const &errstr = nullptr,
           const size_t &errsz = 0)
{
	const auto &spec(find(doc,var));
	if(spec == nullspec)
	{
		mowgli_strlcpy(errstr,_("This parameter is not known or specified."),errsz);
		return false;
	}

	if(!val)
		return true;

	switch(get<Spec::FORM>(spec))
	{
		case Form::BOOLEAN:    return valid_boolean(doc,var,val,errstr,sizeof(errstr));
		case Form::UNSIGNED:   return valid_unsigned(doc,var,val,errstr,sizeof(errstr));
		case Form::TIMESTR:    return valid_timestr(doc,var,val,errstr,sizeof(errstr));
		case Form::CMODES:     return valid_cmodes(doc,var,val,errstr,sizeof(errstr));
		case Form::ACSFLAGS:   return valid_acsflags(doc,var,val,errstr,sizeof(errstr));
		case Form::BALLOT:     return valid_ballot(doc,var,val,errstr,sizeof(errstr));
		case Form::STRING:     return valid_string(doc,var,val,errstr,sizeof(errstr));
		default:
			mowgli_strlcpy(errstr,"An unhandled type was encountered for this variable.",sizeof(errstr));
			return false;
	}
}


} // namespace param
