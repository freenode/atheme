/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */

struct Cfg
{
	static constexpr auto CHANNEL_PREFIX    { SPQF_META_CHAN_NS SPQF_META_CFG_NS  };    // ex: MDC #spqf private:SPQF:c:1:2:3 val
	static constexpr auto OPERDEF_PREFIX    { "def:" SPQF_META_CFG_NS             };    // ex: SPQFM def:c:1:2:3 val
	static constexpr auto OPERLOCK_PREFIX   { "lock:" SPQF_META_CFG_NS            };    // ex: SPQFM lock:c:1:2:3 val

	enum Ns
	{
		CHANNEL,
		OPERDEF,
		OPERLOCK,
	};

	template<class T> using time_enable = std::enable_if<std::is_arithmetic<T>() && std::is_same<T,time_t>(), T>;
	template<class T> using integer_enable = std::enable_if<std::is_arithmetic<T>() && !std::is_same<T,time_t>(), T>;
	template<class T> using string_enable = std::enable_if<!std::is_arithmetic<T>(), T>;

	template<class T> using time_type = typename time_enable<T>::type;
	template<class T> using integer_type = typename integer_enable<T>::type;
	template<class T> using string_type = typename string_enable<T>::type;

	static const char *prefix(const Ns &ns);

  private:
	using key_parts = Metadata::key_parts;

	void *targ;
	Type type;

  public:
	// Utils
	bool exists(const Doc &doc, const Var &var) const;               // channel key exists either for a type or as default
	bool specific(const Doc &doc, const Var &var) const;             // channel key exists specifically for a type
	bool defaults(const Doc &doc, const Var &var) const;             // channel key has a default
	bool operdefed(const Doc &doc, const Var &var) const;            // network operator set a default fallback
	bool operlocked(const Doc &doc, const Var &var) const;           // network operator locked this parameter to their value

	// Get typed values
	template<class T> time_type<T> get(const Doc &doc, const Var &var, const T &def_val = 0) const;
	template<class T> integer_type<T> get(const Doc &doc, const Var &var, const T &def_val = 0) const;
	template<class T = const char *> string_type<T> get(const Doc &doc, const Var &var) const;

	// Set a value in this config
	void set(const Doc &doc, const Var &var, const char *const &val);

	Cfg(void *const &targ, const Type &type = Type::DEFAULT);
};


inline
Cfg::Cfg(void *const &targ,
         const Type &type):
targ(targ),
type(type)
{
}


inline
void Cfg::set(const Doc &doc,
              const Var &var,
              const char *const &val)
{
	Metadata md(targ,prefix(CHANNEL));
	md.set({uint(type),uint(doc),uint(var)},val);
}


template<class T>
Cfg::string_type<T> Cfg::get(const Doc &doc,
                             const Var &var)
const
{
	const key_parts typekey { uint(type), uint(doc), uint(var)          };
	const key_parts defkey  { uint(Type::DEFAULT), uint(doc), uint(var) };

	const char *ret;
	const auto lam0([&ret](const char *const &val) -> bool
	{
		ret = val;
		return val;
	});

	const auto lam1([&](void *const &target, const Ns &ns)
	{
		const Metadata md(target,prefix(ns));
		return md.get<bool>(typekey,lam0) || md.get<bool>(defkey,lam0);
	});

	if(!lam1(metadata,OPERLOCK))
		if(!lam1(targ,CHANNEL))
			lam1(metadata,OPERDEF);

	return ret;
}


template<class T>
Cfg::integer_type<T> Cfg::get(const Doc &doc,
                              const Var &var,
                              const T &defv)
const
{
	const key_parts typekey { uint(type), uint(doc), uint(var)          };
	const key_parts defkey  { uint(Type::DEFAULT), uint(doc), uint(var) };

	T ret(defv);
	bool valid(false);
	const auto lam0([&ret,&valid](const char *const &val)
	{
		valid = val && isnumeric(val,Metadata::VALMAX);

		if(valid)
			ret = atoi(val);

		return valid;
	});

	const auto lam1([&](void *const &target, const Ns &ns)
	{
		const Metadata md(target,prefix(ns));
		return md.get<bool>(typekey,lam0) || md.get<bool>(defkey,lam0);
	});

	if(!lam1(metadata,OPERLOCK))
		if(!lam1(targ,CHANNEL))
			lam1(metadata,OPERDEF);

	return ret;
}


template<class T>
Cfg::time_type<T> Cfg::get(const Doc &doc,
                           const Var &var,
                           const T &defv)
const
{
	const key_parts typekey { uint(type), uint(doc), uint(var)          };
	const key_parts defkey  { uint(Type::DEFAULT), uint(doc), uint(var) };

	T ret;
	const auto lam0([&ret](const char *const &val)
	{
		ret = secs_cast(val);
		return ret >= 0;
	});

	const auto lam1([&](void *const &target, const Ns &ns)
	{
		const Metadata md(target,prefix(ns));
		return md.get<bool>(typekey,lam0) || md.get<bool>(defkey,lam0);
	});

	if(!lam1(metadata,OPERLOCK))
		if(!lam1(targ,CHANNEL))
			if(!lam1(metadata,OPERDEF))
				return defv;

	return ret;
}


inline
bool Cfg::operlocked(const Doc &doc,
                     const Var &var)
const
{
	static const auto existential([](const char *const &val) { return val != nullptr; });

	const key_parts typekey { uint(type), uint(doc), uint(var)          };
	const key_parts defkey  { uint(Type::DEFAULT), uint(doc), uint(var) };

	Metadata md(metadata,prefix(OPERLOCK));
	return md.get<bool>(typekey,existential) || md.get<bool>(defkey,existential);
}


inline
bool Cfg::operdefed(const Doc &doc,
                    const Var &var)
const
{
	static const auto existential([](const char *const &val) { return val != nullptr; });

	const key_parts typekey { uint(type), uint(doc), uint(var)          };
	const key_parts defkey  { uint(Type::DEFAULT), uint(doc), uint(var) };

	Metadata md(metadata,prefix(OPERDEF));
	return md.get<bool>(typekey,existential) || md.get<bool>(defkey,existential);
}


inline
bool Cfg::defaults(const Doc &doc,
                   const Var &var)
const
{
	static const auto existential([](const char *const &val) { return val != nullptr; });

	const key_parts defkey  { uint(Type::DEFAULT), uint(doc), uint(var) };

	Metadata md(targ,prefix(CHANNEL));
	return md.get<bool>(defkey,existential);
}


inline
bool Cfg::specific(const Doc &doc,
                   const Var &var)
const
{
	static const auto existential([](const char *const &val) { return val != nullptr; });

	const key_parts typekey { uint(type), uint(doc), uint(var)          };

	Metadata md(targ,prefix(CHANNEL));
	return md.get<bool>(typekey,existential);
}


inline
bool Cfg::exists(const Doc &doc,
                 const Var &var)
const
{
	static const auto existential([](const char *const &val) { return val != nullptr; });

	const key_parts typekey { uint(type), uint(doc), uint(var)          };
	const key_parts defkey  { uint(Type::DEFAULT), uint(doc), uint(var) };

	Metadata md(targ,prefix(CHANNEL));
	return md.get<bool>(typekey,existential) || md.get<bool>(defkey,existential);
}


inline
const char *Cfg::prefix(const Ns &ns)
{
	switch(ns)
	{
		case Ns::CHANNEL:   return CHANNEL_PREFIX;
		case Ns::OPERDEF:   return OPERDEF_PREFIX;
		case Ns::OPERLOCK:  return OPERLOCK_PREFIX;
	}
}
