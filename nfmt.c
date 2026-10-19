#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h>
#include <err.h>
#include <unistd.h>
#include <string.h>
#include "nfmt.h"

#ifdef __AFL_COMPILER
#include "tests/nfmt/create_chaos.h"

#define main new_enter
#define err(eval, ...) exit(0)
#define errx(eval, ...) exit(0)
#define malloc rand_memory_assign
#endif

int
main(int argc, char* argv[])
{
	prefix pformat;
	type tflag;
	pformat = BINAR;
	if(argc < 3)
		usage();

	if(strlen(argv[1]) <= 4)
		errx(1, "unknown form of arg %s\n", argv[1]);

	if(argv[1][0] == '-' && argv[1][1] == 't') {
		tflag = get_type(&argv[1][3]);
	} else errx(1, "malformed type flag: %s\n", argv[1]);

	if(argv[2][0] == '-' && argv[2][1] == 'f') {
		if(strlen(argv[2]) > 3) {
			if ((pformat = get_format(&argv[2][3])) == NO_FORMAT)
				errx(1, "malformed format: %s\n"
					"known: <binar> <hex> <dec> <octal>"
					"\n", &argv[2][3]);
			if (argv[3] == NULL)
				errx(1, "empty field for value\n");
			get_disp(argv[3], tflag, pformat);
		} else errx(1, "unknown format: %s\n", argv[2]);
	} else {
		if (argv[2] == NULL)
			errx(1, "empty field for value\n");
		get_disp(argv[2], tflag, pformat);
	}
	return 0;
}
#ifdef __AFL_COMPILER
#undef main
int
main(void)
{
	char		buf[255], *arv[200], *wb;
	int		argc, i, state;
	size_t		size, c_idx;
	ssize_t		rc;
	memset(buf, 0, 255);
	memset(arv, 0, 200);
	state = size = c_idx = 0;
	rc = read(0, buf, 255);
	buf[254] = '\0';
	if (rc <= 0)
		return (1);
	for (i = 0; i < rc; ++i) {
		if (state && buf[i] == ' ') {
			char		*tm;
			state = 0;
			if ((tm = malloc(size + 1)) == NULL)
				return (1);
			strlcpy(tm, wb, size + 1);
			arv[c_idx++] = tm;
			if (c_idx > 199)
				return (1);
			size = 0;
		}
		if (!isspace(buf[i] & ~(1 << 7))) {
			if (!state) {
				wb = &buf[i];
				state = 1;
			}
			size++;
		}
	}
	if (c_idx == 0)
		return (1);
	new_enter(c_idx, arv);
	return (0);
}
#endif

void
get_disp(char *val, type tp, prefix pr)
{
	fval op;
	size_t last_size;
	assert(val != NULL);
	struct parse fun[8] = {
	[UINT8_T] = { parse_u8, disp_u8 },
	[INT] =	{ parse_int, disp_int },
	[UINT32_T] = { parse_u32, disp_u32 },
	[FLOAT] = { parse_float, disp_float },
	[DOUBLE] = { parse_double, disp_double },
	[LONG_DOUBLE] = { parse_long_double, disp_long_double },
	[LONG_INT] = { parse_long_int, disp_long_int },
	[UINT64_T] = { parse_u64, disp_u64 }
	};
	if (tp == NONE)
		errx(1, "unknown type at: <-t=type>\nknown types :"
			" <uint8> | <int> | <uint32> | <float>\n\t  "
			" <double> | <ldouble> | <lint> | <uint64>\n"); 
	
	switch (pr) {
	case DEC:
		fun[tp].parse(&op, val, &last_size);
		printf("DEC : ");
		fun[tp].disp(&op, "%d");
		printf("\n");
		break;
	case HEX:
		fun[tp].parse(&op, val, &last_size);
		printf("HEX: ");
		fun[tp].disp(&op, "%X");
		printf("\n");
		break;
	case OCTAL:
		fun[tp].parse(&op, val, &last_size);
		printf("OCTAL: ");
		fun[tp].disp(&op, "%o");
		printf("\n");
		break;
	case BINAR:
		fun[tp].parse(&op, val, &last_size);
		printf("BINAR: ");
		binar((void*)&op, last_size);
		break;
}
}
			
type
get_type(char *sign)
{
	size_t i;
	struct type_table table[8] = {
		{ .name = "uint8", .ttype = UINT8_T },
		{ .name = "int", .ttype = INT },
		{ .name = "uint32", .ttype = UINT32_T },
		{ .name = "float", .ttype = FLOAT },
		{ .name = "double", .ttype = DOUBLE },
		{ .name = "ldouble", .ttype = LONG_DOUBLE },
		{ .name = "lint", .ttype = LONG_INT },
		{ .name = "uint64", .ttype = UINT64_T },
	};
	for (i = 0; i< (sizeof(table)) / sizeof(struct type_table); ++i)
		if((strcmp(table[i].name, sign)) == 0)
			return table[i].ttype;
	return NONE;
}

prefix
get_format(char *sign)
{
	size_t i;
	struct prefix_table pretable[4] = {
		{ .name= "dec", .format = DEC},
		{ .name= "hex", .format = HEX },
		{ .name= "octal", .format = OCTAL },
		{ .name= "binar", .format = BINAR }
	};
	for (i = 0; i < (sizeof(pretable) / sizeof(struct prefix_table)); ++i) {
		if ((strcmp(pretable[i].name, sign)) == 0)
			return pretable[i].format;
	}
	return (NO_FORMAT);
}

void
binar(void* pt, size_t size)
{
	unsigned char 		*bajt;
	int 			i, test;
	test = 1;
	bajt = (unsigned char *)&test;
	if (*bajt) {
		uint8_t 	shift;
		bajt = (unsigned char *)pt + size - 1;
		for (i = (int)size * 8 - 1; i >= 0; --i) {
			shift = i % 8;
			printf("%d", ((*bajt >> shift) & 0x1));
			bajt -= (shift == 0 && i > 0)? 1: 0;
		}
	} else {
		uint8_t 	shift;
		bajt = (unsigned char *)pt;
		for (i = 0; i < (int)size * 8; ++i) {
			shift = i % 8;
			printf("%d", !!(*bajt & (0200 >> shift)));
			bajt += (shift == 7)? 1: 0;
		}
	}
	printf("\n");
}

void
usage(void)
{
	fprintf(stderr, "nfmt -t=<type> [-f=<output_format>] value\n");
	exit(64);
}

void
parse_u8(fval* val, const char* fv, size_t* size)
{
	char *eptr;
	errno = 0;
	val->u8 = (uint8_t)strtoul(fv, &eptr, 0);
	if(errno == ERANGE)
		err(65, "%s", fv);
	if(*eptr != '\0' || eptr == fv)
		errx(1, "illegale value: %s\n", fv);
	*size = sizeof(uint8_t);
}
void
parse_u32(fval* val, const char* fv, size_t* size)
{
	char *eptr;
	errno = 0;
	val->u32 = (uint32_t)strtoul(fv, &eptr, 0);
	if(errno == ERANGE)
		err(65, "%s", fv);
	if(*eptr != '\0' || eptr == fv)
		errx(1, "illegale value: %s\n", fv);
	*size = sizeof(uint32_t);
}
void
parse_u64(fval* val, const char* fv, size_t* size)
{
	char *eptr;
	errno = 0;
	val->u64 = (uint64_t)strtoull(fv, &eptr, 0);
	if(errno == ERANGE)
		err(65, "%s", fv);
	if(*eptr != '\0' || eptr == fv)
		errx(1, "illegale value: %s\n", fv);
	*size = sizeof(uint64_t);
}
void
parse_int(fval* val, const char* fv, size_t* size)
{
	char *eptr;
	errno = 0;
	val->ii = (int)strtol(fv, &eptr, 0);
	if(errno == ERANGE)
		err(65, "%s", fv);
	if(*eptr != '\0' || eptr == fv)
		errx(1, "illegale value: %s\n", fv);
	*size = sizeof(int);
}
void
parse_long_int(fval* val, const char* fv, size_t* size)
{
	char *eptr;
	errno = 0;
	val->lii = (long int)strtol(fv, &eptr, 0);
	if(errno == ERANGE)
		err(65, "%s", fv);
	if(*eptr != '\0' || eptr == fv)
		errx(1, "illegale value: %s\n", fv);
	*size = sizeof(long int);
}
void
parse_float(fval* val, const char* fv, size_t* size)
{
	char *eptr;
	errno = 0;
	if(strlen(fv) >= 2) {
		if(fv[1] == 'x' || fv[1] == 'X') {
			val->u32 = (uint32_t)strtoul(fv, &eptr, 0);
		}
		else
		if(fv[0] == '0') {
			val->u32 = (uint32_t)strtoul(fv, &eptr, 0);
		} else val->fl = (float)strtof(fv, &eptr);
	} else {
		val->fl = (float)strtof(fv, &eptr);
	}
	if(errno == ERANGE)
		err(65, "value out of range: %s\n", fv);
	if(*eptr != '\0' || eptr == fv)
		errx(1, "illegale value: %s\n", fv);
	*size = sizeof(float);
}
void
parse_double(fval* val, const char* fv, size_t* size)
{
	char *eptr;
	errno = 0;
	if(strlen(fv) >= 2) {
		if(fv[1] == 'x' || fv[1] == 'X') {
			val->u64 = (uint64_t)strtoull(fv, &eptr, 0);
		}
		else
		if(fv[0] == '0') {
			val->u64 = (uint64_t)strtoull(fv, &eptr, 0);
		} else val->db = (double)strtod(fv, &eptr);
	} else {
		val->db = (double)strtod(fv, &eptr);
	}
	if(errno == ERANGE)
		err(65, "value out of range: %s\n", fv);
	if(*eptr != '\0' || eptr == fv)
		errx(1, "illegale value: %s\n", fv);
	*size = sizeof(double);
}
void
parse_long_double(fval* val, const char* fv, size_t* size)
{
	char *eptr;
	errno = 0;
	val->ldb = (long double)strtold(fv, &eptr);
	if(errno == ERANGE)
		err(65, "%s", fv);
	if(*eptr != '\0' || eptr == fv)
		errx(1, "illegale value: %s\n", fv);
	*size = sizeof(long double);
}
void
disp_u8(fval* val, const char* wh)
{
	if((strcmp(wh, "%X")) == 0) {
		printf("%" PRIX8, val->u8);
	}
        else
	if((strcmp(wh, "%o")) == 0) {
		printf("%" PRIo8, val->u8);
	} else printf("%" PRIu8, val->u8);
}
void
disp_u32(fval* val, const char* wh)
{
	if((strcmp(wh, "%X")) == 0) {
		printf("%" PRIX32, val->u32);
	}
        else
	if((strcmp(wh, "%o")) == 0) {
		printf("%" PRIo32, val->u32);
	} else printf("%" PRIu32, val->u32);
}
void
disp_u64(fval* val, const char* wh)
{
	if((strcmp(wh, "%X")) == 0) {
		printf("%" PRIX64, val->u64);
	}
        else
	if((strcmp(wh, "%o")) == 0) {
		printf("%" PRIo64, val->u64);
	} else printf("%" PRIu64, val->u64);
}
void
disp_int(fval* val, const char* wh)
{
	if((strcmp(wh, "%X")) == 0) {
		printf("%X", val->ii);
	}
        else
	if((strcmp(wh, "%o")) == 0) {
		printf("%o", val->ii);
	} else printf("%d", val->ii);
}
void
disp_long_int(fval* val, const char* wh)
{
	if((strcmp(wh, "%X")) == 0) {
		printf("%lX", val->lii);
	}
        else
	if((strcmp(wh, "%o")) == 0) {
		printf("%lo", val->lii);
	} else printf("%ld", val->lii);
}
void
disp_double(fval* val, const char* wh)
{
	if((strcmp(wh, "%X")) == 0) {
		printf("%" PRIX64, val->u64);
	}
        else
	if((strcmp(wh, "%o")) == 0) {
		printf("%" PRIo64, val->u64);
	} else printf("%f", val->db);
}
void
disp_long_double(fval* val, const char* wh)
{
	if((strcmp(wh, "%X")) == 0) {
		size_t i;
		unsigned char *bajt = (unsigned char*)&val->ldb;
		for(i = 0; i <(sizeof(long double) - 1) ; ++i) {
			printf("%02X", bajt[i]);
		}
	}
        else
	if((strcmp(wh, "%o")) == 0) {
		size_t i;
		unsigned char *bajt = (unsigned char*)&val->ldb;
		for(i = 0; i <(sizeof(long double) - 1) ; ++i) {
			printf("%03o", bajt[i]);
		}
	} else printf("%Lf", val->ldb);
}
void
disp_float(fval* val, const char* wh)
{
	if((strcmp(wh, "%X")) == 0) {
		printf("%" PRIX32, val->u32);
	}
        else
	if((strcmp(wh, "%o")) == 0) {
		printf("%" PRIo32, val->u32);
	} else printf("%f", val->fl);
}
