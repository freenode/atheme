/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */

struct Metadata
{
	using key_parts = std::initializer_list<uint>;
	template<class R> using Closure = std::function<R (const char *const &)>;

	static constexpr size_t KEYMAX { 64  };
	static constexpr size_t VALMAX { 128 };

	template<size_t SIZE> static void concat(char (&buf)[SIZE], const key_parts &parts);

  private:
	void *targ;
	const char *ns;

  public:
	template<class R> R get(const key_parts &key, const Closure<R> &closure) const;
	const char *operator[](const key_parts &key) const;

	void set(const key_parts &key, const char *const &val);

	Metadata(void *const &targ, const char *const &ns = "");
};


inline
Metadata::Metadata(void *const &targ,
                   const char *const &ns):
targ(targ),
ns(ns)
{
}


inline
void Metadata::set(const key_parts &key,
                   const char *const &val)
{
	char mkey[KEYMAX];
	mowgli_strlcpy(mkey,ns,sizeof(mkey));
	concat(mkey,key);

	if(val)
		metadata_add(targ,mkey,val);
	else
		metadata_delete(targ,mkey);
}


inline
const char *Metadata::operator[](const key_parts &key)
const
{
	static const auto lambda([](const char *const &val)
	{
		return val;
	});

	return get<const char *>(key,lambda);
}


template<class R>
R Metadata::get(const key_parts &key,
                const Closure<R> &closure)
const
{
	char mkey[KEYMAX];
	mowgli_strlcpy(mkey,ns,sizeof(mkey));
	concat(mkey,key);

	const auto md(metadata_find(targ,mkey));
	return closure(md? md->value : nullptr);
}


template<size_t SIZE>
void Metadata::concat(char (&buf)[SIZE],
                      const key_parts &parts)
{
	size_t i(0);
	for(const auto &part : parts)
	{
		char num[16];
		snprintf(num,sizeof(num),"%u",part);
		mowgli_strlcat(buf,num,sizeof(buf));
		if(++i < parts.size())
			mowgli_strlcat(buf,":",sizeof(buf));
	}
}
