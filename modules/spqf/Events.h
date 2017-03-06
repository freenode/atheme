/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */

class Events
{
	using Event = std::tuple<time_t,std::function<void()>>;

	mowgli_eventloop_timer_t *timer;
	std::deque<Event, allocator<Event>> events;

	void set_absolute(const time_t &time);
	void set_relative(const time_t &time);
	void set_next();
	void sort();

	void operator()();
	friend void spqf_event_handler(void *) noexcept;

  public:
	template<class Handler> void at_time(const time_t &abstime, Handler&& handler);
	template<class Handler> void from_now(const time_t &reltime, Handler&& handler);

	Events();
	~Events();
};


inline
Events::Events():
timer(nullptr)
{
}


inline
Events::~Events()
{
	if(timer)
		mowgli_timer_destroy(base_eventloop,timer);
}


template<class Handler>
void Events::from_now(const time_t &reltime,
                      Handler&& handler)
{
	const auto abstime(CURRTIME + reltime);
	at_time(abstime,std::forward<Handler>(handler));
}


template<class Handler>
void Events::at_time(const time_t &abstime,
                     Handler&& handler)
{
	events.emplace_back(abstime,std::forward<Handler>(handler));
	sort();
	set_next();
}


inline
void spqf_event_handler(void *const ptr)
noexcept try
{
	Events &events(*static_cast<Events *>(ptr));
	events();
}
catch(const std::exception &e)
{
	slog(LG_ERROR,SPQF_NAME": Unhandled event callback error: %s",e.what());
}
catch(...)
{
	slog(LG_ERROR,SPQF_NAME": Unhandled event callback error");
}


inline
void Events::operator()()
{
	while(!events.empty()) try
	{
		const auto &event(events.front());
		if(get<0>(event) > CURRTIME)
			break;

		const scope pop([this] { events.pop_front(); });
		const auto &handler(get<1>(event));
		handler();
	}
	catch(const std::exception &e)
	{
		slog(LG_ERROR,SPQF_NAME": Unhandled event error: %s",e.what());
	}
	catch(...)
	{
		slog(LG_ERROR,SPQF_NAME": Unhandled event error");
	}

	if(!events.empty())
		set_next();
	else
		timer = nullptr;
}


inline
void Events::sort()
{
	std::sort(begin(events),end(events),[]
	(const Event &a, const Event &b)
	{
		return get<0>(a) < get<0>(b);
	});
}


inline
void Events::set_next()
{
	const auto &next(events.front());
	if(!timer || get<0>(next) != timer->deadline)
		set_absolute(get<0>(next));
}


inline
void Events::set_absolute(const time_t &abstime)
{
	set_relative(abstime - CURRTIME);
}


inline
void Events::set_relative(const time_t &reltime)
{
	if(timer && timer->deadline > CURRTIME)
		mowgli_timer_destroy(base_eventloop,timer);

	static const auto name(SPQF_NAME"_next_event");
	timer = mowgli_timer_add_once(base_eventloop,name,spqf_event_handler,this,reltime);
}
