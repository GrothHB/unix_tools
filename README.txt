DESCRIPTION:
	This repository contains `nfmt', a numeric formatter, along
	with script builded to manage codebase while writing it.

UTILITIES:
	nfmt -t=<type> [-f=<fomrat>] num

	The `nfmt' is a acronym of `Number Formater'.
	It converts numbers to different format like:
		binary,
		hex,
		octal,
		dec.
	It includes types:
		uint8_t - uint64_t,
		int,
		float,
		double,
		long double = `ldouble',
		long int = `lint',

	c2h file

	It takes file.c and create corresponding .h file with declarations of
	functions.
	It will ask for permision is .h file already exists.
	When .h file exists and contains struct/enum definitions,
	it is safe to use. If it contains something more it may overwrite it.

	gfunc function_name file

	It will print at stdout the definition of function.
	It isnt smart enought to understand limitation of 
	function same so if there are more definitions of 
	function_name* it will alsaw print it.
TESTING:
	The tests/nfmt/ directory contains the fuzzing environment setup.
	It uses afl++ (afl-clang-fast) and ASAN to ensure memory safety.
	Corpus minimization is done via afl-cmin.
	Coverage is verified with gcov (>96% line coverage achieved).
