/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */

bool has_any_access(mychan_t *const &chan, user_t *const &u, const char *const &flags);
bool has_any_mode(mychan_t *const &chan, user_t *const &u, const char *const &mode);

bool enfranchised(const Cfg &cfg, mychan_t *const &chan, user_t *const &user);
bool speaker(const Cfg &cfg, mychan_t *const &chan, user_t *const &user);
bool vetoer(const Cfg &cfg, mychan_t *const &chan, user_t *const &user);

bool cast_abstain(sourceinfo_t *const &si, Ballots &ballots, const uint &id);
bool cast_veto(sourceinfo_t *const &si, Ballots &ballots, const uint &id);
bool cast_nay(sourceinfo_t *const &si, Ballots &ballots, const uint &id);
bool cast_yea(sourceinfo_t *const &si, Ballots &ballots, const uint &id);

bool interceded(const Cfg &cfg, const Ballots &ballots);
uint plurality(const Cfg &cfg, const Ballots &ballots);
uint required(const Cfg &cfg, const Ballots &ballots, const uint &quorum);
uint moreyeas(const Cfg &cfg, const Ballots &ballots, const uint &quorum);
uint calc_quorum(const Cfg &cfg, mychan_t *const &chan);

class Vote
{
	uint id             { 0                  };  // Unique ID number of this vote.
	Type type           { Type::DEFAULT      };  // Type of vote, DEFAULT is an invalid sentinel.
	State state         { State::PENDING     };  // Explains the outcome of the vote.
	uint quorum         { 0                  };  // The quorum required to pass calculated at began
	time_t began        { 0                  };  // Time the motion was opened to ballots.
	time_t ending       { 0                  };  // Time the motion will close to ballots.
	time_t expires      { 0                  };  // Time the effects will be reversed.
	mychan_t *chan      { nullptr            };  // Channel apropos. Votes are removed with the chan.
	std::string acct;                            // Account of initiating user, preserved here
	std::string nick;                            // Nickname initiating user had at the time.
	std::string issue;                           // What the vote is about; depends on Type
	std::string effect;                          // Effects of the vote, depends on Type
	Ballots ballots;

  public:
	const uint &get_id() const                   { return id;                                      }
	const Type &get_type() const                 { return type;                                    }
	const State &get_state() const               { return state;                                   }
	const uint &get_quorum() const               { return quorum;                                  }
	const time_t &get_began() const              { return began;                                   }
	const time_t &get_ending() const             { return ending;                                  }
	const time_t &get_expires() const            { return expires;                                 }
	mychan_t *const &get_chan() const            { return chan;                                    }
	const char *get_chan_name() const            { return chan->name;                              }
	const std::string &get_acct() const          { return acct;                                    }
	const std::string &get_nick() const          { return nick;                                    }
	const std::string &get_issue() const         { return issue;                                   }
	const std::string &get_effect() const        { return effect;                                  }
	const Ballots &get_ballots() const           { return ballots;                                 }
	user_t *const get_user() const               { return user_find(get_nick().c_str());           }
	Cfg get_config() const                       { return { chan, type };                          }
	bool verbose_thresh() const;                 // Enough ballots to make announcements
	time_t remaining_effect() const;             // seconds left for [until expires], otherwise 0
	time_t remaining() const;                    // seconds left to vote [until ending], otherwise 0
	State calc_state() const;                    // Returns the State if the vote ended right now

  protected:
	void set_issue(std::string issue)            { this->issue = std::move(issue);                 }
	void set_effect(std::string effect)          { this->effect = std::move(effect);               }
	void set_expires(const time_t &expires)      { this->expires = expires;                        }
	void set_expires();

  private:
	virtual bool on_start()                      { return true;                                    }
	virtual bool on_ending()                     { return true;                                    }
	virtual bool on_cancel()                     { return true;                                    }
	virtual bool on_passed()                     { return true;                                    }
	virtual bool on_failed()                     { return true;                                    }
	virtual bool on_effect()                     { return true;                                    }
	virtual bool on_expire()                     { return true;                                    }

  public:
	virtual void on_nickchange(hook_user_nick_t *const &hook) {}

  private:
	void announce_expire() const;
	void announce_cancel() const;
	void announce_finish() const;
	void announce_start() const;

	bool attempt_prejudice();

  public:
	void expire();                               // Called by the event timer system to undo effects
	bool finish();                               // Called by the event timer system at motion close
	bool cancel();                               // Request to cancel
	bool start();                                // Starting the motion after construction

	// User submits a ballot to this vote
	bool cast(sourceinfo_t *const &si, const Ballot &ballot);

	// Atheme serializes to database at the intervals :^
	void save(database_handle_t *const &db) const;

	// Atheme deserializes from database [on startup]
	Vote(const uint &id,
	     const Type &type,
	     database_handle_t *const &db);

	// User motion, called from Votes object
	Vote(const uint &id,
	     const Type &type,
	     mychan_t *const &chan,
	     sourceinfo_t *const &user,
	     const char *const &issue);

	Vote(void) = default;
	virtual ~Vote() noexcept = default;
};


inline
Vote::Vote(const uint &id,
           const Type &type,
           mychan_t *const &chan,
           sourceinfo_t *const &user,
           const char *const &issue):
id(id),
type(type),
chan(chan),
acct(user->su->myuser->ent.name),
nick(user->su->nick),
issue(issue)
{
}


inline
Vote::Vote(const uint &id,
           const Type &type,
           database_handle_t *const &db):
id(id),
type(type),
state(static_cast<State>(db_sread_uint(db))),
quorum(db_sread_uint(db)),
began(db_sread_time(db)),
ending(db_sread_time(db)),
expires(db_sread_time(db)),
chan([&db]
{
	const auto name(db_sread_word(db));
	const auto chan(mychan_find(name));

	if(!chan)
		throw std::runtime_error("channel not found");

	if(!chanuser_find(chan->chan,myservice->me))
		join(chan->name,myservice->nick);

	return chan;
}()),
acct(db_sread_word(db)),
nick(db_sread_word(db)),
issue(db_sread_str(db)),
effect(db_sread_str(db)),
ballots(db)
{
}


inline
void Vote::save(database_handle_t *const &db)
const
{
	db_start_row(db,SPQF_DBKEY_VOTE);
	db_write_uint(db,id);
	db_write_uint(db,static_cast<uint>(type));
	db_write_uint(db,static_cast<uint>(state));
	db_write_uint(db,quorum);
	db_write_time(db,began);
	db_write_time(db,ending);
	db_write_time(db,expires);
	db_write_word(db,chan->name);
	db_write_word(db,acct.c_str());
	db_write_word(db,nick.c_str());
	db_write_str(db,issue.c_str());
	db_write_str(db,effect.c_str());
	ballots.save(db);
	db_commit_row(db);
}


inline
bool Vote::cast(sourceinfo_t *const &si,
                const Ballot &ballot)
{
	const auto &cfg(get_config());

	switch(ballot)
	{
		case Ballot::YEA:
			if(!cast_yea(si,ballots,get_id()))
				return false;

			if(state != State::PREJUDICED && cfg.get<bool>(Doc::VOTE,Var::PREJUDICE,false))
				attempt_prejudice();

			break;

		case Ballot::NAY:
			if(!cast_nay(si,ballots,get_id()))
				return false;

			break;

		case Ballot::VETO:
			if(!cast_veto(si,ballots,get_id()))
				return false;

			break;

		case Ballot::ABSTAIN:
			if(!cast_abstain(si,ballots,get_id()))
				return false;

			return true;
	}

	const auto vthresh(cfg.get<uint>(Doc::VERBOSE,Var::THRESHOLD,1));
	if(vthresh && count(ballots) == vthresh)
		announce_start();

	return true;
}


inline
bool Vote::start()
{
	const auto &cfg(get_config());

	began = CURRTIME;
	ending = began + cfg.get<time_t>(Doc::VOTE,Var::DURATION,15);
	quorum = calc_quorum(cfg,get_chan());

	if(!on_start())
		return false;

	append<YEA>(ballots,get_acct().c_str());

	if(cfg.get<uint>(Doc::VERBOSE,Var::THRESHOLD,1) == 1U)
		announce_start();

	if(cfg.get<bool>(Doc::VOTE,Var::PREJUDICE,false))
		attempt_prejudice();

	return true;
}


inline
bool Vote::cancel()
{
	if(!on_cancel())
		return false;

	state = State::CANCELED;

	if(!on_ending())
		return false;

	announce_cancel();
	return true;
}


inline
bool Vote::finish()
{
	const auto prejudice(get_state() == State::PREJUDICED);
	state = calc_state();

	if(!on_ending())
		return false;

	announce_finish();

	if(passed(state) && !prejudice)
		if(!on_effect())
			return false;

	if(failed(state) && prejudice)
		if(!on_expire())
			return false;

	return passed(state)? on_passed():
	       failed(state)? !on_failed():
	                      false;
}


inline
void Vote::expire()
{
	if(!on_expire())
		return;

	announce_expire();
}


inline
bool Vote::attempt_prejudice()
{
	if(!passed(calc_state()))
		return false;

	if(!on_effect())
		return false;

	state = State::PREJUDICED;
	return true;
}


inline
void Vote::announce_start()
const
{
	const auto &cfg(get_config());
	if(!cfg.get<bool>(Doc::VERBOSE,Var::STARTED,true))
		return;

	const auto &ballots(get_ballots());
	const bool pre_enable(cfg.get<bool>(Doc::VOTE,Var::PREJUDICE,false));

	char quobuf[64];
	snprintf(quobuf,sizeof(quobuf),
	         " \2%u\2 yeas are required to pass.",
	         moreyeas(cfg,ballots,get_quorum()));

	char durbuf[32];
	secs_cast(durbuf,sizeof(durbuf),remaining());

	char effstr[32], efftime[32];
	if(get_expires())
	{
		secs_cast(efftime,sizeof(efftime),remaining_effect()+1);
		snprintf(effstr,sizeof(effstr)," for \2%s\2.",efftime);
	}

	notice(myservice->nick,get_chan()->chan->name,
	       "Issue #\2%u\2: \2%s\2 \x1f%s\x0f%s - You have \2%s\2 to vote!%s%s",
	       get_id(),
	       reflect(get_type()),
	       get_issue().c_str(),
	       get_expires()? effstr : "",
	       durbuf,
	       quorum? quobuf : "",
	       pre_enable? " Effects with prejudice." : "");
}


inline
void Vote::announce_finish()
const
{
	const auto &cfg(get_config());
	if(passed(get_state()) && !cfg.get<bool>(Doc::VERBOSE,Var::PASSED,true))
		return;

	if(get_state() == State::QUORUM && !cfg.get<bool>(Doc::VERBOSE,Var::FAIL_QUORUM,true))
		return;

	if(get_state() == State::PLURALITY && !cfg.get<bool>(Doc::VERBOSE,Var::FAIL_PLURALITY,true))
		return;

	if(get_state() == State::VETOED && !cfg.get<bool>(Doc::VERBOSE,Var::FAIL_VETOED,true))
		return;

	if(!verbose_thresh())
		return;

	char effbuf[32], efftime[32];
	const bool effective(passed(get_state()) && get_expires() > CURRTIME);
	if(effective)
	{
		secs_cast(efftime,sizeof(efftime),remaining_effect());
		snprintf(effbuf,sizeof(effbuf),"Effective for \2%s\2.",efftime);
	}

	notice(myservice->nick,get_chan()->chan->name,
	       "Issue #\2%u\2: \2%s\2 \x1f%s\x0f - \2\x03%02d%s%s\x0f: YEA: \2\x03%02d%u\x0f. NAY: \2\x03%02d%u\x0f. %s",
	       get_id(),
	       reflect(get_type()),
	       get_issue().c_str(),
	       failed(get_state())? 5 : 3,
	       failed(get_state())? "FAILED " : "",
	       reflect(get_state()),
	       3,
	       count<YEA>(ballots),
	       5,
	       count<NAY>(ballots),
	       effective? effbuf : "");
}


inline
void Vote::announce_expire()
const
{
	const auto &cfg(get_config());
	if(!cfg.get<bool>(Doc::VERBOSE,Var::EXPIRED,true))
		return;

	if(!verbose_thresh())
		return;

	char efftime[32];
	secs_cast(efftime,sizeof(efftime),CURRTIME - get_ending());

	notice(myservice->nick,get_chan()->chan->name,
	       "Issue #\2%u\2: \2%s\2 \x1f%s\x0f - effects expired after \2%s\2.",
	       get_id(),
	       reflect(get_type()),
	       get_issue().c_str(),
	       efftime);
}


inline
void Vote::announce_cancel()
const
{
	const auto &cfg(get_config());
	if(!cfg.get<bool>(Doc::VERBOSE,Var::CANCELED,true))
		return;

	if(!verbose_thresh())
		return;

	notice(myservice->nick,get_chan()->chan->name,
	       "Issue #\2%u\2: \2%s\2 \x1f%s\x0f - has been canceled.",
	       get_id(),
	       reflect(get_type()),
	       get_issue().c_str());
}


inline
void Vote::set_expires()
{
	const auto &cfg(get_config());
	this->expires = ending + cfg.get<time_t>(Doc::VOTE,Var::EFFECTIVE,30);
}


inline
State Vote::calc_state()
const
{
	const auto &cfg(get_config());
	const auto &ballots(get_ballots());

	if(interceded(cfg,ballots))
		return State::VETOED;

	if(count(ballots) < get_quorum())
		return State::QUORUM;

	if(count<YEA>(ballots) < required(cfg,ballots,get_quorum()))
		return State::PLURALITY;

	return State::PASSED;
}


inline
time_t Vote::remaining_effect()
const
{
	const auto rem(expires - CURRTIME);
	return std::max(rem,time_t(0));
}


inline
time_t Vote::remaining()
const
{
	const auto rem(ending - CURRTIME);
	return std::max(rem,time_t(0));
}


inline
bool Vote::verbose_thresh()
const
{
	const auto &cfg(get_config());
	const auto thresh(cfg.get<uint>(Doc::VERBOSE,Var::THRESHOLD,1));
	return thresh && count(ballots) >= thresh;
}



inline
uint calc_quorum(const Cfg &cfg,
                 mychan_t *const &chan)
{
	const std::initializer_list<uint> vals
	{
		cfg.get<uint>(Doc::QUORUM,Var::BALLOTS,0),
		cfg.get<uint>(Doc::QUORUM,Var::YEAS,0),
	};

	return *std::max_element(begin(vals),end(vals));
}


inline
uint moreyeas(const Cfg &cfg,
              const Ballots &ballots,
              const uint &quorum)
{
	const auto req(required(cfg,ballots,quorum));
	const auto yea(count<YEA>(ballots));
	return req > yea? req - yea : 0;
}


inline
uint required(const Cfg &cfg,
              const Ballots &ballots,
              const uint &quorum)
{
	const auto plura(plurality(cfg,ballots));
	return plura > quorum? plura : quorum;
}


inline
uint plurality(const Cfg &cfg,
               const Ballots &ballots)
{
	const float plura(cfg.get<uint>(Doc::VOTE,Var::PLURALITY,51) / 100.0);
	return ceil(count(ballots) * plura);
}


inline
bool interceded(const Cfg &cfg,
                const Ballots &ballots)
{
	const auto vquorum(cfg.get<uint>(Doc::VETO,Var::QUORUM,1));
	return count<VETO>(ballots) >= vquorum;
}


inline
bool cast_yea(sourceinfo_t *const &user,
              Ballots &ballots,
              const uint &id)
{
	if(exists<YEA>(ballots,user))
	{
		command_fail(user, cmd_faultcode_t(0), _("You have already voted YEA on #\2%u\2."),id);
		return false;
	}

	if(remove<NAY>(ballots,user))
	{
		remove<VETO>(ballots,user);
		append<YEA>(ballots,user);
		command_success_nodata(user, _("You have changed your vote on #\2%u\2 to YEA."),id);
		return true;
	}

	append<YEA>(ballots,user);
	command_success_nodata(user, _("Thanks for casting your YEA vote on #\2%u\2."),id);
	return true;
}


inline
bool cast_nay(sourceinfo_t *const &user,
              Ballots &ballots,
              const uint &id)
{
	if(remove<VETO>(ballots,user))
	{
		command_success_nodata(user, _("You have removed your VETO on #\2%u\2 for a NAY."),id);
		return true;
	}

	if(exists<NAY>(ballots,user))
	{
		command_fail(user, cmd_faultcode_t(0), _("You have already voted NAY on #\2%u\2."),id);
		return false;
	}

	if(exists<YEA>(ballots,user))
	{
		remove<YEA>(ballots,user);
		append<NAY>(ballots,user);
		command_success_nodata(user, _("You have changed your vote on #\2%u\2 to NAY."),id);
		return true;
	}

	command_success_nodata(user, _("Thanks for casting your NAY vote on #\2%u\2."),id);
	append<NAY>(ballots,user);
	return true;
}


inline
bool cast_veto(sourceinfo_t *const &user,
               Ballots &ballots,
               const uint &id)
{
	if(exists<VETO>(ballots,user))
	{
		command_fail(user, cmd_faultcode_t(0), _("You have already vetoed #\2%u\2."),id);
		return false;
	}

	if(remove<YEA>(ballots,user))
	{
		remove<YEA>(ballots,user);
		append<NAY>(ballots,user);
		append<VETO>(ballots,user);
		command_success_nodata(user, _("You have changed your vote to NAY with VETO on #\2%u\2."),id);
		return true;
	}

	if(exists<NAY>(ballots,user))
	{
		append<VETO>(ballots,user);
		command_success_nodata(user, _("You have added a VETO to your NAY vote on #\2%u\2."),id);
		return true;
	}

	command_success_nodata(user, _("Thanks for casting your vote on #\2%u\2."),id);
	append<NAY>(ballots,user);
	append<VETO>(ballots,user);
	return true;
}


inline
bool cast_abstain(sourceinfo_t *const &user,
                  Ballots &ballots,
                  const uint &id)
{
	if(remove<VETO>(ballots,user))
	{
		remove<NAY>(ballots,user);
		command_success_nodata(user, _("You have removed your VETO and NAY vote on #\2%u\2."),id);
		return true;
	}

	if(remove<NAY>(ballots,user))
	{
		command_success_nodata(user, _("You have removed your NAY vote on #\2%u\2."),id);
		return true;
	}

	if(remove<YEA>(ballots,user))
	{
		command_success_nodata(user, _("You have removed your YEA vote on #\2%u\2."),id);
		return true;
	}

	command_fail(user, cmd_faultcode_t(), _("You have not voted for #\2%u\2."),id);
	return false;
}


inline
bool vetoer(const Cfg &cfg,
            mychan_t *const &chan,
            user_t *const &user)
{
	if(!user->myuser)
		return false;

	const auto astr(cfg.get(Doc::VETO,Var::ACCESS));
	const auto mstr(cfg.get(Doc::VETO,Var::MODE));
	return has_any_access(chan,user,astr) ||
	       has_any_mode(chan,user,mstr);
}


inline
bool speaker(const Cfg &cfg,
             mychan_t *const &chan,
             user_t *const &user)
{
	if(!user->myuser)
		return false;

	const auto astr(cfg.get(Doc::SPEAKER,Var::ACCESS));
	const auto mstr(cfg.get(Doc::SPEAKER,Var::MODE));
	return (!astr && !mstr && enfranchised(cfg,chan,user)) ||
	       has_any_access(chan,user,astr) ||
	       has_any_mode(chan,user,mstr);
}


bool enfranchised(const Cfg &cfg,
                  mychan_t *const &chan,
                  user_t *const &user)
{
	if(!user->myuser)
		return false;

	const auto astr(cfg.get(Doc::ENFRANCHISE,Var::ACCESS));
	const auto mstr(cfg.get(Doc::ENFRANCHISE,Var::MODE));
	return (!astr && !mstr) ||
	       has_any_access(chan,user,astr) ||
	       has_any_mode(chan,user,mstr);
}


inline
bool has_any_mode(mychan_t *const &chan,
                  user_t *const &user,
                  const char *const &modes)
{
	if(!modes)
		return false;

	const auto chanuser(chanuser_find(chan->chan,user));
	if(!chanuser)
		return false;

	const auto &ucmode(chanuser->modes);
	return ((ucmode & CSTATUS_OP) && strchr(modes,'o')) ||
	       ((ucmode & CSTATUS_VOICE) && strchr(modes,'v')) ||
	       ((ucmode & CSTATUS_HALFOP) && strchr(modes,'h'));


}


inline
bool has_any_access(mychan_t *const &chan,
                    user_t *const &user,
                    const char *const &flags)
{
	if(!flags)
		return false;

	const auto cflags(flags_to_bitmask(flags,0));
	const auto uflags(chanacs_user_flags(chan,user));
	return cflags & uflags;
}
