/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */


enum Ballot
{
	YEA,
	NAY,
	VETO,
	ABSTAIN,
};

using Tally = std::pair<uint,uint>;
struct Ballots : std::tuple<std::string,std::string,std::string>
{
	void save(database_handle_t *const &db) const;

	Ballots(void) = default;
	Ballots(database_handle_t *const &db);
};

const char *reflect(const Ballot &ballot);
Ballot ballot(const char *const &str);
template<Ballot type> void for_each(const Ballots &ballots, const std::function<void (const char *)> &func);
template<Ballot type> bool exists(const Ballots &ballots, const char *const &acct);
template<Ballot type> bool exists(const Ballots &ballots, const sourceinfo_t *const &si);
template<Ballot type> size_t count(const Ballots &ballots);
Tally tally(const Ballots &ballots);
uint count(const Tally &tally);
uint count(const Ballots &ballots);

template<Ballot type> void append(Ballots &ballots, const char *const &acct);
template<Ballot type> void append(Ballots &ballots, const sourceinfo_t *const &si);
template<Ballot type> bool remove(Ballots &ballots, const char *const &acct);
template<Ballot type> bool remove(Ballots &ballots, const sourceinfo_t *const &si);


inline
Ballots::Ballots(database_handle_t *const &db):
std::tuple<std::string,std::string,std::string>
{
	db_sread_str(db),
	db_sread_str(db),
	db_sread_str(db)
}
{
}


inline
void Ballots::save(database_handle_t *const &db)
const
{
	db_write_str(db,get<YEA>(*this).c_str());
	db_write_str(db,get<NAY>(*this).c_str());
	db_write_str(db,get<VETO>(*this).c_str());
}


template<Ballot type>
bool remove(Ballots &ballots,
            const sourceinfo_t *const &si)
{
	const auto &acct(si->su->myuser->ent.name);
	return remove<type>(ballots,acct);
}


template<Ballot type>
bool remove(Ballots &ballots,
            const char *const &acct)
{
	const auto len(strnlen(acct,NICKLEN));
	const auto pos(get<type>(ballots).find(acct));
	if(pos == std::string::npos)
		return false;

	get<type>(ballots).erase(pos,len+1);
	return true;
}


template<Ballot type>
void append(Ballots &ballots,
            const sourceinfo_t *const &si)
{
	const auto &acct(si->su->myuser->ent.name);
	append<type>(ballots,acct);
}


template<Ballot type>
void append(Ballots &ballots,
            const char *const &acct)
{
	get<type>(ballots).append(acct);
	get<type>(ballots).push_back(',');
}


inline
uint count(const Ballots &ballots)
{
	return count(tally(ballots));
}


inline
uint count(const Tally &tally)
{
	return get<YEA>(tally) + get<NAY>(tally);
}


inline
Tally tally(const Ballots &ballots)
{
	return
	{
		count<YEA>(ballots),
		count<NAY>(ballots)
	};
}


template<Ballot type>
size_t count(const Ballots &ballots)
{
	return std::count(begin(get<type>(ballots)),end(get<type>(ballots)),',');
}


template<Ballot type>
void for_each(const Ballots &ballots,
              const std::function<void (const char *)> &func)
{
	tokens(get<type>(ballots).c_str(),",",func);
}


template<Ballot type>
bool exists(const Ballots &ballots,
            const sourceinfo_t *const &si)
{
	const auto &acct(si->su->myuser->ent.name);
	return exists<type>(ballots,acct);
}


template<Ballot type>
bool exists(const Ballots &ballots,
            const char *const &acct)
{
	return get<type>(ballots).find(acct) != std::string::npos;
}


inline
Ballot ballot(const char *const &str)
{
	char buf[16] {0};
	const auto len(strnlen(str,sizeof(buf)-1));
	std::transform(str,str+len,buf,::tolower);
	switch(hash(buf))
	{
		case hash("y"):
		case hash("yes"):
		case hash("yea"):
			return Ballot::YEA;

		case hash("n"):
		case hash("no"):
		case hash("nay"):
			return Ballot::NAY;

		case hash("v"):
		case hash("veto"):
			return Ballot::VETO;

		default:
		case hash("a"):
		case hash("abstain"):
			return Ballot::ABSTAIN;
	}
}


inline
const char *reflect(const Ballot &ballot)
{
	switch(ballot)
	{
		case Ballot::YEA:         return "YEA";
		case Ballot::NAY:         return "NAY";
		case Ballot::VETO:        return "VETO";
		case Ballot::ABSTAIN:     return "ABSTAIN";
		default:                  return "????";
	}
}
