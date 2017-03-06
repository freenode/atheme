/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */


struct scope
{
	const std::function<void ()> func;

	template<class Func> scope(Func&& func): func(std::forward<Func>(func)) {}
	~scope() { func(); }
};


constexpr
size_t hash(const char *const &str,
            const size_t i = 0)
{
	return !str[i]? 7681ULL : (hash(str,i+1) * 33ULL) ^ str[i];
}


inline
size_t hash(const std::string &str,
            const size_t i = 0)
{
	return i >= str.size()? 7681ULL : (hash(str,i+1) * 33ULL) ^ str.at(i);
}


inline
bool isnumeric(const char *const &val,
               const size_t n = BUFSIZE)
{
	return std::all_of(val,val+strnlen(val,n),::isdigit);
}


inline
void tokens(char *const &str,
            const char *const &delim,
            const std::function<void (char *)> &lambda)
{
	char *ctx;
	char *tok(strtok_r(str,delim,&ctx)); do
	{
		lambda(tok);
	}
	while((tok = strtok_r(NULL,delim,&ctx)) != NULL);
}


inline
void tokens(const char *const &str,
            const char *const &delim,
            const std::function<void (char *)> &lambda)
{
	char buf[BUFSIZE];
	mowgli_strlcpy(buf,str,sizeof(buf));
	tokens(buf,delim,lambda);
}


inline
void tokensa(const char *const &str,
             const char *const &delim,
             const std::function<void (char *)> &lambda)
{
	const auto free([](void *const ptr) { std::free(ptr); });
	const std::unique_ptr<char[], void(*)(void *)> cpy(strdup(str),free);
	tokens(cpy.get(),delim,lambda);
}



// For conforming enums
template<class Enum>
constexpr
typename std::underlying_type<Enum>::type
num_of()
{
	return static_cast<typename std::underlying_type<Enum>::type>(Enum::_NUM_);
}


template<class Enum>
uint index(const char *const &str,
           const size_t &n = 16)
{
	size_t i(0);
	for(; i < num_of<Enum>(); ++i)
		if(strncasecmp(str,reflect(static_cast<Enum>(i)),n) == 0)
			break;

	return i;
}


template<class Enum>
bool exists(const char *const &str,
            const size_t &n = 16)
{
	return index<Enum>(str,n) < num_of<Enum>();
}


template<class Enum>
Enum reflect(const char *const &str,
             const size_t &n = 16)
{
	const auto idx(index<Enum>(str,n));
	if(idx >= num_of<Enum>())
		throw std::out_of_range("string is not reflected by the enum");

	return static_cast<Enum>(idx);
}


template<class Enum>
void for_each(const std::function<void (const Enum &)> &func)
{
	for(size_t i(0); i < num_of<Enum>(); ++i)
		func(static_cast<Enum>(i));
}



void *operator new(std::size_t n)
{
	return mowgli_alloc(n);
}


void operator delete(void *ptr)
noexcept
{
	mowgli_free(ptr);
}


template<class T>
struct allocator
{
	using value_type         = T;
	using pointer            = T *;
	using const_pointer      = const T *;
	using reference          = T &;
	using const_reference    = const T &;
	using size_type          = std::size_t;
	using difference_type    = std::ptrdiff_t;

	size_type max_size() const
	{
		return std::numeric_limits<size_type>::max();
	}

	pointer address(reference x) const
	{
		return &x;
	}

	const_pointer address(const_reference x) const
	{
		return &x;
	}

	pointer allocate(size_type n, const_pointer hint = 0)
	{
		return reinterpret_cast<pointer>(mowgli_alloc(n * sizeof(T)));
	}

	void deallocate(pointer p, size_type n)
	{
		mowgli_free(p);
	}

	allocator() = default;
	allocator(allocator &&) = default;
	allocator(const allocator &) = default;
	template<class U> allocator(const allocator<U> &) {}
};


template<class T1,
         class T2>
bool operator==(const allocator<T1> &a, const allocator<T2> &b)
{
	return true;
}


template<class T1,
         class T2>
bool operator!=(const allocator<T1> &a, const allocator<T2> &b)
{
	return false;
}


inline
void channel_mode(user_t *const &src,
                  channel_t *const &targ,
                  const std::string &effect)
{
	char buf[effect.size()+1];
	mowgli_strlcpy(buf,effect.c_str(),sizeof(buf));

	std::vector<char *> parv;
	tokens(buf," ",[&parv](char *const &arg)
	{
		parv.emplace_back(arg);
	});

	parv.emplace_back(nullptr);
	channel_mode(src,targ,parv.size(),parv.data());
}


inline
time_t secs_cast(const char *const &dur,
                 const size_t &n = 16)
{
	if(!dur)
		return -1;

	const auto len(strnlen(dur,n));
	if(!len)
		return -1;

	if(isnumeric(dur,n))
		return atol(dur);

	if(len == 1 || !std::all_of(dur,dur+len-1,::isdigit))
		return -1;

	char *endptr;
	auto ret(strtol(dur,&endptr,10));
	const auto &postfix(dur[len-1]);
	if(&postfix != endptr)
		return -1;

	switch(postfix)
	{
		case 'y':  ret *= 12;
		case 'M':  ret *= 4;
		case 'w':  ret *= 7;
		case 'd':  ret *= 24;
		case 'h':  ret *= 60;
		case 'm':  ret *= 60;
		case 's':  return ret;
		default:   return -1;
	}
}


inline
void secs_cast(char *const &buf,
               const size_t &n,
               const time_t &seconds)
{

	const time_t years     { seconds / (60L * 60L * 24L * 7L * 4L * 12L)  };
	if(years > 1)
	{
		snprintf(buf,n,"%ld years",years);
		return;
	}
	else if(years == 1)
	{
		snprintf(buf,n,"1 year");
		return;
	}

	const time_t months    { seconds / (60L * 60L * 24L * 7L * 4L)        };
	if(months > 1)
	{
		snprintf(buf,n,"%ld months",months);
		return;
	}
	else if(months == 1)
	{
		snprintf(buf,n,"1 month");
		return;
	}

	const time_t weeks     { seconds / (60L * 60L * 24L * 7L)             };
	if(weeks > 1)
	{
		snprintf(buf,n,"%ld weeks",weeks);
		return;
	}
	else if(weeks == 1)
	{
		snprintf(buf,n,"1 week");
		return;
	}

	const time_t days      { seconds / (60L * 60L * 24L)                  };
	if(days > 1)
	{
		snprintf(buf,n,"%ld days",days);
		return;
	}
	else if(days == 1)
	{
		snprintf(buf,n,"1 day");
		return;
	}

	const time_t hours     { seconds / (60L * 60L)                        };
	if(hours > 1)
	{
		snprintf(buf,n,"%ld hours",hours);
		return;
	}
	else if(hours == 1)
	{
		snprintf(buf,n,"1 hour");
		return;
	}

	const time_t minutes   { seconds / (60L)                              };
	if(minutes > 1)
	{
		snprintf(buf,n,"%ld minutes",minutes);
		return;
	}
	else if(minutes == 1)
	{
		snprintf(buf,n,"1 minute");
		return;
	}

	if(seconds == 1)
		snprintf(buf,n,"1 second");
	else
		snprintf(buf,n,"%ld seconds",seconds);
}
