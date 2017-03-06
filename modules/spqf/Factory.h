/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 *
 *
 * The factory model has gotten a really bad rap, often semantically perverted, and I
 * even feel dirty using the word, but just hear this out to understand what is
 * actually going on here beyond the name:
 *
 * We want the functionality of each vote type to be different, but to implement abstract
 * functionality without code duplication -- basic OO physics -- so we allow each module
 * to have its virtual overrides contained within that module's .c file itself.
 *
 * The only caveat is that we must purge the system of all votes introduced by a module when
 * the module is removed (simply removing all votes of a certain type), otherwise the vtable
 * will have dangling references into the unloaded module. Also, if the module for a vote
 * type is not loaded on startup or when votes are read from the db, then the vote can't be
 * added into memory because the abstract virtual stubs will not fulfill the vote's
 * functionality.
 */

class Factory
{
	using int_t = std::underlying_type<Type>::type;
	using Pointer = std::unique_ptr<Vote>;
	using Callback = std::function<Pointer (const uint &, const Type &, database_handle_t *)>;

	std::array<Callback,num_of<Type>()> registry;

  public:
	bool enabled(const Type &type) const;

	Pointer operator()(const uint &id, const Type &type, database_handle_t *const &db);

	template<class C> void set_callback(const Type &type, C&& callback);
	void unset_callback(const Type &type);
};


inline
void Factory::unset_callback(const Type &type)
try
{
	registry.at(int_t(type)) = nullptr;
}
catch(const std::out_of_range &e)
{
	slog(LG_ERROR,
	     SPQF_NAME": Voting submodule tried to deregister a type (%u) that doesn't exist",
	     uint(type));
}


template<class C>
void Factory::set_callback(const Type &type,
                           C&& callback)
try
{
	registry.at(int_t(type)) = callback;
}
catch(const std::out_of_range &e)
{
	slog(LG_ERROR,
	     SPQF_NAME": Voting submodule tried to register a type (%u) that doesn't exist",
	     uint(type));
}


/*
 * Throws std::out_of_range if the type doesn't exist.
 * Throws std::bad_function_call if the type has no registered callback.
 *
 * This function throws to avoid repeated checks of the type existing,
 * having a callback, or the returned pointer being a valid construction.
 */
inline
Factory::Pointer Factory::operator()(const uint &id,
                                     const Type &type,
                                     database_handle_t *const &db)
{
	const auto &cb(registry.at(int_t(type)));
	return cb(id,type,db);
}


inline
bool Factory::enabled(const Type &type)
const try
{
	return bool(registry.at(int_t(type)));
}
catch(const std::out_of_range &e)
{
	return false;
}
