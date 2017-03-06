/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */


struct Votes
{
	Events &events;
	Factory &factory;

	// Index of votes by ID number (master)
	using vidx_key = uint;
	using vidx_val = std::unique_ptr<Vote>;
	using vidx_cmp = std::less<vidx_key>;
	using vidx_alloc = allocator<std::pair<const vidx_key, vidx_val>>;
	std::map<vidx_key, vidx_val, vidx_cmp, vidx_alloc> vidx;

	// Index of votes by channel (slave)
	using cidx_key = const mychan_t *;
	using cidx_val = Vote *;
	using cidx_cmp = std::less<cidx_key>;
	using cidx_alloc = allocator<std::pair<const cidx_key, cidx_val>>;
	std::multimap<cidx_key, cidx_val, cidx_cmp, cidx_alloc> cidx;

	// Set of votes in a pending state (slave)
	using pidx_key = Vote *;
	using pidx_cmp = std::less<pidx_key>;
	using pidx_alloc = allocator<pidx_key>;
	std::set<pidx_key, pidx_cmp, pidx_alloc> pidx;

	// Not found returns null
	template<class T = Vote> const T *get(const uint &id) const;
	template<class T = Vote> T *get(const uint &id);

	// Utils
	void for_each(const mychan_t *const &chan, const std::function<void (const Vote &vote)> &func) const;
	void for_each(const mychan_t *const &chan, const std::function<bool (const Vote &vote)> &func) const;
	void for_each(const mychan_t *const &chan, const std::function<void (Vote &vote)> &func);
	void for_each(const mychan_t *const &chan, const std::function<bool (Vote &vote)> &func);
	bool any_of(const mychan_t *const &chan, const std::function<bool (const Vote &vote)> &func) const;
	size_t count_if(const mychan_t *const &chan, const std::function<bool (const Vote &vote)> &func) const;
	size_t count(const mychan_t *const &chan) const;

  private:
	uint next_id() const;
	void erase_cidx(const Vote *const &vote);

	void handle_expire(const uint id);
	void set_expire_event(const Vote &vote);

	bool finish(Vote &vote);
	void handle_finish(const uint id);
	void set_finish_event(const Vote &vote);

	// Tries to cast a YEA ballot on a duplicate motion, otherwise null
	Vote *duplicate_motion(const Type &type, mychan_t *const &chan, sourceinfo_t *const &user, const char *const &issue);
	bool valid_motion(const Type &type, mychan_t *const &chan, sourceinfo_t *const &user, const char *const &issue) const;

  public:
	// Proper way to cast ballots to a vote
	bool cast(sourceinfo_t *const &si, Vote &vote, const Ballot &ballot);
	bool cast(sourceinfo_t *const &si, const uint &id, const Ballot &ballot);

	// Proper way to initiate a new voting motion. Returns null if failed to begin.
	template<class T> T *motion(const Type &type, mychan_t *const &chan, sourceinfo_t *const &user, const char *const &issue);
	template<class T> T *motion(const Type &type, const char *const &chan, sourceinfo_t *const &user, const char *const &issue);

	// Control interface used by main module
	void nickchange(hook_user_nick_t *const &hook);
	void parse(database_handle_t *const &db, const char *const &type);
	void save(database_handle_t *const &db);
	size_t erase(const mychan_t *const &chan);
	void erase(const Type &type);

	Votes(Events &events, Factory &factory);
};


inline
Votes::Votes(Events &events,
             Factory &factory):
events(events),
factory(factory)
{
}


inline
void Votes::erase(const Type &type)
{
	for(auto it(vidx.cbegin()); it != vidx.cend();)
	{
		const auto &vote(it->second);
		if(vote->get_type() == type)
		{
			const auto id(it->first);
			pidx.erase(vote.get());
			erase_cidx(vote.get());
			vidx.erase(it++);

			slog(LG_VERBOSE,SPQF_NAME": Removed %s vote #%u",reflect(type),id);
		}
		else ++it;
	}
}


inline
size_t Votes::erase(const mychan_t *const &chan)
{
	const auto pit(cidx.equal_range(chan));
	for(auto it(pit.first); it != pit.second; ++it)
	{
		const auto &vote(it->second);
		const auto id(vote->get_id());
		pidx.erase(vote);
		vidx.erase(id);
	}

	const auto ret(std::distance(pit.first,pit.second));
	cidx.erase(pit.first,pit.second);
	return ret;
}


inline
void Votes::save(database_handle_t *const &db)
{
	for(const auto &p : vidx)
	{
		const auto &id(p.first);
		const auto &vote(p.second);
		vote->save(db);
	}
}


inline
void Votes::parse(database_handle_t *const &db,
                  const char *const &_type)
try
{
	const auto id(db_sread_uint(db));
	const auto type(static_cast<Type>(db_sread_uint(db)));
	const auto iit(vidx.emplace(id,factory(id,type,db)));
	const auto &ptr(iit.first->second);
	auto &vote(*ptr);

	const auto chan(vote.get_chan());
	cidx.emplace(chan,ptr.get());

	if(pending(vote.get_state()))
	{
		if(vote.get_ending() > CURRTIME)
		{
			pidx.emplace(&vote);
			set_finish_event(vote);
		}
		else finish(vote);
	}
	else if(vote.get_expires() > CURRTIME)
		set_expire_event(vote);

}
catch(const std::exception &e)
{
	slog(LG_ERROR,
	     SPQF_NAME": Error when loading a vote from the db: %s",
	     e.what());
}


inline
void Votes::nickchange(hook_user_nick_t *const &hook)
{
	std::for_each(begin(pidx),end(pidx),[&hook]
	(Vote *const &vote)
	{
		vote->on_nickchange(hook);
	});
}


template<class T>
T *Votes::motion(const Type &type,
                 const char *const &chan,
                 sourceinfo_t *const &user,
                 const char *const &issue)
{
	const auto mchan(mychan_find(chan));
	if(!mchan)
	{
		command_fail(user, fault_nosuch_target, _("Channel is not registered."));
		return nullptr;
	}

	return motion<T>(type,mchan,user,issue);
}


template<class T>
T *Votes::motion(const Type &type,
                 mychan_t *const &chan,
                 sourceinfo_t *const &user,
                 const char *const &issue)
{
	const auto dup(duplicate_motion(type,chan,user,issue));
	if(dup)
		return static_cast<T *>(dup);

	if(!valid_motion(type,chan,user,issue))
		return nullptr;

	const auto id(next_id());
	std::unique_ptr<Vote> vote(new T(id,type,chan,user,issue));

	if(!vote->start())
		return nullptr;

	auto iit(vidx.emplace(id,std::move(vote)));
	if(iit.second)
	{
		auto &vote(iit.first->second);
		const auto &chan(vote->get_chan());
		cidx.emplace(chan,vote.get());
		pidx.emplace(vote.get());
		set_finish_event(*vote);
		return static_cast<T *>(vote.get());
	}
	else throw std::runtime_error("Vote ID number already in use");
}


inline
bool Votes::cast(sourceinfo_t *const &si,
                 const uint &id,
                 const Ballot &ballot)
{
	auto *const vote(get(id));
	if(!vote)
	{
		command_fail(si, fault_nosuch_key, _("Failed to find a vote with that ID number."));
		return false;
	}

	return cast(si,*vote,ballot);
}


inline
bool Votes::cast(sourceinfo_t *const &si,
                 Vote &vote,
                 const Ballot &ballot)
{
	if(!pending(vote.get_state()))
	{
		command_fail(si, fault_nochange, _("Vote #\2%u\2 is not currently open to ballots."),vote.get_id());
		return false;
	}

	const auto cfg(vote.get_config());
	if(!enfranchised(cfg,vote.get_chan(),si->su))
	{
		command_fail(si, fault_authfail, _("You are not enfranchised to vote on #\2%u\2."),vote.get_id());
		return false;
	}

	if(ballot == Ballot::VETO && !vetoer(cfg,vote.get_chan(),si->su))
	{
		command_fail(si, fault_authfail, _("You are not authorized to veto #\2%u\2."),vote.get_id());
		return false;
	}

	if(!vote.cast(si,ballot))
		return false;

	if(ballot == Ballot::VETO && cfg.get<bool>(Doc::VETO,Var::QUICK) && interceded(cfg,vote.get_ballots()))
		finish(vote);

	return true;
}


inline
bool Votes::valid_motion(const Type &type,
                         mychan_t *const &chan,
                         sourceinfo_t *const &user,
                         const char *const &issue)
const
{
	const Cfg cfg(chan,type);
	if(!cfg.get<bool>(Doc::VOTE,Var::ENABLE,false))
	{
		command_fail(user, fault_authfail, _("Votes of this type \2%s\2 are not enabled on \2%s\2"),
		             reflect(type),
		             chan->name);

		return false;
	}

	if(!speaker(cfg,chan,user->su))
	{
		command_fail(user, fault_authfail, _("You are not authorized to create this vote."));
		return false;
	}

	// The chan limit is for all votes in all states
	{
		const auto limit(cfg.get<uint>(Doc::LIMIT,Var::CHAN,5000));
		if(count(chan) >= limit)
		{
			command_fail(user, fault_toomany, _("Channel \2%s\2 has reached its maximum(\2%lu\2) number of votes."),
			             chan->name,
			             limit);
			return false;
		}
	}

	// The user limit is only for votes in pending states
	{
		const auto &name(user->smu->ent.name);
		const auto limit(cfg.get<uint>(Doc::LIMIT,Var::USER,3));
		const auto lambda([&name](const Vote &vote)
		{
			return pending(vote.get_state()) && vote.get_acct() == name;
		});

		if(count_if(chan,lambda) >= limit)
		{
			command_fail(user, fault_toomany, _("You have reached the limit(\2%u\2) for pending motions in \2%s\2."),
			             limit,
			             chan->name);
			return false;
		}
	}

	// The type limit is only for votes in pending states
	{
		const auto limit(cfg.get<uint>(Doc::LIMIT,Var::TYPE,32));
		const auto lambda([&type](const Vote &vote)
		{
			return pending(vote.get_state()) && vote.get_type() == type;
		});

		if(count_if(chan,lambda) >= limit)
		{
			command_fail(user, fault_toomany, _("You have reached the limit(\2%u\2) for pending motions of type \2%s\2 in \2%s\2."),
			             limit,
			             reflect(type),
			             chan->name);
			return false;
		}
	}

	// The retry limits
	{
		const auto failmsg([&](const State &state, const time_t &limit)
		{
			char timestr[32];
			secs_cast(timestr,sizeof(timestr),limit);
			command_fail(user, fault_toomany,
			             _("You may not make another \2%s\2 vote in \2%s\2 when one failed by \2\x03%02u%s\x0f within the last \2%s\2."),
			             reflect(type),
			             chan->name,
			             colors::FG::RED,
			             reflect(state),
			             timestr);
		});

		const auto exceeds([&](const Vote &vote, const State &state, const time_t &limit)
		{
			return vote.get_type() == type &&
			       vote.get_issue() == issue &&
			       vote.get_state() == state &&
			       CURRTIME < vote.get_ending() + limit;
		});

		const auto test([&](const State &state, const Var &var)
		{
			const auto limit(cfg.get<uint>(Doc::LIMIT,var,0));
			const auto found(any_of(chan,[&](const Vote &vote)
			{
				return exceeds(vote,state,limit);
			}));

			if(found)
				failmsg(state,limit);

			return !found;
		});

		return test(State::QUORUM,Var::RETRY_QUORUM) &&
		       test(State::VETOED,Var::RETRY_VETOED) &&
		       test(State::PLURALITY,Var::RETRY_PLURALITY);
	}

	return true;
}


inline
Vote *Votes::duplicate_motion(const Type &type,
                              mychan_t *const &chan,
                              sourceinfo_t *const &user,
                              const char *const &issue)
{
	auto pit(cidx.equal_range(chan));
	for(; pit.first != pit.second; ++pit.first)
	{
		const auto &vote(pit.first->second);
		if(vote->get_type() != type)
			continue;

		if(!pending(vote->get_state()))
			continue;

		if(strcasecmp(vote->get_issue().c_str(),issue) != 0)
			continue;

		cast(user,*vote,Ballot::YEA);
		return vote;
	}

	return nullptr;
}


inline
void Votes::set_finish_event(const Vote &vote)
{
	events.at_time(vote.get_ending(),std::bind(&Votes::handle_finish,this,vote.get_id()));

	slog(LG_VERBOSE,
	     SPQF_NAME": Set timer for vote #%u finish at %zu",
		 vote.get_id(),
		 vote.get_ending());
}


inline
void Votes::handle_finish(const uint id)
{
	slog(LG_VERBOSE,
	     SPQF_NAME": Vote #%u finish event handler",
	     id);

	const auto it(vidx.find(id));
	if(it == end(vidx))
		throw std::runtime_error("Failed to find vote ID in finish event callback");

	const auto &vote(it->second);
	finish(*vote);
}


inline
bool Votes::finish(Vote &vote)
{
	if(!pending(vote.get_state()))
		return false;

	const scope pidx_erase([this,&vote]
	{
		pidx.erase(&vote);
	});

	if(!vote.finish())
		return false;

	if(vote.get_expires() <= CURRTIME)
		return false;

	set_expire_event(vote);
	return true;
}


inline
void Votes::set_expire_event(const Vote &vote)
{
	events.at_time(vote.get_expires(),std::bind(&Votes::handle_expire,this,vote.get_id()));

	slog(LG_VERBOSE,
	     SPQF_NAME": Set timer for vote #%u effects expiration at %zu",
	     vote.get_id(),
	     vote.get_expires());
}


inline
void Votes::handle_expire(const uint id)
{
	slog(LG_VERBOSE,
	     SPQF_NAME": Vote #%u expire event handler.",
	     id);

	const auto it(vidx.find(id));
	if(it == end(vidx))
		throw std::runtime_error("Failed to find vote ID in expire event callback");

	const auto &vote(it->second);
	vote->expire();
}


inline
void Votes::erase_cidx(const Vote *const &vote)
{
	auto cit(cidx.equal_range(vote->get_chan()));
	for(; cit.first != cit.second; ++cit.first)
	{
		if(cit.first->second == vote)
		{
			cidx.erase(cit.first);
			break;
		}
	}
}


inline
uint Votes::next_id()
const
{
	if(vidx.empty())
		return 1;

	const auto rit(vidx.crbegin());
	return rit->first + 1;
}


inline
size_t Votes::count(const mychan_t *const &chan)
const
{
	const auto pit(cidx.equal_range(chan));
	return std::distance(pit.first,pit.second);
}


inline
bool Votes::any_of(const mychan_t *const &chan,
                   const std::function<bool (const Vote &vote)> &closure)
const
{
	const auto pit(cidx.equal_range(chan));
	return std::any_of(pit.first,pit.second,[&closure]
	(const decltype(cidx)::value_type &p)
	{
		const auto &vote(p.second);
		return closure(*vote);
	});
}


inline
size_t Votes::count_if(const mychan_t *const &chan,
                       const std::function<bool (const Vote &vote)> &closure)
const
{
	const auto pit(cidx.equal_range(chan));
	return std::count_if(pit.first,pit.second,[&closure]
	(const decltype(cidx)::value_type &p)
	{
		const auto &vote(p.second);
		return closure(*vote);
	});
}


inline
void Votes::for_each(const mychan_t *const &chan,
                     const std::function<bool (Vote &vote)> &closure)
{
	auto pit(cidx.equal_range(chan));
	for(; pit.first != pit.second; ++pit.first)
	{
		auto &it(pit.first);
		auto &vote(it->second);
		if(!closure(*vote))
			break;
	}
}


inline
void Votes::for_each(const mychan_t *const &chan,
                     const std::function<void (Vote &vote)> &closure)
{
	auto pit(cidx.equal_range(chan));
	std::for_each(pit.first,pit.second,[&closure]
	(decltype(cidx)::value_type &p)
	{
		auto &vote(p.second);
		closure(*vote);
	});
}


inline
void Votes::for_each(const mychan_t *const &chan,
                     const std::function<bool (const Vote &vote)> &closure)
const
{
	auto pit(cidx.equal_range(chan));
	for(; pit.first != pit.second; ++pit.first)
	{
		const auto &it(pit.first);
		const auto &vote(it->second);
		if(!closure(*vote))
			break;
	}
}


inline
void Votes::for_each(const mychan_t *const &chan,
                     const std::function<void (const Vote &vote)> &closure)
const
{
	auto pit(cidx.equal_range(chan));
	std::for_each(pit.first,pit.second,[&closure]
	(const decltype(cidx)::value_type &p)
	{
		const auto &vote(p.second);
		closure(*vote);
	});
}


template<class T>
T *Votes::get(const uint &id)
{
	const auto it(vidx.find(id));
	if(it == end(vidx))
		return nullptr;

	const auto &ptr(it->second);
	return static_cast<T *>(ptr.get());
}


template<class T>
const T *Votes::get(const uint &id)
const
{
	const auto it(vidx.find(id));
	if(it == end(vidx))
		return nullptr;

	const auto &ptr(it->second);
	return static_cast<const T *>(ptr.get());
}
