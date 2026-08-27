/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 53 "engines/director/lingo/lingo-gr.y"

#define FORBIDDEN_SYMBOL_EXCEPTION_FILE
#define FORBIDDEN_SYMBOL_EXCEPTION_fprintf
#define FORBIDDEN_SYMBOL_EXCEPTION_fwrite
#define FORBIDDEN_SYMBOL_EXCEPTION_fread
#define FORBIDDEN_SYMBOL_EXCEPTION_stdin
#define FORBIDDEN_SYMBOL_EXCEPTION_stdout
#define FORBIDDEN_SYMBOL_EXCEPTION_stderr
#define FORBIDDEN_SYMBOL_EXCEPTION_exit
#define FORBIDDEN_SYMBOL_EXCEPTION_getc

#include "common/endian.h"
#include "common/hash-str.h"
#include "common/rect.h"

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-ast.h"
#include "director/lingo/lingo-code.h"
#include "director/lingo/lingo-codegen.h"
#include "director/lingo/lingo-gr.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-the.h"

extern int yylex();

using namespace Director;

// if we have the trim garbage flag on, and we encounter
// an unwelcome surprise in an expression, use everything up until
// the first failing token.
#define TRIM_GARBAGE(src, target) \
	if (g_lingo->_compiler->_trimGarbage) { \
		warning("Trimming garbage and continuing.");	\
		src = target;	\
		yyerrok; \
	} else { \
		src = nullptr; \
		YYABORT; \
	}	\


static void yyerror(const char *s) {
	LingoCompiler *compiler = g_lingo->_compiler;
	compiler->_hadError = true;
	warning("%s  LINGO: %s at line %d col %d in %s id: %d",
		(g_director->_noFatalLingoError ? "####" : "######################"), s, compiler->_linenumber, compiler->_colnumber, scriptType2str(compiler->_assemblyContext->_scriptType),
		compiler->_assemblyContext->_id);
	if (compiler->_lines[2] != compiler->_lines[1])
		warning("# %3d: %s", compiler->_linenumber - 2, Common::String(compiler->_lines[2], compiler->_lines[1] - 1).c_str());

	if (compiler->_lines[1] != compiler->_lines[0])
		warning("# %3d: %s", compiler->_linenumber - 1, Common::String(compiler->_lines[1], compiler->_lines[0] - 1).c_str());

	const char *ptr = compiler->_lines[0];

	while (*ptr && *ptr != '\n')
		ptr++;

	warning("# %3d: %s", compiler->_linenumber, Common::String(compiler->_lines[0], ptr).c_str());

	Common::String arrow;
	for (uint i = 0; i < compiler->_colnumber; i++)
		arrow += ' ';

	warning("#      %s^ about here", arrow.c_str());
}

static void checkEnd(Common::String *token, Common::String *expect, bool required) {
	if (required) {
		if (token->compareToIgnoreCase(*expect)) {
			Common::String err = Common::String::format("end mismatch. Expected %s but got %s", expect->c_str(), token->c_str());
			yyerror(err.c_str());
		}
	}
}


#line 150 "engines/director/lingo/lingo-gr.cpp"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "lingo-gr.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_tUNARY = 3,                     /* tUNARY  */
  YYSYMBOL_tINT = 4,                       /* tINT  */
  YYSYMBOL_tFLOAT = 5,                     /* tFLOAT  */
  YYSYMBOL_tVARID = 6,                     /* tVARID  */
  YYSYMBOL_tSTRING = 7,                    /* tSTRING  */
  YYSYMBOL_tSYMBOL = 8,                    /* tSYMBOL  */
  YYSYMBOL_tENDCLAUSE = 9,                 /* tENDCLAUSE  */
  YYSYMBOL_tCAST = 10,                     /* tCAST  */
  YYSYMBOL_tFIELD = 11,                    /* tFIELD  */
  YYSYMBOL_tSCRIPT = 12,                   /* tSCRIPT  */
  YYSYMBOL_tWINDOW = 13,                   /* tWINDOW  */
  YYSYMBOL_tMEMBER = 14,                   /* tMEMBER  */
  YYSYMBOL_tCASTLIB = 15,                  /* tCASTLIB  */
  YYSYMBOL_tDELETE = 16,                   /* tDELETE  */
  YYSYMBOL_tDOWN = 17,                     /* tDOWN  */
  YYSYMBOL_tELSE = 18,                     /* tELSE  */
  YYSYMBOL_tEXIT = 19,                     /* tEXIT  */
  YYSYMBOL_tFRAME = 20,                    /* tFRAME  */
  YYSYMBOL_tGLOBAL = 21,                   /* tGLOBAL  */
  YYSYMBOL_tGO = 22,                       /* tGO  */
  YYSYMBOL_tHILITE = 23,                   /* tHILITE  */
  YYSYMBOL_tIF = 24,                       /* tIF  */
  YYSYMBOL_tIN = 25,                       /* tIN  */
  YYSYMBOL_tINTO = 26,                     /* tINTO  */
  YYSYMBOL_tMACRO = 27,                    /* tMACRO  */
  YYSYMBOL_tRETURN = 28,                   /* tRETURN  */
  YYSYMBOL_tMOVIE = 29,                    /* tMOVIE  */
  YYSYMBOL_tNEXT = 30,                     /* tNEXT  */
  YYSYMBOL_tOF = 31,                       /* tOF  */
  YYSYMBOL_tPREVIOUS = 32,                 /* tPREVIOUS  */
  YYSYMBOL_tPUT = 33,                      /* tPUT  */
  YYSYMBOL_tREPEAT = 34,                   /* tREPEAT  */
  YYSYMBOL_tSET = 35,                      /* tSET  */
  YYSYMBOL_tTHEN = 36,                     /* tTHEN  */
  YYSYMBOL_tTO = 37,                       /* tTO  */
  YYSYMBOL_tWHEN = 38,                     /* tWHEN  */
  YYSYMBOL_tWITH = 39,                     /* tWITH  */
  YYSYMBOL_tWHILE = 40,                    /* tWHILE  */
  YYSYMBOL_tFACTORY = 41,                  /* tFACTORY  */
  YYSYMBOL_tOPEN = 42,                     /* tOPEN  */
  YYSYMBOL_tPLAY = 43,                     /* tPLAY  */
  YYSYMBOL_tINSTANCE = 44,                 /* tINSTANCE  */
  YYSYMBOL_tGE = 45,                       /* tGE  */
  YYSYMBOL_tLE = 46,                       /* tLE  */
  YYSYMBOL_tEQ = 47,                       /* tEQ  */
  YYSYMBOL_tNEQ = 48,                      /* tNEQ  */
  YYSYMBOL_tAND = 49,                      /* tAND  */
  YYSYMBOL_tOR = 50,                       /* tOR  */
  YYSYMBOL_tNOT = 51,                      /* tNOT  */
  YYSYMBOL_tMOD = 52,                      /* tMOD  */
  YYSYMBOL_tAFTER = 53,                    /* tAFTER  */
  YYSYMBOL_tBEFORE = 54,                   /* tBEFORE  */
  YYSYMBOL_tCONCAT = 55,                   /* tCONCAT  */
  YYSYMBOL_tCONTAINS = 56,                 /* tCONTAINS  */
  YYSYMBOL_tSTARTS = 57,                   /* tSTARTS  */
  YYSYMBOL_tCHAR = 58,                     /* tCHAR  */
  YYSYMBOL_tCHARS = 59,                    /* tCHARS  */
  YYSYMBOL_tITEM = 60,                     /* tITEM  */
  YYSYMBOL_tITEMS = 61,                    /* tITEMS  */
  YYSYMBOL_tLINE = 62,                     /* tLINE  */
  YYSYMBOL_tLINES = 63,                    /* tLINES  */
  YYSYMBOL_tWORD = 64,                     /* tWORD  */
  YYSYMBOL_tWORDS = 65,                    /* tWORDS  */
  YYSYMBOL_tABBREVIATED = 66,              /* tABBREVIATED  */
  YYSYMBOL_tABBREV = 67,                   /* tABBREV  */
  YYSYMBOL_tABBR = 68,                     /* tABBR  */
  YYSYMBOL_tLONG = 69,                     /* tLONG  */
  YYSYMBOL_tSHORT = 70,                    /* tSHORT  */
  YYSYMBOL_tDATE = 71,                     /* tDATE  */
  YYSYMBOL_tLAST = 72,                     /* tLAST  */
  YYSYMBOL_tMENU = 73,                     /* tMENU  */
  YYSYMBOL_tMENUS = 74,                    /* tMENUS  */
  YYSYMBOL_tMENUITEM = 75,                 /* tMENUITEM  */
  YYSYMBOL_tMENUITEMS = 76,                /* tMENUITEMS  */
  YYSYMBOL_tNUMBER = 77,                   /* tNUMBER  */
  YYSYMBOL_tTHE = 78,                      /* tTHE  */
  YYSYMBOL_tTIME = 79,                     /* tTIME  */
  YYSYMBOL_tXTRAS = 80,                    /* tXTRAS  */
  YYSYMBOL_tCASTLIBS = 81,                 /* tCASTLIBS  */
  YYSYMBOL_tSOUND = 82,                    /* tSOUND  */
  YYSYMBOL_tSPRITE = 83,                   /* tSPRITE  */
  YYSYMBOL_tINTERSECTS = 84,               /* tINTERSECTS  */
  YYSYMBOL_tWITHIN = 85,                   /* tWITHIN  */
  YYSYMBOL_tTELL = 86,                     /* tTELL  */
  YYSYMBOL_tPROPERTY = 87,                 /* tPROPERTY  */
  YYSYMBOL_tON = 88,                       /* tON  */
  YYSYMBOL_tMETHOD = 89,                   /* tMETHOD  */
  YYSYMBOL_tENDIF = 90,                    /* tENDIF  */
  YYSYMBOL_tENDREPEAT = 91,                /* tENDREPEAT  */
  YYSYMBOL_tENDTELL = 92,                  /* tENDTELL  */
  YYSYMBOL_tASSERTERROR = 93,              /* tASSERTERROR  */
  YYSYMBOL_94_ = 94,                       /* '<'  */
  YYSYMBOL_95_ = 95,                       /* '>'  */
  YYSYMBOL_96_ = 96,                       /* '&'  */
  YYSYMBOL_97_ = 97,                       /* '+'  */
  YYSYMBOL_98_ = 98,                       /* '-'  */
  YYSYMBOL_99_ = 99,                       /* '*'  */
  YYSYMBOL_100_ = 100,                     /* '/'  */
  YYSYMBOL_101_n_ = 101,                   /* '\n'  */
  YYSYMBOL_102_ = 102,                     /* ','  */
  YYSYMBOL_103_ = 103,                     /* '('  */
  YYSYMBOL_104_ = 104,                     /* ')'  */
  YYSYMBOL_105_ = 105,                     /* '.'  */
  YYSYMBOL_106_ = 106,                     /* '['  */
  YYSYMBOL_107_ = 107,                     /* ']'  */
  YYSYMBOL_108_ = 108,                     /* ':'  */
  YYSYMBOL_YYACCEPT = 109,                 /* $accept  */
  YYSYMBOL_script = 110,                   /* script  */
  YYSYMBOL_scriptpartlist = 111,           /* scriptpartlist  */
  YYSYMBOL_scriptpart = 112,               /* scriptpart  */
  YYSYMBOL_macro = 113,                    /* macro  */
  YYSYMBOL_factory = 114,                  /* factory  */
  YYSYMBOL_method = 115,                   /* method  */
  YYSYMBOL_methodlist = 116,               /* methodlist  */
  YYSYMBOL_nonemptymethodlist = 117,       /* nonemptymethodlist  */
  YYSYMBOL_methodlistline = 118,           /* methodlistline  */
  YYSYMBOL_handler = 119,                  /* handler  */
  YYSYMBOL_endargdef = 120,                /* endargdef  */
  YYSYMBOL_CMDID = 121,                    /* CMDID  */
  YYSYMBOL_ID = 122,                       /* ID  */
  YYSYMBOL_idlist = 123,                   /* idlist  */
  YYSYMBOL_nonemptyidlist = 124,           /* nonemptyidlist  */
  YYSYMBOL_stmt = 125,                     /* stmt  */
  YYSYMBOL_stmt_insideif = 126,            /* stmt_insideif  */
  YYSYMBOL_stmtoneliner = 127,             /* stmtoneliner  */
  YYSYMBOL_proc = 128,                     /* proc  */
  YYSYMBOL_cmdargs = 129,                  /* cmdargs  */
  YYSYMBOL_trailingcomma = 130,            /* trailingcomma  */
  YYSYMBOL_frameargs = 131,                /* frameargs  */
  YYSYMBOL_asgn = 132,                     /* asgn  */
  YYSYMBOL_to = 133,                       /* to  */
  YYSYMBOL_definevars = 134,               /* definevars  */
  YYSYMBOL_ifstmt = 135,                   /* ifstmt  */
  YYSYMBOL_ifelsestmt = 136,               /* ifelsestmt  */
  YYSYMBOL_endif = 137,                    /* endif  */
  YYSYMBOL_loop = 138,                     /* loop  */
  YYSYMBOL_tell = 139,                     /* tell  */
  YYSYMBOL_when = 140,                     /* when  */
  YYSYMBOL_stmtlist = 141,                 /* stmtlist  */
  YYSYMBOL_nonemptystmtlist = 142,         /* nonemptystmtlist  */
  YYSYMBOL_stmtlistline = 143,             /* stmtlistline  */
  YYSYMBOL_stmtlist_insideif = 144,        /* stmtlist_insideif  */
  YYSYMBOL_nonemptystmtlist_insideif = 145, /* nonemptystmtlist_insideif  */
  YYSYMBOL_stmtlistline_insideif = 146,    /* stmtlistline_insideif  */
  YYSYMBOL_simpleexpr_nounarymath = 147,   /* simpleexpr_nounarymath  */
  YYSYMBOL_var = 148,                      /* var  */
  YYSYMBOL_varorchunk = 149,               /* varorchunk  */
  YYSYMBOL_varorthe = 150,                 /* varorthe  */
  YYSYMBOL_chunk = 151,                    /* chunk  */
  YYSYMBOL_chunktype = 152,                /* chunktype  */
  YYSYMBOL_object = 153,                   /* object  */
  YYSYMBOL_refargs = 154,                  /* refargs  */
  YYSYMBOL_the = 155,                      /* the  */
  YYSYMBOL_theobj = 156,                   /* theobj  */
  YYSYMBOL_menu = 157,                     /* menu  */
  YYSYMBOL_thedatetime = 158,              /* thedatetime  */
  YYSYMBOL_thenumberof = 159,              /* thenumberof  */
  YYSYMBOL_inof = 160,                     /* inof  */
  YYSYMBOL_writablethe = 161,              /* writablethe  */
  YYSYMBOL_writabletheobj = 162,           /* writabletheobj  */
  YYSYMBOL_list = 163,                     /* list  */
  YYSYMBOL_proplist = 164,                 /* proplist  */
  YYSYMBOL_proppair = 165,                 /* proppair  */
  YYSYMBOL_unarymath = 166,                /* unarymath  */
  YYSYMBOL_simpleexpr = 167,               /* simpleexpr  */
  YYSYMBOL_expr = 168,                     /* expr  */
  YYSYMBOL_expr_nounarymath = 169,         /* expr_nounarymath  */
  YYSYMBOL_expr_noeq = 170,                /* expr_noeq  */
  YYSYMBOL_sprite = 171,                   /* sprite  */
  YYSYMBOL_exprlist = 172,                 /* exprlist  */
  YYSYMBOL_nonemptyexprlist = 173          /* nonemptyexprlist  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if 1

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* 1 */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  202
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   4886

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  109
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  65
/* YYNRULES -- Number of rules.  */
#define YYNRULES  373
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  743

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   348


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     101,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    96,     2,
     103,   104,    99,    97,   102,    98,   105,   100,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   108,     2,
      94,     2,    95,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   106,     2,   107,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   225,   225,   227,   233,   240,   241,   242,   243,   244,
     245,   274,   275,   280,   281,   284,   285,   288,   289,   292,
     298,   305,   306,   307,   312,   316,   320,   322,   326,   327,
     328,   335,   336,   337,   338,   339,   340,   341,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,   380,   381,   382,   383,
     386,   387,   388,   389,   390,   391,   392,   393,   394,   395,
     396,   397,   398,   399,   400,   401,   402,   403,   404,   405,
     408,   409,   410,   413,   417,   427,   428,   431,   432,   433,
     434,   435,   436,   439,   440,   441,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
     458,   463,   464,   465,   466,   467,   468,   469,   470,   471,
     472,   473,   474,   475,   476,   477,   480,   483,   488,   492,
     498,   503,   506,   511,   515,   521,   528,   528,   534,   539,
     545,   551,   557,   565,   566,   567,   568,   569,   570,   571,
     572,   575,   575,   577,   578,   579,   580,   581,   582,   585,
     589,   591,   595,   601,   605,   609,   613,   620,   622,   624,
     626,   628,   630,   632,   634,   636,   640,   644,   646,   650,
     652,   653,   657,   663,   670,   671,   674,   675,   679,   685,
     692,   693,   699,   700,   701,   702,   703,   704,   705,   706,
     712,   717,   723,   724,   725,   726,   727,   728,   731,   733,
     734,   737,   738,   741,   742,   747,   748,   753,   754,   755,
     757,   759,   761,   763,   765,   767,   769,   771,   774,   775,
     776,   777,   780,   781,   784,   789,   792,   797,   803,   804,
     805,   806,   807,   810,   811,   812,   813,   814,   817,   819,
     820,   821,   822,   823,   824,   825,   826,   827,   828,   832,
     833,   834,   835,   836,   837,   838,   839,   842,   842,   844,
     845,   846,   853,   854,   855,   856,   857,   860,   861,   862,
     868,   872,   875,   880,   881,   882,   883,   884,   887,   888,
     891,   892,   896,   897,   898,   899,   900,   901,   902,   903,
     904,   905,   906,   907,   908,   909,   910,   911,   912,   913,
     914,   921,   922,   923,   924,   925,   926,   927,   928,   929,
     930,   931,   932,   933,   934,   935,   936,   937,   938,   939,
     942,   943,   944,   945,   946,   947,   948,   949,   950,   951,
     952,   953,   954,   955,   956,   957,   958,   959,   962,   963,
     966,   967,   970,   974
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if 1
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  static const char *const yy_sname[] =
  {
  "end of file", "error", "invalid token", "tUNARY", "tINT", "tFLOAT",
  "tVARID", "tSTRING", "tSYMBOL", "tENDCLAUSE", "tCAST", "tFIELD",
  "tSCRIPT", "tWINDOW", "tMEMBER", "tCASTLIB", "tDELETE", "tDOWN", "tELSE",
  "tEXIT", "tFRAME", "tGLOBAL", "tGO", "tHILITE", "tIF", "tIN", "tINTO",
  "tMACRO", "tRETURN", "tMOVIE", "tNEXT", "tOF", "tPREVIOUS", "tPUT",
  "tREPEAT", "tSET", "tTHEN", "tTO", "tWHEN", "tWITH", "tWHILE",
  "tFACTORY", "tOPEN", "tPLAY", "tINSTANCE", "tGE", "tLE", "tEQ", "tNEQ",
  "tAND", "tOR", "tNOT", "tMOD", "tAFTER", "tBEFORE", "tCONCAT",
  "tCONTAINS", "tSTARTS", "tCHAR", "tCHARS", "tITEM", "tITEMS", "tLINE",
  "tLINES", "tWORD", "tWORDS", "tABBREVIATED", "tABBREV", "tABBR", "tLONG",
  "tSHORT", "tDATE", "tLAST", "tMENU", "tMENUS", "tMENUITEM", "tMENUITEMS",
  "tNUMBER", "tTHE", "tTIME", "tXTRAS", "tCASTLIBS", "tSOUND", "tSPRITE",
  "tINTERSECTS", "tWITHIN", "tTELL", "tPROPERTY", "tON", "tMETHOD",
  "tENDIF", "tENDREPEAT", "tENDTELL", "tASSERTERROR", "'<'", "'>'", "'&'",
  "'+'", "'-'", "'*'", "'/'", "'\\n'", "','", "'('", "')'", "'.'", "'['",
  "']'", "':'", "$accept", "script", "scriptpartlist", "scriptpart",
  "macro", "factory", "method", "methodlist", "nonemptymethodlist",
  "methodlistline", "handler", "endargdef", "CMDID", "ID", "idlist",
  "nonemptyidlist", "stmt", "stmt_insideif", "stmtoneliner", "proc",
  "cmdargs", "trailingcomma", "frameargs", "asgn", "to", "definevars",
  "ifstmt", "ifelsestmt", "endif", "loop", "tell", "when", "stmtlist",
  "nonemptystmtlist", "stmtlistline", "stmtlist_insideif",
  "nonemptystmtlist_insideif", "stmtlistline_insideif",
  "simpleexpr_nounarymath", "var", "varorchunk", "varorthe", "chunk",
  "chunktype", "object", "refargs", "the", "theobj", "menu", "thedatetime",
  "thenumberof", "inof", "writablethe", "writabletheobj", "list",
  "proplist", "proppair", "unarymath", "simpleexpr", "expr",
  "expr_nounarymath", "expr_noeq", "sprite", "exprlist",
  "nonemptyexprlist", YY_NULLPTR
  };
  return yy_sname[yysymbol];
}
#endif

#define YYPACT_NINF (-626)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-269)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    3574,  -626,  4035,  -626,  -626,  -626,  -626,  -626,  -626,   172,
    -626,     8,  -626,  4035,  2456,   172,  2559,  -626,  -626,  4035,
    1115,  -626,     5,  -626,  -626,  2662,    64,  2765,  -626,   -82,
    -626,  -626,  4035,  2662,  2456,  4035,  -626,  -626,  -626,  -626,
    -626,  -626,  -626,  -626,  -626,  -626,  -626,  -626,  -626,  -626,
    -626,  -626,  -626,  -626,  -626,  -626,  -626,  -626,  -626,  -626,
    -626,  -626,  -626,  2559,  4035,  4035,   -71,  4289,  -626,    68,
    3574,  -626,  -626,  -626,  -626,  2662,  -626,  -626,  -626,  -626,
    -626,  -626,  -626,  -626,  -626,  -626,  -626,  -626,  -626,  -626,
    -626,  -626,  -626,  -626,  -626,  -626,  -626,  -626,  -626,  -626,
    -626,  -626,  -626,  -626,  -626,  -626,  -626,  -626,  -626,  -626,
    -626,   -43,  -626,  -626,  2868,  2868,  2868,  2868,  2559,  2559,
    2559,  2559,   -25,    14,   -50,    19,  -626,  -626,    20,   -31,
    -626,  -626,  -626,  -626,  2868,  2868,  2868,  2868,  2868,  2868,
    2559,  2559,  2971,  2559,  2559,  2559,  2559,  4120,  2559,  2971,
    2971,  1941,  1218,   -29,    23,    24,   -27,  -626,  -626,  -626,
    -626,  -626,  -626,  -626,  -626,  -626,  1323,  -626,    27,  2559,
    4486,  4035,    -3,  -626,   406,    28,    34,  1426,  4035,  2559,
    2559,  2559,  4120,  2559,  2559,   -24,    -4,  -626,    18,  -626,
    -626,    35,    39,  1529,    40,    43,    44,   225,    47,  4035,
    -626,  -626,  -626,  -626,    48,  1632,  -626,  4035,  2044,  -626,
      76,  -626,  -626,  -626,    81,  -626,   922,  1002,  4338,  4360,
     244,    25,  -626,  -626,    33,  -626,    62,  -626,  4035,  -626,
    -626,  1735,  4717,  -626,   -10,     4,    15,    26,    31,   244,
     140,   142,  4704,  -626,  -626,  -626,  3383,  4508,     6,    82,
      88,    89,    69,   -15,    -7,  -626,  4717,    93,    86,  2147,
     100,  -626,   101,  -626,  4035,   176,  2559,  2559,  2559,  2559,
    2559,  2559,  2559,  2559,  2559,  2559,  2559,  2559,  2559,  2559,
    2559,  2559,  2559,  2559,  -626,   -27,  4602,  -626,   107,  -626,
    4542,  3481,    49,  -626,   108,  -626,   111,  -626,   113,  -626,
    4205,  4205,  4205,    13,   576,  -626,   641,  -626,  4452,   -20,
     641,   641,  -626,  -626,  2559,  4035,   124,     7,   132,  -626,
    2559,   138,  -626,   143,  -626,   148,  -626,   164,  4289,  3666,
     167,  -626,    51,   183,  -626,  -626,  -626,  4521,   209,   214,
    2971,  2559,  2971,  2559,  2971,  2559,  2971,  2559,  -626,  -626,
    -626,  -626,    59,  -626,  -626,  -626,  -626,   227,  4773,  -626,
    -626,  -626,  -626,  -626,  -626,  -626,  -626,  -626,  -626,  1838,
    3074,  2971,  2971,  4615,  2250,  -626,  2559,  2559,  2559,  2559,
    -626,  2559,  3177,  -626,  -626,  2559,  -626,  3383,   184,  -626,
    -626,  -626,  2559,   123,   123,   123,   123,   909,   909,  -626,
     161,   123,   123,   123,   123,   161,    80,    80,  -626,  -626,
     184,  2559,  2559,  2559,  2559,  2559,  2559,  2559,  2559,  2559,
    2559,  2559,  2559,  2559,  2559,  2559,  2559,  2559,  2559,  -626,
    -626,   191,  3942,   267,   193,  3666,  -626,  -626,  -626,   -25,
    -626,  -626,    53,  -626,    54,    55,  2559,  2559,   197,  3666,
    2559,  2559,  2559,  2559,  2559,  2559,  2559,  2559,  2559,  2559,
    2559,  2559,  2559,  2559,  2559,  2559,   230,  3280,   611,  -626,
       7,  4035,  4035,  -626,  -626,  -626,     7,  -626,  -626,  4686,
    -626,  -626,  -626,  3666,  -626,  -626,  -626,   215,  3666,  -626,
    -626,   217,  3666,  -626,  2353,  2971,  2971,  -626,  1015,  -626,
    4373,  -626,  4395,  -626,  4430,  -626,  -626,  2971,  2559,    59,
      59,    59,    59,  2971,  -626,  2971,    59,  -626,  -626,  2971,
    2971,  -626,  -626,  -626,  -626,  -626,  -626,  2559,   223,  -626,
     184,  4717,  4717,  4717,  4717,  4717,  -626,  4717,  4717,  4628,
    2559,   226,  4717,  -626,   123,   123,   123,   123,   909,   909,
    -626,   161,   123,   123,   123,   123,   161,    80,    80,  -626,
    -626,   184,  3942,  -626,  -626,     9,  3942,  -626,  3758,  3666,
    -626,   231,  -626,   232,  -626,   233,  -626,   722,   891,  3666,
     222,   155,   155,   155,  4786,  4786,  -626,   212,   155,   155,
     155,   155,   212,   179,   179,  -626,  -626,  2559,  2559,  2559,
    2559,  2559,  -626,   236,   241,  -626,  -626,    57,  4035,  -626,
    -626,   251,   243,  -626,  3666,   336,  -626,   184,  -626,  -626,
    2971,  2971,  2971,  2971,  -626,  4717,  2971,  2971,  2971,  2971,
    -626,   349,   309,  -626,  -626,   184,  -626,   279,  2559,   280,
    -626,  -626,   312,  3850,   302,  -626,  -626,  3942,  -626,  -626,
    -626,  -626,  -626,   303,  3666,   368,  2559,   315,   307,   641,
     304,   349,   305,   308,  -626,  -626,    56,   311,  -626,   405,
    4035,   316,  -626,  -626,  -626,  -626,  -626,  -626,  -626,  -626,
     342,  2971,  -626,   317,  -626,   184,  -626,  -626,  3942,  -626,
    -626,   312,  3666,   325,  2559,   747,   318,  -626,   321,  3666,
    -626,  4035,    67,  -626,  2971,  -626,   319,   312,  -626,   333,
     324,   804,   326,  3666,  -626,  3666,  -626,    83,  -626,  -626,
    -626,  -626,   327,  -626,   328,  3666,  3666,   335,  -626,  -626,
    -626,  3666,   339,   340,   332,   343,   338,   344,  -626,   345,
    -626,  -626,  -626
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       0,    31,    28,    37,    44,    66,    75,    56,    38,    42,
      43,     0,    45,   100,   146,    46,     0,    47,    49,     0,
       0,    60,    61,    63,    64,   146,    65,     0,    73,     0,
      76,    74,     0,   146,   146,   100,    35,    36,    39,    40,
      50,    51,    53,    54,    78,    79,    32,    33,    34,    55,
      68,    41,    52,    57,    58,    59,    62,    71,    72,    69,
      70,    48,    77,     0,   100,     0,     0,    67,     5,     0,
       2,     3,     6,     7,     8,   146,     9,   105,   107,   113,
     114,   115,   108,   109,   110,   111,   112,    82,    42,    81,
      83,    85,    86,    46,    87,    89,    96,    61,    95,    65,
      97,    99,    84,    92,    93,    88,    98,    94,    91,    90,
      67,     0,    80,    29,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   135,   103,     0,   101,
     212,   213,   215,   214,    37,    44,    66,    75,    56,    38,
      45,    60,     0,    39,    50,    53,    78,    71,    70,     0,
       0,     0,   370,   228,     0,     0,   310,   223,   224,   225,
     226,   261,   262,   227,   311,   312,   156,   313,     0,     0,
       0,   100,     0,   137,     0,     0,     0,   156,     0,     0,
      57,    58,    71,    69,    70,   231,     0,   232,     0,   292,
     199,     0,     0,   156,     0,     0,     0,     0,     0,   100,
     106,   145,     1,     4,     0,   156,    10,     0,     0,   235,
     254,   233,   254,   237,   254,   238,     0,     0,     0,     0,
       0,     0,   141,   136,     0,   133,     0,   173,   102,   252,
     253,   158,   159,   216,    32,    33,    34,    55,    68,    52,
      62,   258,     0,   308,   309,   151,   223,     0,   212,   213,
     215,   214,     0,   228,     0,   300,   372,     0,   371,     0,
       0,   120,     0,   122,     0,    63,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   157,   147,   331,   156,   332,     0,   143,
       0,     0,     0,   138,     0,   139,     0,   131,     0,   118,
      49,    35,    36,     0,     0,   350,   293,   351,     0,   289,
     295,   296,   171,   172,     0,     0,     0,    17,     0,   128,
      76,     0,   124,     0,   126,     0,   177,     0,     0,   200,
       0,   175,     0,     0,   116,    30,   255,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   248,   250,
     251,   249,     0,   142,   134,   174,   104,    63,   162,   269,
     270,   271,   272,   273,   274,   275,   276,   277,   278,     0,
       0,     0,     0,   156,     0,   222,     0,     0,     0,     0,
     298,     0,     0,   299,   297,     0,   217,   223,   156,   121,
     123,   221,     0,   323,   324,   321,   322,   325,   326,   318,
     328,   329,   330,   320,   319,   327,   314,   315,   316,   317,
     156,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   157,   149,
     144,     0,   206,   179,     0,   200,   140,   132,   119,    71,
     228,   229,     0,   230,     0,     0,     0,     0,     0,   200,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   291,
      17,    28,     0,    21,    22,    13,    18,    19,   129,     0,
     125,   127,   178,   200,   196,   204,   205,     0,   201,   202,
     176,     0,   200,   117,     0,     0,     0,   239,     0,   243,
       0,   245,     0,   241,     0,   287,   288,     0,     0,    40,
      51,    54,    79,    57,   284,    58,    59,   285,   286,    69,
      70,   260,   264,   263,   259,   368,   369,   157,     0,   152,
     156,   306,   307,   305,   303,   304,   301,   302,   373,   156,
     157,     0,   161,   148,   342,   343,   340,   341,   344,   345,
     337,   347,   348,   349,   339,   338,   346,   333,   334,   335,
     336,   156,   206,   210,   211,   186,   207,   208,     0,   200,
      11,     0,   163,     0,   165,     0,   167,     0,     0,   200,
       0,   360,   361,   359,   362,   363,   356,   365,   366,   367,
     358,   357,   364,   352,   353,   354,   355,     0,    57,    58,
      69,    70,   290,   292,     0,   169,    14,     0,   100,    20,
     130,     0,     0,   203,   200,    26,   256,   156,   234,   236,
       0,     0,     0,     0,   247,   160,     0,     0,     0,     0,
     268,     0,     0,   266,   267,   156,   154,     0,   157,     0,
     218,   150,   186,     0,     0,   180,   209,   206,   182,    12,
     164,   166,   168,     0,   200,     0,     0,     0,     0,   294,
     350,   350,   350,   350,   170,    23,     0,     0,   197,    27,
      28,     0,   240,   244,   246,   242,   279,   281,   282,   280,
       0,     0,   283,     0,   153,   156,   219,   181,   206,   184,
     187,   186,   200,     0,     0,     0,     0,   188,     0,   200,
     198,    28,     0,   257,     0,   155,     0,   186,   183,     0,
       0,     0,     0,   200,   189,   200,    15,     0,    24,   265,
     220,   185,     0,   194,     0,   200,   200,     0,    16,    25,
     195,   200,     0,     0,     0,     0,     0,     0,   190,     0,
     192,   191,   193
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -626,  -626,  -626,   367,  -626,  -626,  -626,   -23,  -626,   -33,
    -626,  -466,     0,    -1,   -32,  -626,     2,  -419,   -61,  -626,
     128,  -199,   416,  -626,  -626,  -626,  -626,  -626,  -625,  -626,
    -626,  -626,    50,  -626,   -40,  -555,  -626,  -109,  -140,   -13,
    -136,  -626,    -5,  -626,  -626,   200,  -626,    90,  -172,  -626,
    -626,  -317,  -626,    -2,  -626,  -626,    84,  -626,   414,    97,
    -223,   -65,  -104,  -626,  -237
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,    69,    70,    71,    72,    73,   474,   475,   476,   477,
      74,   111,   112,   153,   128,   129,   486,    77,    78,    79,
     154,   284,   155,    80,   314,    81,    82,    83,   645,    84,
      85,    86,   487,   488,   489,   565,   566,   567,   156,   157,
     442,   186,   158,   352,   159,   209,   160,   524,   522,   161,
     162,   507,   187,   188,   163,   254,   255,   164,   165,   256,
     286,   306,   167,   257,   258
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      75,   113,    76,   196,   123,   607,   201,   642,   358,   124,
     168,   467,   127,   564,   185,   221,   471,   687,   171,   190,
     224,   226,   388,   373,   260,   262,   285,   643,   288,   296,
     200,   191,   198,   312,   127,   298,   316,   285,   446,   175,
     318,   321,   125,   313,   323,   325,   410,   220,   330,   333,
     434,   223,   491,   285,   571,   573,   575,   698,   206,   207,
     447,   359,   287,   127,   199,   285,   708,    75,   202,   360,
      75,   228,    76,   287,   259,   361,   307,   307,   264,   307,
     307,  -223,   721,   362,   505,  -258,   363,   429,   259,   287,
     506,   285,   691,   381,   364,   382,   472,   365,   293,   644,
     383,   287,   367,   178,   179,   366,   285,   338,   473,   126,
     368,   166,   339,   170,   376,   222,   308,   174,   310,   311,
     225,   227,   177,   315,   261,   263,   353,   287,   289,   297,
     193,   166,   272,   707,   354,   299,   317,   530,   246,   292,
     319,   322,   287,   564,   324,   326,   241,   564,   331,   334,
     435,   253,   492,   176,   572,   574,   576,   699,   665,   207,
     197,   192,   194,   355,   539,   444,   445,   332,   718,   207,
     127,   369,   205,   370,   528,   272,   380,   303,   273,   281,
     282,   309,   114,   115,   729,   207,   116,   117,   385,   541,
     377,   561,   626,   627,   628,   629,   378,   379,   127,   632,
     384,   389,   390,   204,   702,   392,   335,   455,   430,   436,
     456,   543,   437,   272,   438,   216,   217,   218,   219,   278,
     279,   280,   281,   282,   495,   470,   327,   356,   564,   496,
     118,   455,   119,   478,   120,   717,   121,   231,   232,   480,
     216,   217,   218,   219,   481,   242,   387,   285,   247,   482,
     122,   461,   462,   463,   464,   465,   508,   617,   279,   280,
     281,   282,   328,   391,   455,   483,   290,   484,   490,   564,
     266,   267,   268,   269,   270,   271,   304,   272,   464,   465,
     273,   274,   275,   287,   493,   568,   540,   441,   441,   441,
     635,    75,   562,   433,   569,   443,   443,   443,   579,   440,
     440,   440,   348,   597,   349,   337,   350,   612,   351,   462,
     463,   464,   465,   658,   469,   211,   213,   215,   614,   276,
     277,   278,   279,   280,   281,   282,   329,   636,    75,    75,
     640,   637,   650,   651,   652,   211,   229,   230,   213,   215,
     639,  -263,   664,   667,   668,   670,   307,   307,   307,   307,
     307,   307,   307,   307,   307,   307,   307,   307,   307,   307,
     307,   307,   641,   393,   394,   395,   396,   397,   398,   399,
     400,   401,   402,   403,   404,   405,   406,   407,   408,   409,
     680,   253,   681,   684,   686,   581,   582,   583,   584,   585,
     586,   587,   588,   589,   590,   591,   592,   593,   594,   595,
     596,   685,   644,   690,   692,   694,   696,   294,   697,  -268,
    -266,   468,   700,  -267,   701,   704,   710,   479,   671,   714,
     703,   705,   715,   720,   722,   723,   734,   726,   730,   731,
     736,   737,    75,   738,   739,    75,   683,   203,   498,   740,
     500,   189,   502,   609,   504,   741,   742,   606,   613,    75,
     195,   266,   267,   268,   269,   270,   271,   646,   272,   521,
     682,   273,   274,   275,     0,   602,   536,     0,     0,     0,
     113,   608,     0,   531,   532,   533,   534,     0,   535,   537,
       0,     0,   538,    75,     0,   570,   706,     0,    75,   542,
       0,     0,    75,   307,   307,   307,   307,   307,     0,   580,
     276,   277,   278,   279,   280,   281,   282,   295,   544,   545,
     546,   547,   548,   549,   550,   551,   552,   553,   554,   555,
     556,   557,   558,   559,   560,     0,     0,     0,   210,   212,
     214,   212,   659,   611,   308,   310,   311,     0,     0,     0,
       0,     0,   615,   577,   578,     0,     0,     0,   210,   212,
     212,   212,   214,   212,     0,     0,   233,     0,     0,     0,
       0,     0,    75,   243,   244,     0,    75,     0,    75,    75,
     648,     0,     0,     0,     0,     0,   666,   448,     0,    75,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   305,   305,     0,   305,   305,     0,
       0,     0,     0,     0,     0,   625,     0,   127,     0,     0,
       0,     0,   604,     0,    75,     0,     0,     0,     0,   649,
       0,   266,   267,   268,   269,   270,   271,     0,   272,   657,
       0,   273,   274,   275,     0,     0,     0,   538,     0,     0,
       0,     0,     0,    75,     0,   689,     0,    75,     0,     0,
       0,     0,     0,     0,    75,     0,   266,   267,   268,   269,
     270,   271,     0,   272,   669,     0,   273,   274,   275,   113,
     276,   277,   278,   279,   280,   281,   282,   449,     0,     0,
       0,     0,     0,     0,     0,     0,   450,   451,    75,   452,
     453,   454,    75,   455,     0,     0,   456,   457,   458,    75,
     113,     0,     0,     0,   693,   276,   277,   278,   279,   280,
     281,   282,   605,    75,     0,    75,     0,     0,     0,     0,
       0,     0,     0,   653,     0,    75,    75,     0,     0,     0,
       0,    75,     0,     0,     0,   459,   460,   461,   462,   463,
     464,   465,   709,     0,     0,     0,     0,     0,   712,   716,
       0,     0,     0,   695,   497,     0,   499,     0,   501,     0,
     503,     0,     0,   727,     0,   728,     0,   266,   267,   268,
     269,   270,   271,     0,   272,   732,   733,   273,   274,   275,
       0,   735,     0,   523,   523,   525,   526,     0,     0,     0,
       0,   711,   266,   267,   268,   269,   270,   271,     0,   272,
       0,     0,   273,   274,   275,   724,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   276,   277,   278,   279,
     280,   281,   282,   654,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   276,   277,   278,   279,   280,   281,   282,   713,   266,
     267,   268,   269,   270,   271,     0,   272,     0,     0,   273,
     274,   275,     0,     0,   305,   305,   305,   305,   305,   305,
     305,   305,   305,   305,   305,   305,   305,   305,   305,   305,
       0,   603,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   276,   277,
     278,   279,   280,   281,   282,   725,     0,     0,   655,   618,
     619,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   624,     0,     0,     0,     0,     0,   630,   656,   631,
       0,     0,     0,   633,   634,     0,   266,   267,   268,   269,
     270,   271,     0,   272,     0,     0,   273,   274,   275,     0,
       0,     0,     0,   340,   266,   267,   268,   269,     0,   341,
       0,   272,     0,     0,   273,   274,   275,   266,   267,   268,
     269,   270,   271,     0,   272,     0,     0,   273,   274,   275,
       0,     0,     0,     0,     0,   276,   277,   278,   279,   280,
     281,   282,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   276,   277,   278,   279,   280,   281,   282,
       0,   305,   660,   661,   662,   663,   276,   277,   278,   279,
     280,   281,   282,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   342,   672,   673,   674,   675,     0,   343,
     676,   677,   678,   679,     0,     0,   620,   266,   267,   268,
     269,   270,   271,     0,   272,     0,     0,   273,   274,   275,
     266,   267,   268,   269,   270,   271,     0,   272,     0,     0,
     273,   274,   275,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   630,   276,   277,   278,   279,
     280,   281,   282,     0,     0,     0,     0,     0,     0,   276,
     277,   278,   279,   280,   281,   282,   172,     0,   719,   130,
     131,     1,   132,   133,    87,   134,   135,   136,   137,   138,
     139,    88,    10,    89,    90,    12,    91,    92,    93,    94,
      17,    18,    95,    96,    21,    97,    23,    24,    98,    99,
     100,   101,    28,     0,    30,    31,   102,   103,   104,   105,
       0,     0,     0,     0,     0,     0,   142,     0,    36,    37,
       0,     0,     0,   143,    39,   144,    41,   145,    43,   146,
      45,    46,    47,    48,    49,    50,    51,    52,    53,     0,
      54,    55,    56,   147,    58,     0,     0,    59,   148,    61,
      62,   106,   107,   108,   109,     0,     0,     0,   110,     0,
       0,     0,   149,   150,     0,     0,   173,     0,   169,     0,
       0,   152,   248,   249,     1,   250,   251,    87,   134,   135,
     136,   137,   138,   139,    88,    10,    89,    90,    12,    91,
      92,    93,    94,    17,    18,    95,    96,    21,    97,    23,
      24,    98,    99,   100,   101,    28,     0,    30,    31,   102,
     103,   104,   105,     0,     0,     0,     0,     0,     0,   142,
       0,    36,    37,     0,     0,     0,   143,    39,   144,    41,
     145,    43,   146,    45,    46,    47,    48,    49,    50,    51,
      52,    53,     0,    54,    55,    56,   147,    58,     0,     0,
      59,   148,    61,    62,   106,   107,   108,   109,     0,     0,
       0,   110,     0,     0,     0,   149,   150,     0,     0,     0,
       0,   169,     0,     0,   152,     0,   252,   130,   131,     1,
     132,   133,    87,   134,   135,   136,   137,   138,   139,    88,
      10,    89,    90,    12,    91,    92,    93,    94,    17,    18,
      95,    96,    21,    97,   265,    24,    98,    99,   100,   101,
      28,     0,    30,    31,   102,   103,   104,   105,   266,   267,
     268,   269,   270,   271,   142,   272,    36,    37,   273,   274,
     275,   143,    39,   144,    41,   145,    43,   146,    45,    46,
      47,    48,    49,    50,    51,    52,    53,     0,    54,    55,
      56,   147,    58,     0,     0,    59,   148,    61,    62,   106,
     107,   108,   109,     0,     0,     0,   110,   276,   277,   278,
     279,   280,   281,   282,     0,   283,   169,     0,     0,   152,
     130,   131,     1,   132,   133,    87,   134,   135,   136,   137,
     138,   139,    88,    10,    89,    90,    12,    91,    92,    93,
      94,    17,   300,    95,    96,    21,    97,    23,    24,    98,
      99,   100,   101,    28,     0,    30,    31,   102,   103,   104,
     105,   266,   267,   268,   269,   270,   271,   142,   272,   301,
     302,   273,   274,   275,   143,    39,   144,    41,   145,    43,
     146,    45,    46,    47,    48,    49,    50,    51,    52,    53,
       0,    54,    55,    56,   147,    58,     0,     0,    59,   148,
      61,    62,   106,   107,   108,   109,     0,     0,     0,   110,
     276,   277,   278,   279,   280,   281,   282,     0,   283,   169,
       0,     0,   152,   130,   131,     1,   132,   133,    87,   134,
     135,   136,   137,   138,   139,    88,    10,    89,    90,    12,
      91,    92,    93,    94,    17,    18,    95,    96,    21,    97,
      23,    24,    98,    99,   100,   101,    28,     0,   320,    31,
     102,   103,   104,   105,   266,   267,   268,   269,   270,   271,
     142,   272,    36,    37,   273,   274,   275,   143,    39,   144,
      41,   145,    43,   146,    45,    46,    47,    48,    49,    50,
      51,    52,    53,     0,    54,    55,    56,   147,    58,     0,
       0,    59,   148,    61,    62,   106,   107,   108,   109,     0,
       0,     0,   110,   276,   277,   278,   279,   280,   281,   282,
       0,   283,   169,     0,     0,   152,   130,   131,     1,   132,
     133,    87,   134,   135,   136,   137,   138,   139,    88,    10,
      89,    90,    12,    91,    92,    93,    94,    17,    18,    95,
      96,    21,    97,    23,    24,    98,    99,   100,   101,    28,
       0,    30,    31,   102,   103,   104,   105,   266,   267,   268,
     269,   270,   271,   142,   272,    36,    37,   273,   274,   275,
     143,    39,   144,    41,   145,    43,   146,    45,    46,    47,
      48,    49,    50,    51,    52,    53,     0,    54,    55,    56,
     147,    58,     0,     0,    59,   148,    61,    62,   106,   107,
     108,   109,     0,     0,     0,   110,   276,   277,   278,   279,
     280,   281,   282,     0,   283,   169,     0,     0,   152,   130,
     131,     1,   132,   133,    87,   134,   135,   136,   137,   138,
     139,    88,    10,    89,    90,    12,    91,    92,    93,    94,
      17,    18,    95,    96,    21,    97,   357,    24,    98,    99,
     100,   101,    28,     0,    30,    31,   102,   103,   104,   105,
     266,   267,   268,   269,   270,   271,   142,   272,    36,    37,
     273,   274,   275,   143,    39,   144,    41,   145,    43,   146,
      45,    46,    47,    48,    49,    50,    51,    52,    53,     0,
      54,    55,    56,   147,    58,     0,     0,    59,   148,    61,
      62,   106,   107,   108,   109,     0,     0,     0,   110,   276,
     277,   278,   279,   280,   281,   282,     0,     0,   169,     0,
       0,   152,   130,   131,     1,   132,   133,    87,   134,   135,
     136,   137,   138,   139,    88,    10,    89,    90,    12,    91,
      92,    93,    94,    17,    18,    95,    96,    21,    97,    23,
      24,    98,    99,   100,   101,    28,     0,    30,    31,   102,
     103,   104,   105,     0,     0,     0,     0,     0,     0,   142,
       0,    36,    37,     0,     0,     0,   143,   509,   144,   510,
     145,   511,   146,   512,    46,    47,    48,    49,    50,    51,
      52,   513,   514,   515,   516,    56,   147,    58,   517,   518,
     519,   520,    61,    62,   106,   107,   108,   109,     0,     0,
       0,   110,     0,     0,     0,   149,   150,     0,     0,     0,
       0,   169,     0,     0,   152,   130,   131,     1,   132,   133,
      87,   134,   135,   136,   137,   138,   139,    88,    10,    89,
      90,    12,    91,    92,    93,    94,    17,    18,    95,    96,
      21,    97,    23,    24,    98,    99,   100,   101,    28,     0,
      30,    31,   102,   103,   104,   105,     0,     0,     0,     0,
       0,     0,   142,     0,    36,    37,     0,     0,     0,   143,
      39,   144,    41,   145,    43,   146,    45,    46,    47,    48,
      49,    50,    51,    52,    53,     0,    54,    55,    56,   147,
      58,     0,     0,    59,   148,    61,    62,   106,   107,   108,
     109,     0,     0,     0,   110,     0,     0,     0,   149,   150,
       0,     0,     0,     0,   169,   245,     0,   152,   130,   131,
       1,   132,   133,    87,   134,   135,   136,   137,   138,   139,
      88,    10,    89,    90,    12,    91,    92,    93,    94,    17,
      18,    95,    96,    21,    97,    23,    24,    98,    99,   100,
     101,    28,     0,    30,    31,   102,   103,   104,   105,     0,
       0,     0,     0,     0,     0,   142,     0,    36,    37,     0,
       0,     0,   143,    39,   144,    41,   145,    43,   146,    45,
      46,    47,    48,    49,    50,    51,    52,    53,     0,    54,
      55,    56,   147,    58,     0,     0,    59,   148,    61,    62,
     106,   107,   108,   109,     0,     0,     0,   110,     0,     0,
       0,   149,   150,     0,     0,     0,     0,   169,   336,     0,
     152,   130,   131,     1,   132,   133,    87,   134,   135,   136,
     137,   138,   139,    88,    10,    89,    90,    12,    91,    92,
      93,    94,    17,    18,    95,    96,    21,    97,    23,    24,
      98,    99,   100,   101,    28,     0,    30,    31,   102,   103,
     104,   105,     0,     0,     0,     0,     0,     0,   142,     0,
      36,    37,     0,     0,     0,   143,    39,   144,    41,   145,
      43,   146,    45,    46,    47,    48,    49,    50,    51,    52,
      53,     0,    54,    55,    56,   147,    58,     0,     0,    59,
     148,    61,    62,   106,   107,   108,   109,     0,     0,     0,
     110,     0,     0,     0,   149,   150,     0,     0,     0,     0,
     169,   386,     0,   152,   130,   131,     1,   132,   133,    87,
     134,   135,   136,   137,   138,   139,    88,    10,    89,    90,
      12,    91,    92,    93,    94,    17,    18,    95,    96,    21,
      97,    23,    24,    98,    99,   100,   101,    28,     0,    30,
      31,   102,   103,   104,   105,     0,     0,     0,     0,     0,
       0,   142,     0,    36,    37,     0,     0,     0,   143,    39,
     144,    41,   145,    43,   146,    45,    46,    47,    48,    49,
      50,    51,    52,    53,     0,    54,    55,    56,   147,    58,
       0,     0,    59,   148,    61,    62,   106,   107,   108,   109,
       0,     0,     0,   110,     0,     0,     0,   149,   150,     0,
       0,     0,     0,   169,   529,     0,   152,   130,   131,     1,
     132,   133,    87,   134,   135,   136,   137,   138,   139,    88,
      10,    89,    90,    12,    91,    92,    93,    94,    17,    18,
      95,    96,    21,    97,    23,    24,    98,    99,   100,   101,
      28,     0,    30,    31,   102,   103,   104,   105,     0,     0,
       0,     0,     0,     0,   142,     0,    36,    37,     0,     0,
       0,   143,    39,   144,    41,   145,    43,   146,    45,    46,
      47,    48,    49,    50,    51,    52,    53,     0,    54,    55,
      56,   147,    58,     0,     0,    59,   148,    61,    62,   106,
     107,   108,   109,     0,     0,     0,   110,     0,     0,     0,
     149,   150,     0,     0,     0,     0,   169,   616,     0,   152,
     130,   131,     1,   132,   133,    87,   134,   135,   136,   137,
     138,   139,    88,    10,    89,    90,   140,    91,    92,    93,
      94,    17,    18,    95,    96,   141,    97,    23,    24,    98,
      99,   100,   101,    28,     0,    30,    31,   102,   103,   104,
     105,     0,     0,     0,     0,     0,     0,   142,     0,    36,
      37,     0,     0,     0,   143,    39,   144,    41,   145,    43,
     146,    45,    46,    47,    48,    49,    50,    51,    52,    53,
       0,    54,    55,    56,   147,    58,     0,     0,    59,   148,
      61,    62,   106,   107,   108,   109,     0,     0,     0,   110,
       0,     0,     0,   149,   150,     0,     0,     0,     0,   151,
       0,     0,   152,   130,   131,     1,   132,   133,    87,   134,
     135,   136,   137,   138,   139,    88,    10,    89,    90,    12,
      91,    92,    93,    94,    17,    18,    95,    96,    21,    97,
      23,    24,    98,    99,   100,   101,    28,     0,    30,    31,
     102,   103,   104,   105,     0,     0,     0,     0,     0,     0,
     142,     0,    36,    37,     0,     0,     0,   143,    39,   144,
      41,   145,    43,   146,    45,    46,    47,    48,    49,    50,
      51,    52,    53,     0,    54,    55,    56,   147,    58,     0,
       0,    59,   148,    61,    62,   106,   107,   108,   109,     0,
       0,     0,   110,     0,     0,     0,   149,   150,     0,     0,
       0,     0,   169,     0,     0,   152,   130,   131,     1,   132,
     133,    87,   134,   135,   136,   137,   138,   139,    88,    10,
      89,    90,    12,    91,    92,    93,    94,    17,    18,    95,
      96,    21,    97,    23,    24,    98,    99,   100,   101,    28,
       0,    30,    31,   102,   103,   104,   105,     0,     0,     0,
       0,     0,     0,   142,     0,    36,    37,     0,     0,     0,
     143,    39,   144,    41,   145,    43,   146,    45,    46,    47,
      48,    49,    50,    51,    52,    53,     0,    54,    55,    56,
     147,    58,     0,     0,    59,   148,    61,    62,   106,   107,
     108,   109,     0,     0,     0,   110,     0,     0,     0,   149,
     150,     0,     0,     0,     0,   151,     0,     0,   152,   130,
     131,     1,   132,   133,    87,   134,   135,   136,   137,   138,
     139,    88,    10,    89,    90,    12,    91,    92,    93,    94,
      17,    18,    95,    96,    21,    97,    23,    24,    98,    99,
     100,   101,    28,     0,    30,    31,   102,   103,   104,   105,
       0,     0,     0,     0,     0,     0,   142,     0,    36,    37,
       0,     0,     0,   143,    39,   144,    41,   145,    43,   146,
      45,    46,    47,    48,    49,    50,    51,    52,   180,     0,
     181,    55,    56,   182,    58,     0,     0,   183,   184,    61,
      62,   106,   107,   108,   109,     0,     0,     0,   110,     0,
       0,     0,   149,   150,     0,     0,     0,     0,   169,     0,
       0,   152,   130,   131,     1,   132,   133,    87,   134,   135,
     136,   137,   138,   139,    88,    10,    89,    90,    12,    91,
      92,    93,    94,    17,    18,    95,    96,    21,    97,    23,
      24,    98,    99,   100,   101,    28,     0,    30,    31,   102,
     103,   104,   105,     0,     0,     0,     0,     0,     0,   142,
       0,    36,    37,     0,     0,     0,   143,    39,   144,    41,
     145,    43,   146,    45,    46,    47,    48,    49,    50,    51,
      52,    53,     0,    54,    55,    56,   147,    58,     0,     0,
      59,    60,    61,    62,   106,   107,   108,   109,     0,     0,
       0,   110,     0,     0,     0,   149,   150,     0,     0,     0,
       0,   208,     0,     0,   152,   130,   131,     1,   132,   133,
      87,   134,   135,   136,   137,   138,   139,    88,    10,    89,
      90,    12,    91,    92,    93,    94,    17,    18,    95,    96,
      21,    97,    23,    24,    98,    99,   100,   101,    28,     0,
      30,    31,   102,   103,   104,   105,     0,     0,     0,     0,
       0,     0,   142,     0,    36,    37,     0,     0,     0,   143,
      39,   144,    41,   145,    43,   146,    45,    46,    47,    48,
      49,    50,    51,    52,    53,     0,    54,    55,    56,   147,
      58,     0,     0,    59,    60,    61,    62,   106,   107,   108,
     109,     0,     0,     0,   110,     0,     0,     0,   149,   150,
       0,     0,     0,     0,   169,     0,     0,   152,   130,   131,
       1,   132,   133,    87,   134,   135,   136,   137,   138,   139,
      88,    10,    89,    90,    12,    91,    92,    93,    94,    17,
      18,    95,    96,    21,    97,    23,    24,    98,    99,   100,
     101,    28,     0,    30,    31,   102,   103,   104,   105,     0,
       0,     0,     0,     0,     0,   142,     0,    36,    37,     0,
       0,     0,   143,    39,   144,    41,   145,    43,   146,    45,
      46,    47,    48,    49,    50,    51,    52,   513,     0,   515,
      55,    56,   147,    58,     0,     0,   519,   520,    61,    62,
     106,   107,   108,   109,     0,     0,     0,   110,     0,     0,
       0,   149,   150,     0,     0,     0,     0,   169,     0,     0,
     152,   248,   249,     1,   250,   251,    87,   134,   135,   136,
     137,   138,   139,    88,    10,    89,    90,    12,    91,    92,
      93,    94,    17,    18,    95,    96,    21,    97,    23,    24,
      98,    99,   100,   101,    28,     0,    30,    31,   102,   103,
     104,   105,     0,     0,     0,     0,     0,     0,   142,     0,
      36,    37,     0,     0,     0,   143,    39,   144,    41,   145,
      43,   146,    45,    46,    47,    48,    49,    50,    51,    52,
      53,     0,    54,    55,    56,   147,    58,     0,     0,    59,
     148,    61,    62,   106,   107,   108,   109,     0,     0,     0,
     110,     0,     0,     0,   149,   150,     0,     0,     0,     0,
     169,     0,     0,   152,   130,   131,     1,   132,   133,    87,
     134,   135,   136,   137,   138,   139,    88,    10,    89,    90,
      12,    91,    92,    93,    94,    17,    18,    95,    96,    21,
      97,    23,    24,    98,    99,   100,   101,    28,     0,    30,
      31,   102,   103,   104,   105,     0,     0,     0,     0,     0,
       0,   142,     0,    36,    37,     0,     0,     0,   143,    39,
     144,    41,   145,    43,   146,    45,    46,    47,    48,    49,
      50,    51,    52,   598,     0,   599,    55,    56,   147,    58,
       0,     0,   600,   601,    61,    62,   106,   107,   108,   109,
       0,     0,     0,   110,     0,     0,     0,   149,   150,     0,
       0,     0,     0,   169,     0,     0,   152,   130,   131,     1,
     132,   133,    87,   134,   135,   136,   137,   138,   139,    88,
      10,    89,    90,    12,    91,    92,    93,    94,    17,    18,
      95,    96,    21,    97,    23,    24,    98,    99,   100,   101,
      28,     0,    30,    31,   102,   103,   104,   105,     0,     0,
       0,     0,     0,     0,   142,     0,    36,    37,     0,     0,
       0,   143,    39,   144,    41,   145,    43,   146,    45,    46,
      47,    48,    49,    50,    51,    52,    53,     0,    54,    55,
      56,   147,    58,     0,     0,    59,   148,    61,    62,   106,
     107,   108,   109,     0,     0,     0,   110,     0,     0,     0,
       0,     0,   431,     0,     0,     0,   169,     1,     0,   152,
       0,     3,     4,     5,     6,     7,     8,     9,    10,     0,
      11,    12,    13,    14,    15,    16,    17,    18,     0,    20,
      21,    22,    23,    24,    25,    26,    27,     0,    28,    29,
      30,    31,     0,    33,    34,    35,     0,     0,     0,     0,
       0,     0,     0,     0,    36,    37,     0,     0,     0,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,     0,    54,    55,    56,    57,
      58,     0,     0,    59,    60,    61,    62,    63,    64,     0,
       0,    66,     0,     0,    67,     0,     0,     0,     0,     0,
       1,     0,   432,     2,     3,     4,     5,     6,     7,     8,
       9,    10,     0,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
       0,    28,    29,    30,    31,    32,    33,    34,    35,     0,
       0,     0,     0,     0,     0,     0,     0,    36,    37,     0,
       0,     0,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,     0,    54,
      55,    56,    57,    58,     0,     0,    59,    60,    61,    62,
      63,    64,    65,     0,    66,     0,     0,    67,     0,     0,
       0,     0,     1,     0,     0,    68,     3,     4,     5,     6,
       7,     8,     9,    10,     0,    11,    12,    13,    14,    15,
      16,    17,    18,     0,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    28,    29,    30,    31,     0,    33,    34,
      35,     0,     0,     0,     0,     0,     0,     0,     0,    36,
      37,     0,     0,     0,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
       0,    54,    55,    56,    57,    58,     0,     0,    59,    60,
      61,    62,    63,    64,     0,     0,    66,     0,     0,    67,
       0,     0,     0,     0,     1,     0,     0,   485,     3,     4,
       5,     6,     7,     8,     9,    10,     0,    11,    12,    13,
      14,    15,    16,    17,    18,     0,    20,    21,    22,    23,
      24,    25,    26,    27,     0,    28,    29,    30,    31,     0,
      33,    34,    35,     0,     0,     0,     0,     0,     0,     0,
       0,    36,    37,     0,     0,     0,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,     0,    54,    55,    56,    57,    58,     0,     0,
      59,    60,    61,    62,    63,    64,     0,     0,    66,     0,
       0,    67,     0,     0,     0,     0,     1,     0,     0,   647,
       3,     4,     5,     6,     7,     8,     9,    10,     0,    11,
      12,    13,    14,    15,    16,    17,    18,     0,    20,    21,
      22,    23,    24,    25,    26,    27,     0,    28,    29,    30,
      31,     0,    33,    34,    35,     0,     0,     0,     0,     0,
       0,     0,     0,    36,    37,     0,     0,     0,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,     0,    54,    55,    56,    57,    58,
       0,     0,    59,    60,    61,    62,    63,    64,     0,     0,
      66,     0,     0,    67,     0,     0,     0,     0,     1,     0,
       0,   688,     3,     4,     5,     6,     7,     8,     9,    10,
       0,    11,    12,    13,    14,    15,    16,    17,    18,     0,
      20,    21,    22,    23,    24,    25,    26,    27,     0,    28,
      29,    30,    31,     0,    33,    34,    35,     0,     0,     0,
       0,     0,     0,     0,     0,    36,    37,     0,     0,     0,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,     0,    54,    55,    56,
      57,    58,     0,     0,    59,    60,    61,    62,    63,    64,
       0,     0,     0,     0,     0,    67,     0,     0,     0,     0,
       0,     1,     0,   563,    87,     3,     4,     5,     6,     7,
       8,    88,    10,    89,    90,    12,    91,    92,    93,    94,
      17,    18,    95,    96,    21,    97,    23,    24,    98,    99,
     100,   101,    28,     0,    30,    31,   102,   103,   104,   105,
       0,     0,     0,     0,     0,     0,     0,     0,    36,    37,
       0,     0,     0,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,     0,
      54,    55,    56,    57,    58,     0,     0,    59,    60,    61,
      62,   106,   107,   108,   109,     0,     1,     0,   110,    87,
       3,     4,     5,     6,     7,     8,    88,    10,    89,    90,
      12,    91,    92,    93,    94,    17,    18,    95,    96,    21,
      97,    23,    24,    98,    99,   100,   101,    28,     0,    30,
      31,   102,   103,   104,   105,     0,     0,     0,     0,     0,
       0,     0,     0,    36,    37,     0,     0,     0,    38,    39,
      40,    41,    42,    43,    44,    45,   234,   235,   236,   237,
     238,    51,   239,    53,     0,    54,    55,   240,    57,    58,
       0,     0,    59,    60,    61,    62,   106,   107,   108,   109,
       0,     1,     0,   110,    87,   134,   135,     5,     6,   138,
     139,    88,    10,    89,    90,    12,    91,    92,    93,    94,
      17,    18,    95,    96,    21,    97,    23,    24,    98,    99,
     100,   101,    28,     0,    30,    31,   102,   103,   104,   105,
       0,     0,     0,     0,     0,     0,     0,     0,    36,    37,
       0,     0,     0,   143,    39,   144,    41,   145,    43,   146,
      45,    46,    47,    48,    49,    50,    51,    52,    53,     0,
      54,    55,    56,   439,    58,     0,     0,    59,    60,    61,
      62,   106,   107,   108,   109,     1,     0,     0,   110,     3,
       4,     5,     6,     7,     8,     9,    10,     0,    11,    12,
      13,    14,    15,     0,    17,    18,     0,    20,    21,    22,
      23,    24,    25,    99,    27,     0,    28,     0,    30,    31,
       0,    33,    34,    35,     0,     0,     0,     0,     0,     0,
       0,     0,    36,    37,     0,     0,     0,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,     0,    54,    55,    56,    57,    58,   344,
       0,    59,    60,    61,    62,   345,    64,     0,     0,     0,
       0,     0,    67,   266,   267,   268,   269,   270,   271,     0,
     272,   346,     0,   273,   274,   275,     0,   347,     0,     0,
       0,     0,     0,     0,   621,   266,   267,   268,   269,   270,
     271,     0,   272,     0,     0,   273,   274,   275,   266,   267,
     268,   269,   270,   271,     0,   272,   622,     0,   273,   274,
     275,     0,   276,   277,   278,   279,   280,   281,   282,     0,
     266,   267,   268,   269,   270,   271,     0,   272,     0,     0,
     273,   274,   275,     0,   276,   277,   278,   279,   280,   281,
     282,   623,     0,     0,     0,     0,     0,   276,   277,   278,
     279,   280,   281,   282,     0,   266,   267,   268,   269,   270,
     271,     0,   272,   466,     0,   273,   274,   275,     0,   276,
     277,   278,   279,   280,   281,   282,     0,   450,   451,     0,
     452,   453,   454,     0,   455,     0,     0,   456,   457,   458,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   291,     0,   276,   277,   278,   279,   280,   281,
     282,   266,   267,   268,   269,   270,   271,     0,   272,     0,
       0,   273,   274,   275,     0,     0,   459,   460,   461,   462,
     463,   464,   465,   266,   267,   268,   269,   270,   271,     0,
     272,     0,     0,   273,   274,   275,   266,   267,   268,   269,
     270,   271,     0,   272,     0,     0,   273,   274,   275,     0,
     276,   277,   278,   279,   280,   281,   282,   266,   267,   268,
     269,   270,   271,     0,   272,     0,     0,   273,   274,   275,
       0,     0,   276,   277,   278,   279,   280,   281,   282,     0,
     374,     0,   375,     0,     0,   276,   277,   278,   279,   280,
     281,   282,     0,   494,     0,   375,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   276,   277,   278,   279,
     280,   281,   282,     0,     0,     0,   375,   411,   412,   413,
     414,   415,   416,     0,   417,     0,     0,   418,   419,   420,
     411,   412,   413,   414,   415,   416,     0,   417,     0,     0,
     418,   419,   420,   411,   412,   413,   414,   415,   416,     0,
     417,     0,     0,   418,   419,   420,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   421,   422,   423,   424,
     425,   426,   427,     0,   428,     0,     0,     0,     0,   421,
     422,   423,   424,   425,   426,   427,     0,   527,     0,     0,
       0,     0,   421,   422,   423,   424,   425,   426,   427,     0,
     638,   266,   267,   268,   269,   270,   271,     0,   272,     0,
       0,   273,   274,   275,     0,     0,     0,     0,     0,   266,
     267,   268,   269,   270,   271,     0,   272,     0,     0,   273,
     274,   275,   266,   267,   268,   269,   270,   271,     0,   272,
       0,     0,   273,   274,   275,     0,     0,     0,     0,     0,
     276,   277,   278,   279,   280,   281,   282,   610,   371,   372,
       0,     0,     0,     0,     0,     0,     0,     0,   276,   277,
     278,   279,   280,   281,   282,     0,     0,     0,     0,     0,
       0,   276,   277,   278,   279,   280,   281,   282,   411,   412,
     413,   414,   415,   416,     0,   417,     0,     0,   418,   419,
     420,   450,   451,     0,   452,     0,     0,     0,   455,     0,
       0,   456,   457,   458,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   421,   422,   423,
     424,   425,   426,   427,     0,     0,     0,     0,     0,     0,
     459,   460,   461,   462,   463,   464,   465
};

static const yytype_int16 yycheck[] =
{
       0,     2,     0,    35,     9,   471,    67,   562,   231,     1,
      15,    31,    13,   432,    27,     1,     9,   642,    19,   101,
       1,     1,   259,   246,     1,     1,   166,    18,     1,     1,
     101,    32,    64,    37,    35,     1,     1,   177,    25,    34,
       1,     1,    34,    47,     1,     1,   283,    72,     1,     1,
       1,   101,     1,   193,     1,     1,     1,     1,   101,   102,
      47,    71,   166,    64,    65,   205,   691,    67,     0,    79,
      70,   102,    70,   177,   103,    71,   180,   181,   105,   183,
     184,   105,   707,    79,    25,   105,    71,   286,   103,   193,
      31,   231,   647,   108,    79,   102,    89,    71,   101,    90,
     107,   205,    71,    39,    40,    79,   246,    31,   101,   101,
      79,    14,    31,    16,   108,   101,   181,    20,   183,   184,
     101,   101,    25,   105,   101,   101,   101,   231,   101,   101,
      33,    34,    52,   688,   101,   101,   101,   374,   151,   171,
     101,   101,   246,   562,   101,   101,   147,   566,   101,   101,
     101,   152,   101,    25,   101,   101,   101,   101,   101,   102,
      63,    33,    34,   101,   387,   301,   302,   199,   101,   102,
     171,    31,    75,    31,   373,    52,   107,   178,    55,    99,
     100,   182,    10,    11,   101,   102,    14,    15,   102,   388,
     108,   428,   509,   510,   511,   512,   108,   108,   199,   516,
     107,   101,   101,    75,   670,    29,   207,    52,   101,   101,
      55,   410,   101,    52,   101,   118,   119,   120,   121,    96,
      97,    98,    99,   100,    15,   101,     1,   228,   647,    15,
      58,    52,    60,   101,    62,   701,    64,   140,   141,   101,
     143,   144,   145,   146,   101,   148,   259,   387,   151,   101,
      78,    96,    97,    98,    99,   100,    29,   494,    97,    98,
      99,   100,    37,   264,    52,   101,   169,   328,   101,   688,
      45,    46,    47,    48,    49,    50,   179,    52,    99,   100,
      55,    56,    57,   387,   101,    18,   102,   300,   301,   302,
     527,   291,   101,   291,   101,   300,   301,   302,   101,   300,
     301,   302,    58,    73,    60,   208,    62,    92,    64,    97,
      98,    99,   100,    91,   315,   115,   116,   117,   101,    94,
      95,    96,    97,    98,    99,   100,   101,   104,   328,   329,
     104,   530,   101,   101,   101,   135,   136,   137,   138,   139,
     539,   105,   101,    92,   101,     9,   450,   451,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   561,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
      31,   382,    73,   104,   104,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   638,    90,   101,   101,    37,    91,     1,   101,   105,
     105,   314,   101,   105,     9,    73,    91,   320,   617,   101,
     104,   104,   101,   104,    91,   101,    91,   101,   101,   101,
      91,    91,   432,   101,    91,   435,   635,    70,   341,   101,
     343,    27,   345,   476,   347,   101,   101,   470,   488,   449,
      34,    45,    46,    47,    48,    49,    50,   566,    52,   369,
     632,    55,    56,    57,    -1,   467,   382,    -1,    -1,    -1,
     471,   472,    -1,   376,   377,   378,   379,    -1,   381,   382,
      -1,    -1,   385,   483,    -1,   435,   685,    -1,   488,   392,
      -1,    -1,   492,   597,   598,   599,   600,   601,    -1,   449,
      94,    95,    96,    97,    98,    99,   100,   101,   411,   412,
     413,   414,   415,   416,   417,   418,   419,   420,   421,   422,
     423,   424,   425,   426,   427,    -1,    -1,    -1,   114,   115,
     116,   117,   597,   483,   599,   600,   601,    -1,    -1,    -1,
      -1,    -1,   492,   446,   447,    -1,    -1,    -1,   134,   135,
     136,   137,   138,   139,    -1,    -1,   142,    -1,    -1,    -1,
      -1,    -1,   562,   149,   150,    -1,   566,    -1,   568,   569,
     568,    -1,    -1,    -1,    -1,    -1,   608,     1,    -1,   579,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   180,   181,    -1,   183,   184,    -1,
      -1,    -1,    -1,    -1,    -1,   508,    -1,   608,    -1,    -1,
      -1,    -1,     1,    -1,   614,    -1,    -1,    -1,    -1,   569,
      -1,    45,    46,    47,    48,    49,    50,    -1,    52,   579,
      -1,    55,    56,    57,    -1,    -1,    -1,   540,    -1,    -1,
      -1,    -1,    -1,   643,    -1,   643,    -1,   647,    -1,    -1,
      -1,    -1,    -1,    -1,   654,    -1,    45,    46,    47,    48,
      49,    50,    -1,    52,   614,    -1,    55,    56,    57,   670,
      94,    95,    96,    97,    98,    99,   100,   101,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    45,    46,   688,    48,
      49,    50,   692,    52,    -1,    -1,    55,    56,    57,   699,
     701,    -1,    -1,    -1,   654,    94,    95,    96,    97,    98,
      99,   100,   101,   713,    -1,   715,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     1,    -1,   725,   726,    -1,    -1,    -1,
      -1,   731,    -1,    -1,    -1,    94,    95,    96,    97,    98,
      99,   100,   692,    -1,    -1,    -1,    -1,    -1,     1,   699,
      -1,    -1,    -1,   656,   340,    -1,   342,    -1,   344,    -1,
     346,    -1,    -1,   713,    -1,   715,    -1,    45,    46,    47,
      48,    49,    50,    -1,    52,   725,   726,    55,    56,    57,
      -1,   731,    -1,   369,   370,   371,   372,    -1,    -1,    -1,
      -1,   694,    45,    46,    47,    48,    49,    50,    -1,    52,
      -1,    -1,    55,    56,    57,     1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    94,    95,    96,    97,
      98,    99,   100,   101,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    94,    95,    96,    97,    98,    99,   100,   101,    45,
      46,    47,    48,    49,    50,    -1,    52,    -1,    -1,    55,
      56,    57,    -1,    -1,   450,   451,   452,   453,   454,   455,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
      -1,   467,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,    95,
      96,    97,    98,    99,   100,   101,    -1,    -1,    17,   495,
     496,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   507,    -1,    -1,    -1,    -1,    -1,   513,    37,   515,
      -1,    -1,    -1,   519,   520,    -1,    45,    46,    47,    48,
      49,    50,    -1,    52,    -1,    -1,    55,    56,    57,    -1,
      -1,    -1,    -1,    31,    45,    46,    47,    48,    -1,    37,
      -1,    52,    -1,    -1,    55,    56,    57,    45,    46,    47,
      48,    49,    50,    -1,    52,    -1,    -1,    55,    56,    57,
      -1,    -1,    -1,    -1,    -1,    94,    95,    96,    97,    98,
      99,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    94,    95,    96,    97,    98,    99,   100,
      -1,   597,   598,   599,   600,   601,    94,    95,    96,    97,
      98,    99,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    31,   620,   621,   622,   623,    -1,    37,
     626,   627,   628,   629,    -1,    -1,    31,    45,    46,    47,
      48,    49,    50,    -1,    52,    -1,    -1,    55,    56,    57,
      45,    46,    47,    48,    49,    50,    -1,    52,    -1,    -1,
      55,    56,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   681,    94,    95,    96,    97,
      98,    99,   100,    -1,    -1,    -1,    -1,    -1,    -1,    94,
      95,    96,    97,    98,    99,   100,     1,    -1,   704,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    -1,    39,    40,    41,    42,    43,    44,
      -1,    -1,    -1,    -1,    -1,    -1,    51,    -1,    53,    54,
      -1,    -1,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    -1,
      75,    76,    77,    78,    79,    -1,    -1,    82,    83,    84,
      85,    86,    87,    88,    89,    -1,    -1,    -1,    93,    -1,
      -1,    -1,    97,    98,    -1,    -1,   101,    -1,   103,    -1,
      -1,   106,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    -1,    39,    40,    41,
      42,    43,    44,    -1,    -1,    -1,    -1,    -1,    -1,    51,
      -1,    53,    54,    -1,    -1,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    -1,    75,    76,    77,    78,    79,    -1,    -1,
      82,    83,    84,    85,    86,    87,    88,    89,    -1,    -1,
      -1,    93,    -1,    -1,    -1,    97,    98,    -1,    -1,    -1,
      -1,   103,    -1,    -1,   106,    -1,   108,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    -1,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    -1,    75,    76,
      77,    78,    79,    -1,    -1,    82,    83,    84,    85,    86,
      87,    88,    89,    -1,    -1,    -1,    93,    94,    95,    96,
      97,    98,    99,   100,    -1,   102,   103,    -1,    -1,   106,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    -1,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      -1,    75,    76,    77,    78,    79,    -1,    -1,    82,    83,
      84,    85,    86,    87,    88,    89,    -1,    -1,    -1,    93,
      94,    95,    96,    97,    98,    99,   100,    -1,   102,   103,
      -1,    -1,   106,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    -1,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    -1,    75,    76,    77,    78,    79,    -1,
      -1,    82,    83,    84,    85,    86,    87,    88,    89,    -1,
      -1,    -1,    93,    94,    95,    96,    97,    98,    99,   100,
      -1,   102,   103,    -1,    -1,   106,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      -1,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    -1,    75,    76,    77,
      78,    79,    -1,    -1,    82,    83,    84,    85,    86,    87,
      88,    89,    -1,    -1,    -1,    93,    94,    95,    96,    97,
      98,    99,   100,    -1,   102,   103,    -1,    -1,   106,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    -1,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    -1,
      75,    76,    77,    78,    79,    -1,    -1,    82,    83,    84,
      85,    86,    87,    88,    89,    -1,    -1,    -1,    93,    94,
      95,    96,    97,    98,    99,   100,    -1,    -1,   103,    -1,
      -1,   106,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    -1,    39,    40,    41,
      42,    43,    44,    -1,    -1,    -1,    -1,    -1,    -1,    51,
      -1,    53,    54,    -1,    -1,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    -1,    -1,
      -1,    93,    -1,    -1,    -1,    97,    98,    -1,    -1,    -1,
      -1,   103,    -1,    -1,   106,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    -1,
      39,    40,    41,    42,    43,    44,    -1,    -1,    -1,    -1,
      -1,    -1,    51,    -1,    53,    54,    -1,    -1,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    -1,    75,    76,    77,    78,
      79,    -1,    -1,    82,    83,    84,    85,    86,    87,    88,
      89,    -1,    -1,    -1,    93,    -1,    -1,    -1,    97,    98,
      -1,    -1,    -1,    -1,   103,   104,    -1,   106,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    -1,    39,    40,    41,    42,    43,    44,    -1,
      -1,    -1,    -1,    -1,    -1,    51,    -1,    53,    54,    -1,
      -1,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    -1,    75,
      76,    77,    78,    79,    -1,    -1,    82,    83,    84,    85,
      86,    87,    88,    89,    -1,    -1,    -1,    93,    -1,    -1,
      -1,    97,    98,    -1,    -1,    -1,    -1,   103,   104,    -1,
     106,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    -1,    39,    40,    41,    42,
      43,    44,    -1,    -1,    -1,    -1,    -1,    -1,    51,    -1,
      53,    54,    -1,    -1,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    -1,    75,    76,    77,    78,    79,    -1,    -1,    82,
      83,    84,    85,    86,    87,    88,    89,    -1,    -1,    -1,
      93,    -1,    -1,    -1,    97,    98,    -1,    -1,    -1,    -1,
     103,   104,    -1,   106,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    -1,    39,
      40,    41,    42,    43,    44,    -1,    -1,    -1,    -1,    -1,
      -1,    51,    -1,    53,    54,    -1,    -1,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    -1,    75,    76,    77,    78,    79,
      -1,    -1,    82,    83,    84,    85,    86,    87,    88,    89,
      -1,    -1,    -1,    93,    -1,    -1,    -1,    97,    98,    -1,
      -1,    -1,    -1,   103,   104,    -1,   106,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    -1,    39,    40,    41,    42,    43,    44,    -1,    -1,
      -1,    -1,    -1,    -1,    51,    -1,    53,    54,    -1,    -1,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    -1,    75,    76,
      77,    78,    79,    -1,    -1,    82,    83,    84,    85,    86,
      87,    88,    89,    -1,    -1,    -1,    93,    -1,    -1,    -1,
      97,    98,    -1,    -1,    -1,    -1,   103,   104,    -1,   106,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    -1,    39,    40,    41,    42,    43,
      44,    -1,    -1,    -1,    -1,    -1,    -1,    51,    -1,    53,
      54,    -1,    -1,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      -1,    75,    76,    77,    78,    79,    -1,    -1,    82,    83,
      84,    85,    86,    87,    88,    89,    -1,    -1,    -1,    93,
      -1,    -1,    -1,    97,    98,    -1,    -1,    -1,    -1,   103,
      -1,    -1,   106,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    -1,    39,    40,
      41,    42,    43,    44,    -1,    -1,    -1,    -1,    -1,    -1,
      51,    -1,    53,    54,    -1,    -1,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    -1,    75,    76,    77,    78,    79,    -1,
      -1,    82,    83,    84,    85,    86,    87,    88,    89,    -1,
      -1,    -1,    93,    -1,    -1,    -1,    97,    98,    -1,    -1,
      -1,    -1,   103,    -1,    -1,   106,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      -1,    39,    40,    41,    42,    43,    44,    -1,    -1,    -1,
      -1,    -1,    -1,    51,    -1,    53,    54,    -1,    -1,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    -1,    75,    76,    77,
      78,    79,    -1,    -1,    82,    83,    84,    85,    86,    87,
      88,    89,    -1,    -1,    -1,    93,    -1,    -1,    -1,    97,
      98,    -1,    -1,    -1,    -1,   103,    -1,    -1,   106,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    -1,    39,    40,    41,    42,    43,    44,
      -1,    -1,    -1,    -1,    -1,    -1,    51,    -1,    53,    54,
      -1,    -1,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    -1,
      75,    76,    77,    78,    79,    -1,    -1,    82,    83,    84,
      85,    86,    87,    88,    89,    -1,    -1,    -1,    93,    -1,
      -1,    -1,    97,    98,    -1,    -1,    -1,    -1,   103,    -1,
      -1,   106,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    -1,    39,    40,    41,
      42,    43,    44,    -1,    -1,    -1,    -1,    -1,    -1,    51,
      -1,    53,    54,    -1,    -1,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    -1,    75,    76,    77,    78,    79,    -1,    -1,
      82,    83,    84,    85,    86,    87,    88,    89,    -1,    -1,
      -1,    93,    -1,    -1,    -1,    97,    98,    -1,    -1,    -1,
      -1,   103,    -1,    -1,   106,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    -1,
      39,    40,    41,    42,    43,    44,    -1,    -1,    -1,    -1,
      -1,    -1,    51,    -1,    53,    54,    -1,    -1,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    -1,    75,    76,    77,    78,
      79,    -1,    -1,    82,    83,    84,    85,    86,    87,    88,
      89,    -1,    -1,    -1,    93,    -1,    -1,    -1,    97,    98,
      -1,    -1,    -1,    -1,   103,    -1,    -1,   106,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    -1,    39,    40,    41,    42,    43,    44,    -1,
      -1,    -1,    -1,    -1,    -1,    51,    -1,    53,    54,    -1,
      -1,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    -1,    75,
      76,    77,    78,    79,    -1,    -1,    82,    83,    84,    85,
      86,    87,    88,    89,    -1,    -1,    -1,    93,    -1,    -1,
      -1,    97,    98,    -1,    -1,    -1,    -1,   103,    -1,    -1,
     106,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    -1,    39,    40,    41,    42,
      43,    44,    -1,    -1,    -1,    -1,    -1,    -1,    51,    -1,
      53,    54,    -1,    -1,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    -1,    75,    76,    77,    78,    79,    -1,    -1,    82,
      83,    84,    85,    86,    87,    88,    89,    -1,    -1,    -1,
      93,    -1,    -1,    -1,    97,    98,    -1,    -1,    -1,    -1,
     103,    -1,    -1,   106,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    -1,    39,
      40,    41,    42,    43,    44,    -1,    -1,    -1,    -1,    -1,
      -1,    51,    -1,    53,    54,    -1,    -1,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    -1,    75,    76,    77,    78,    79,
      -1,    -1,    82,    83,    84,    85,    86,    87,    88,    89,
      -1,    -1,    -1,    93,    -1,    -1,    -1,    97,    98,    -1,
      -1,    -1,    -1,   103,    -1,    -1,   106,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    -1,    39,    40,    41,    42,    43,    44,    -1,    -1,
      -1,    -1,    -1,    -1,    51,    -1,    53,    54,    -1,    -1,
      -1,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    -1,    75,    76,
      77,    78,    79,    -1,    -1,    82,    83,    84,    85,    86,
      87,    88,    89,    -1,    -1,    -1,    93,    -1,    -1,    -1,
      -1,    -1,     1,    -1,    -1,    -1,   103,     6,    -1,   106,
      -1,    10,    11,    12,    13,    14,    15,    16,    17,    -1,
      19,    20,    21,    22,    23,    24,    25,    26,    -1,    28,
      29,    30,    31,    32,    33,    34,    35,    -1,    37,    38,
      39,    40,    -1,    42,    43,    44,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    53,    54,    -1,    -1,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    -1,    75,    76,    77,    78,
      79,    -1,    -1,    82,    83,    84,    85,    86,    87,    -1,
      -1,    90,    -1,    -1,    93,    -1,    -1,    -1,    -1,    -1,
       6,    -1,   101,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    -1,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      -1,    37,    38,    39,    40,    41,    42,    43,    44,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    53,    54,    -1,
      -1,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    -1,    75,
      76,    77,    78,    79,    -1,    -1,    82,    83,    84,    85,
      86,    87,    88,    -1,    90,    -1,    -1,    93,    -1,    -1,
      -1,    -1,     6,    -1,    -1,   101,    10,    11,    12,    13,
      14,    15,    16,    17,    -1,    19,    20,    21,    22,    23,
      24,    25,    26,    -1,    28,    29,    30,    31,    32,    33,
      34,    35,    -1,    37,    38,    39,    40,    -1,    42,    43,
      44,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    53,
      54,    -1,    -1,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      -1,    75,    76,    77,    78,    79,    -1,    -1,    82,    83,
      84,    85,    86,    87,    -1,    -1,    90,    -1,    -1,    93,
      -1,    -1,    -1,    -1,     6,    -1,    -1,   101,    10,    11,
      12,    13,    14,    15,    16,    17,    -1,    19,    20,    21,
      22,    23,    24,    25,    26,    -1,    28,    29,    30,    31,
      32,    33,    34,    35,    -1,    37,    38,    39,    40,    -1,
      42,    43,    44,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    53,    54,    -1,    -1,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    -1,    75,    76,    77,    78,    79,    -1,    -1,
      82,    83,    84,    85,    86,    87,    -1,    -1,    90,    -1,
      -1,    93,    -1,    -1,    -1,    -1,     6,    -1,    -1,   101,
      10,    11,    12,    13,    14,    15,    16,    17,    -1,    19,
      20,    21,    22,    23,    24,    25,    26,    -1,    28,    29,
      30,    31,    32,    33,    34,    35,    -1,    37,    38,    39,
      40,    -1,    42,    43,    44,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    53,    54,    -1,    -1,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    -1,    75,    76,    77,    78,    79,
      -1,    -1,    82,    83,    84,    85,    86,    87,    -1,    -1,
      90,    -1,    -1,    93,    -1,    -1,    -1,    -1,     6,    -1,
      -1,   101,    10,    11,    12,    13,    14,    15,    16,    17,
      -1,    19,    20,    21,    22,    23,    24,    25,    26,    -1,
      28,    29,    30,    31,    32,    33,    34,    35,    -1,    37,
      38,    39,    40,    -1,    42,    43,    44,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    53,    54,    -1,    -1,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    -1,    75,    76,    77,
      78,    79,    -1,    -1,    82,    83,    84,    85,    86,    87,
      -1,    -1,    -1,    -1,    -1,    93,    -1,    -1,    -1,    -1,
      -1,     6,    -1,   101,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    -1,    39,    40,    41,    42,    43,    44,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    53,    54,
      -1,    -1,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    -1,
      75,    76,    77,    78,    79,    -1,    -1,    82,    83,    84,
      85,    86,    87,    88,    89,    -1,     6,    -1,    93,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    -1,    39,
      40,    41,    42,    43,    44,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    53,    54,    -1,    -1,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    -1,    75,    76,    77,    78,    79,
      -1,    -1,    82,    83,    84,    85,    86,    87,    88,    89,
      -1,     6,    -1,    93,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    -1,    39,    40,    41,    42,    43,    44,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    53,    54,
      -1,    -1,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    -1,
      75,    76,    77,    78,    79,    -1,    -1,    82,    83,    84,
      85,    86,    87,    88,    89,     6,    -1,    -1,    93,    10,
      11,    12,    13,    14,    15,    16,    17,    -1,    19,    20,
      21,    22,    23,    -1,    25,    26,    -1,    28,    29,    30,
      31,    32,    33,    34,    35,    -1,    37,    -1,    39,    40,
      -1,    42,    43,    44,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    53,    54,    -1,    -1,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    -1,    75,    76,    77,    78,    79,    31,
      -1,    82,    83,    84,    85,    37,    87,    -1,    -1,    -1,
      -1,    -1,    93,    45,    46,    47,    48,    49,    50,    -1,
      52,    31,    -1,    55,    56,    57,    -1,    37,    -1,    -1,
      -1,    -1,    -1,    -1,    31,    45,    46,    47,    48,    49,
      50,    -1,    52,    -1,    -1,    55,    56,    57,    45,    46,
      47,    48,    49,    50,    -1,    52,    31,    -1,    55,    56,
      57,    -1,    94,    95,    96,    97,    98,    99,   100,    -1,
      45,    46,    47,    48,    49,    50,    -1,    52,    -1,    -1,
      55,    56,    57,    -1,    94,    95,    96,    97,    98,    99,
     100,    31,    -1,    -1,    -1,    -1,    -1,    94,    95,    96,
      97,    98,    99,   100,    -1,    45,    46,    47,    48,    49,
      50,    -1,    52,    31,    -1,    55,    56,    57,    -1,    94,
      95,    96,    97,    98,    99,   100,    -1,    45,    46,    -1,
      48,    49,    50,    -1,    52,    -1,    -1,    55,    56,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    36,    -1,    94,    95,    96,    97,    98,    99,
     100,    45,    46,    47,    48,    49,    50,    -1,    52,    -1,
      -1,    55,    56,    57,    -1,    -1,    94,    95,    96,    97,
      98,    99,   100,    45,    46,    47,    48,    49,    50,    -1,
      52,    -1,    -1,    55,    56,    57,    45,    46,    47,    48,
      49,    50,    -1,    52,    -1,    -1,    55,    56,    57,    -1,
      94,    95,    96,    97,    98,    99,   100,    45,    46,    47,
      48,    49,    50,    -1,    52,    -1,    -1,    55,    56,    57,
      -1,    -1,    94,    95,    96,    97,    98,    99,   100,    -1,
     102,    -1,   104,    -1,    -1,    94,    95,    96,    97,    98,
      99,   100,    -1,   102,    -1,   104,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    94,    95,    96,    97,
      98,    99,   100,    -1,    -1,    -1,   104,    45,    46,    47,
      48,    49,    50,    -1,    52,    -1,    -1,    55,    56,    57,
      45,    46,    47,    48,    49,    50,    -1,    52,    -1,    -1,
      55,    56,    57,    45,    46,    47,    48,    49,    50,    -1,
      52,    -1,    -1,    55,    56,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    94,    95,    96,    97,
      98,    99,   100,    -1,   102,    -1,    -1,    -1,    -1,    94,
      95,    96,    97,    98,    99,   100,    -1,   102,    -1,    -1,
      -1,    -1,    94,    95,    96,    97,    98,    99,   100,    -1,
     102,    45,    46,    47,    48,    49,    50,    -1,    52,    -1,
      -1,    55,    56,    57,    -1,    -1,    -1,    -1,    -1,    45,
      46,    47,    48,    49,    50,    -1,    52,    -1,    -1,    55,
      56,    57,    45,    46,    47,    48,    49,    50,    -1,    52,
      -1,    -1,    55,    56,    57,    -1,    -1,    -1,    -1,    -1,
      94,    95,    96,    97,    98,    99,   100,   101,    84,    85,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,    95,
      96,    97,    98,    99,   100,    -1,    -1,    -1,    -1,    -1,
      -1,    94,    95,    96,    97,    98,    99,   100,    45,    46,
      47,    48,    49,    50,    -1,    52,    -1,    -1,    55,    56,
      57,    45,    46,    -1,    48,    -1,    -1,    -1,    52,    -1,
      -1,    55,    56,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,    95,    96,
      97,    98,    99,   100,    -1,    -1,    -1,    -1,    -1,    -1,
      94,    95,    96,    97,    98,    99,   100
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     6,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    37,    38,
      39,    40,    41,    42,    43,    44,    53,    54,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    75,    76,    77,    78,    79,    82,
      83,    84,    85,    86,    87,    88,    90,    93,   101,   110,
     111,   112,   113,   114,   119,   121,   125,   126,   127,   128,
     132,   134,   135,   136,   138,   139,   140,     9,    16,    18,
      19,    21,    22,    23,    24,    27,    28,    30,    33,    34,
      35,    36,    41,    42,    43,    44,    86,    87,    88,    89,
      93,   120,   121,   122,    10,    11,    14,    15,    58,    60,
      62,    64,    78,   151,     1,    34,   101,   122,   123,   124,
       4,     5,     7,     8,    10,    11,    12,    13,    14,    15,
      20,    29,    51,    58,    60,    62,    64,    78,    83,    97,
      98,   103,   106,   122,   129,   131,   147,   148,   151,   153,
     155,   158,   159,   163,   166,   167,   168,   171,   151,   103,
     168,   122,     1,   101,   168,    34,   129,   168,    39,    40,
      73,    75,    78,    82,    83,   148,   150,   161,   162,   167,
     101,   122,   129,   168,   129,   131,   123,   168,   123,   122,
     101,   127,     0,   112,   129,   168,   101,   102,   103,   154,
     167,   154,   167,   154,   167,   154,   168,   168,   168,   168,
      72,     1,   101,   101,     1,   101,     1,   101,   102,   154,
     154,   168,   168,   167,    66,    67,    68,    69,    70,    72,
      77,   122,   168,   167,   167,   104,   148,   168,     4,     5,
       7,     8,   108,   122,   164,   165,   168,   172,   173,   103,
       1,   101,     1,   101,   105,    31,    45,    46,    47,    48,
      49,    50,    52,    55,    56,    57,    94,    95,    96,    97,
      98,    99,   100,   102,   130,   147,   169,   171,     1,   101,
     168,    36,   123,   101,     1,   101,     1,   101,     1,   101,
      26,    53,    54,   122,   168,   167,   170,   171,   170,   122,
     170,   170,    37,    47,   133,   105,     1,   101,     1,   101,
      39,     1,   101,     1,   101,     1,   101,     1,    37,   101,
       1,   101,   123,     1,   101,   122,   104,   168,    31,    31,
      31,    37,    31,    37,    31,    37,    31,    37,    58,    60,
      62,    64,   152,   101,   101,   101,   122,    31,   169,    71,
      79,    71,    79,    71,    79,    71,    79,    71,    79,    31,
      31,    84,    85,   169,   102,   104,   108,   108,   108,   108,
     107,   108,   102,   107,   107,   102,   104,   148,   173,   101,
     101,   122,    29,   168,   168,   168,   168,   168,   168,   168,
     168,   168,   168,   168,   168,   168,   168,   168,   168,   168,
     173,    45,    46,    47,    48,    49,    50,    52,    55,    56,
      57,    94,    95,    96,    97,    98,    99,   100,   102,   130,
     101,     1,   101,   125,     1,   101,   101,   101,   101,    78,
     122,   148,   149,   151,   149,   149,    25,    47,     1,   101,
      45,    46,    48,    49,    50,    52,    55,    56,    57,    94,
      95,    96,    97,    98,    99,   100,    31,    31,   168,   122,
     101,     9,    89,   101,   115,   116,   117,   118,   101,   168,
     101,   101,   101,   101,   127,   101,   125,   141,   142,   143,
     101,     1,   101,   101,   102,    15,    15,   167,   168,   167,
     168,   167,   168,   167,   168,    25,    31,   160,    29,    59,
      61,    63,    65,    73,    74,    75,    76,    80,    81,    82,
      83,   156,   157,   167,   156,   167,   167,   102,   130,   104,
     173,   168,   168,   168,   168,   168,   165,   168,   168,   169,
     102,   130,   168,   130,   168,   168,   168,   168,   168,   168,
     168,   168,   168,   168,   168,   168,   168,   168,   168,   168,
     168,   173,   101,   101,   126,   144,   145,   146,    18,   101,
     141,     1,   101,     1,   101,     1,   101,   168,   168,   101,
     141,   170,   170,   170,   170,   170,   170,   170,   170,   170,
     170,   170,   170,   170,   170,   170,   170,    73,    73,    75,
      82,    83,   162,   167,     1,   101,   116,   120,   122,   118,
     101,   141,    92,   143,   101,   141,   104,   173,   167,   167,
      31,    31,    31,    31,   167,   168,   160,   160,   160,   160,
     167,   167,   160,   167,   167,   173,   104,   130,   102,   130,
     104,   130,   144,    18,    90,   137,   146,   101,   125,   141,
     101,   101,   101,     1,   101,    17,    37,   141,    91,   170,
     167,   167,   167,   167,   101,   101,   123,    92,   101,   141,
       9,   130,   167,   167,   167,   167,   167,   167,   167,   167,
      31,    73,   157,   130,   104,   173,   104,   137,   101,   125,
     101,   144,   101,   141,    37,   168,    91,   101,     1,   101,
     101,     9,   120,   104,    73,   104,   130,   144,   137,   141,
      91,   168,     1,   101,   101,   101,   141,   120,   101,   167,
     104,   137,    91,   101,     1,   101,   101,   141,   141,   101,
     101,   101,   141,   141,    91,   141,    91,    91,   101,    91,
     101,   101,   101
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,   109,   110,   111,   111,   112,   112,   112,   112,   112,
     112,   113,   113,   114,   114,   115,   115,   116,   116,   117,
     117,   118,   118,   118,   119,   119,   119,   119,   120,   120,
     120,   121,   121,   121,   121,   121,   121,   121,   121,   121,
     121,   121,   121,   121,   121,   121,   121,   121,   121,   121,
     121,   121,   121,   121,   121,   121,   121,   121,   121,   121,
     121,   121,   121,   121,   121,   121,   121,   121,   121,   121,
     121,   121,   121,   121,   121,   121,   121,   121,   121,   121,
     122,   122,   122,   122,   122,   122,   122,   122,   122,   122,
     122,   122,   122,   122,   122,   122,   122,   122,   122,   122,
     123,   123,   123,   124,   124,   125,   125,   126,   126,   126,
     126,   126,   126,   127,   127,   127,   128,   128,   128,   128,
     128,   128,   128,   128,   128,   128,   128,   128,   128,   128,
     128,   128,   128,   128,   128,   128,   128,   128,   128,   128,
     128,   128,   128,   128,   128,   128,   129,   129,   129,   129,
     129,   129,   129,   129,   129,   129,   130,   130,   131,   131,
     131,   131,   131,   132,   132,   132,   132,   132,   132,   132,
     132,   133,   133,   134,   134,   134,   134,   134,   134,   135,
     135,   135,   136,   136,   136,   136,   137,   137,   138,   138,
     138,   138,   138,   138,   138,   138,   139,   139,   139,   140,
     141,   141,   142,   142,   143,   143,   144,   144,   145,   145,
     146,   146,   147,   147,   147,   147,   147,   147,   147,   147,
     147,   147,   147,   147,   147,   147,   147,   147,   148,   149,
     149,   150,   150,   151,   151,   151,   151,   151,   151,   151,
     151,   151,   151,   151,   151,   151,   151,   151,   152,   152,
     152,   152,   153,   153,   154,   154,   154,   154,   155,   155,
     155,   155,   155,   156,   156,   156,   156,   156,   157,   158,
     158,   158,   158,   158,   158,   158,   158,   158,   158,   159,
     159,   159,   159,   159,   159,   159,   159,   160,   160,   161,
     161,   161,   162,   162,   162,   162,   162,   163,   163,   163,
     164,   164,   164,   165,   165,   165,   165,   165,   166,   166,
     167,   167,   168,   168,   168,   168,   168,   168,   168,   168,
     168,   168,   168,   168,   168,   168,   168,   168,   168,   168,
     168,   169,   169,   169,   169,   169,   169,   169,   169,   169,
     169,   169,   169,   169,   169,   169,   169,   169,   169,   169,
     170,   170,   170,   170,   170,   170,   170,   170,   170,   170,
     170,   170,   170,   170,   170,   170,   170,   170,   171,   171,
     172,   172,   173,   173
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     1,     1,
       3,     5,     6,     4,     5,     5,     6,     0,     1,     1,
       2,     1,     1,     3,     8,     9,     5,     6,     0,     1,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       0,     1,     2,     1,     3,     1,     2,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     4,     3,     4,
       3,     4,     3,     4,     3,     4,     3,     4,     3,     4,
       5,     3,     4,     3,     4,     2,     3,     2,     3,     3,
       4,     3,     4,     3,     4,     2,     0,     2,     4,     3,
       5,     2,     4,     6,     5,     7,     0,     1,     2,     2,
       5,     4,     3,     5,     6,     5,     6,     5,     6,     5,
       6,     1,     1,     3,     4,     3,     4,     3,     4,     4,
       6,     7,     6,     8,     7,     9,     0,     2,     7,     8,
      11,    12,    12,    13,     9,    10,     4,     6,     7,     2,
       0,     1,     1,     2,     1,     1,     0,     1,     1,     2,
       1,     1,     1,     1,     1,     1,     2,     3,     5,     6,
       8,     3,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     5,     2,     5,     2,     2,     4,
       6,     4,     6,     4,     6,     4,     6,     5,     1,     1,
       1,     1,     2,     2,     1,     2,     4,     6,     2,     4,
       4,     1,     1,     1,     1,     5,     2,     2,     2,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     6,
       6,     6,     6,     6,     4,     4,     4,     1,     1,     2,
       4,     3,     1,     2,     5,     2,     2,     3,     3,     3,
       1,     3,     3,     3,     3,     3,     3,     3,     2,     2,
       1,     1,     1,     1,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     1,     1,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       1,     1,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     4,     4,
       0,     1,     1,     3
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


/* Context of a parse error.  */
typedef struct
{
  yy_state_t *yyssp;
  yysymbol_kind_t yytoken;
} yypcontext_t;

/* Put in YYARG at most YYARGN of the expected tokens given the
   current YYCTX, and return the number of tokens stored in YYARG.  If
   YYARG is null, return the number of expected tokens (guaranteed to
   be less than YYNTOKENS).  Return YYENOMEM on memory exhaustion.
   Return 0 if there are more than YYARGN expected tokens, yet fill
   YYARG up to YYARGN. */
static int
yypcontext_expected_tokens (const yypcontext_t *yyctx,
                            yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  int yyn = yypact[+*yyctx->yyssp];
  if (!yypact_value_is_default (yyn))
    {
      /* Start YYX at -YYN if negative to avoid negative indexes in
         YYCHECK.  In other words, skip the first -YYN actions for
         this state because they are default actions.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;
      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yyx;
      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
        if (yycheck[yyx + yyn] == yyx && yyx != YYSYMBOL_YYerror
            && !yytable_value_is_error (yytable[yyx + yyn]))
          {
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = YY_CAST (yysymbol_kind_t, yyx);
          }
    }
  if (yyarg && yycount == 0 && 0 < yyargn)
    yyarg[0] = YYSYMBOL_YYEMPTY;
  return yycount;
}




/* The kind of the lookahead of this context.  */
static yysymbol_kind_t
yypcontext_token (const yypcontext_t *yyctx) YY_ATTRIBUTE_UNUSED;

static yysymbol_kind_t
yypcontext_token (const yypcontext_t *yyctx)
{
  return yyctx->yytoken;
}



/* User defined function to report a syntax error.  */
static int
yyreport_syntax_error (const yypcontext_t *yyctx);

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  switch (yykind)
    {
    case YYSYMBOL_tVARID: /* tVARID  */
#line 218 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2452 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tSTRING: /* tSTRING  */
#line 218 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2458 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tSYMBOL: /* tSYMBOL  */
#line 218 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2464 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tENDCLAUSE: /* tENDCLAUSE  */
#line 218 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2470 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_script: /* script  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2476 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_scriptpart: /* scriptpart  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2482 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_macro: /* macro  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2488 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_factory: /* factory  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2494 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_method: /* method  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2500 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_methodlistline: /* methodlistline  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2506 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_handler: /* handler  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2512 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_CMDID: /* CMDID  */
#line 218 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2518 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_ID: /* ID  */
#line 218 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).s); }
#line 2524 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_stmt: /* stmt  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2530 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_stmt_insideif: /* stmt_insideif  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2536 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_stmtoneliner: /* stmtoneliner  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2542 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_proc: /* proc  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2548 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_asgn: /* asgn  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2554 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_definevars: /* definevars  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2560 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_ifstmt: /* ifstmt  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2566 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_ifelsestmt: /* ifelsestmt  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2572 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_loop: /* loop  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2578 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_tell: /* tell  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2584 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_when: /* when  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2590 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_stmtlistline: /* stmtlistline  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2596 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_stmtlistline_insideif: /* stmtlistline_insideif  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2602 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_simpleexpr_nounarymath: /* simpleexpr_nounarymath  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2608 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_var: /* var  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2614 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_varorchunk: /* varorchunk  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2620 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_varorthe: /* varorthe  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2626 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_chunk: /* chunk  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2632 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_object: /* object  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2638 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_the: /* the  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2644 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_theobj: /* theobj  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2650 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_menu: /* menu  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2656 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_thedatetime: /* thedatetime  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2662 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_thenumberof: /* thenumberof  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2668 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_writablethe: /* writablethe  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2674 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_writabletheobj: /* writabletheobj  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2680 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_list: /* list  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2686 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_proppair: /* proppair  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2692 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_unarymath: /* unarymath  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2698 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_simpleexpr: /* simpleexpr  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2704 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_expr: /* expr  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2710 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_expr_nounarymath: /* expr_nounarymath  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2716 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_expr_noeq: /* expr_noeq  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2722 "engines/director/lingo/lingo-gr.cpp"
        break;

    case YYSYMBOL_sprite: /* sprite  */
#line 219 "engines/director/lingo/lingo-gr.y"
            { delete ((*yyvaluep).node); }
#line 2728 "engines/director/lingo/lingo-gr.cpp"
        break;

      default:
        break;
    }
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* script: scriptpartlist  */
#line 225 "engines/director/lingo/lingo-gr.y"
                                                        { g_lingo->_compiler->_assemblyAST = Common::SharedPtr<Node>(new ScriptNode((yyvsp[0].nodelist))); (yyval.node) = nullptr; }
#line 2998 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 3: /* scriptpartlist: scriptpart  */
#line 227 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 3009 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 4: /* scriptpartlist: scriptpartlist scriptpart  */
#line 233 "engines/director/lingo/lingo-gr.y"
                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3019 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 5: /* scriptpart: '\n'  */
#line 240 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = nullptr; }
#line 3025 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 10: /* scriptpart: tENDCLAUSE endargdef '\n'  */
#line 245 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = nullptr; delete (yyvsp[-2].s); }
#line 3031 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 11: /* macro: tMACRO ID idlist '\n' stmtlist  */
#line 274 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 3037 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 12: /* macro: tMACRO ID idlist error '\n' stmtlist  */
#line 275 "engines/director/lingo/lingo-gr.y"
                                                { TRIM_GARBAGE((yyval.node), new HandlerNode((yyvsp[-4].s), (yyvsp[-3].idlist), (yyvsp[0].nodelist))); }
#line 3043 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 13: /* factory: tFACTORY ID '\n' methodlist  */
#line 280 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new FactoryNode((yyvsp[-2].s), (yyvsp[0].nodelist)); }
#line 3049 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 14: /* factory: tFACTORY ID error '\n' methodlist  */
#line 281 "engines/director/lingo/lingo-gr.y"
                                                { TRIM_GARBAGE((yyval.node), new FactoryNode((yyvsp[-3].s), (yyvsp[0].nodelist))); }
#line 3055 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 15: /* method: tMETHOD ID idlist '\n' stmtlist  */
#line 284 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 3061 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 16: /* method: tMETHOD ID idlist error '\n' stmtlist  */
#line 285 "engines/director/lingo/lingo-gr.y"
                                                        { TRIM_GARBAGE((yyval.node), new HandlerNode((yyvsp[-4].s), (yyvsp[-3].idlist), (yyvsp[0].nodelist))); }
#line 3067 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 17: /* methodlist: %empty  */
#line 288 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.nodelist) = new NodeList; }
#line 3073 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 19: /* nonemptymethodlist: methodlistline  */
#line 292 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 3084 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 20: /* nonemptymethodlist: nonemptymethodlist methodlistline  */
#line 298 "engines/director/lingo/lingo-gr.y"
                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3094 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 21: /* methodlistline: '\n'  */
#line 305 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = nullptr; }
#line 3100 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 23: /* methodlistline: tENDCLAUSE endargdef '\n'  */
#line 307 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = nullptr; delete (yyvsp[-2].s); }
#line 3106 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 24: /* handler: tON ID idlist '\n' stmtlist tENDCLAUSE endargdef '\n'  */
#line 312 "engines/director/lingo/lingo-gr.y"
                                                               {	// D3
		(yyval.node) = new HandlerNode((yyvsp[-6].s), (yyvsp[-5].idlist), (yyvsp[-3].nodelist));
		checkEnd((yyvsp[-2].s), (yyvsp[-6].s), false);
		delete (yyvsp[-2].s); }
#line 3115 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 25: /* handler: tON ID idlist error '\n' stmtlist tENDCLAUSE endargdef '\n'  */
#line 316 "engines/director/lingo/lingo-gr.y"
                                                                      {	// D3
		TRIM_GARBAGE((yyval.node), new HandlerNode((yyvsp[-7].s), (yyvsp[-6].idlist), (yyvsp[-3].nodelist)));
		checkEnd((yyvsp[-2].s), (yyvsp[-7].s), false);
		delete (yyvsp[-2].s); }
#line 3124 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 26: /* handler: tON ID idlist '\n' stmtlist  */
#line 320 "engines/director/lingo/lingo-gr.y"
                                      {	// D4. No 'end' clause
		(yyval.node) = new HandlerNode((yyvsp[-3].s), (yyvsp[-2].idlist), (yyvsp[0].nodelist)); }
#line 3131 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 27: /* handler: tON ID idlist error '\n' stmtlist  */
#line 322 "engines/director/lingo/lingo-gr.y"
                                            {	// D4. No 'end' clause
		TRIM_GARBAGE((yyval.node), new HandlerNode((yyvsp[-4].s), (yyvsp[-3].idlist), (yyvsp[0].nodelist))); }
#line 3138 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 29: /* endargdef: ID  */
#line 327 "engines/director/lingo/lingo-gr.y"
                                                        { delete (yyvsp[0].s); }
#line 3144 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 30: /* endargdef: endargdef ',' ID  */
#line 328 "engines/director/lingo/lingo-gr.y"
                                                { delete (yyvsp[0].s); }
#line 3150 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 32: /* CMDID: tABBREVIATED  */
#line 336 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("abbreviated"); }
#line 3156 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 33: /* CMDID: tABBREV  */
#line 337 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("abbrev"); }
#line 3162 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 34: /* CMDID: tABBR  */
#line 338 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("abbr"); }
#line 3168 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 35: /* CMDID: tAFTER  */
#line 339 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("after"); }
#line 3174 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 36: /* CMDID: tBEFORE  */
#line 340 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("before"); }
#line 3180 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 37: /* CMDID: tCAST  */
#line 341 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("cast"); }
#line 3186 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 38: /* CMDID: tCASTLIB  */
#line 342 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("castLib"); }
#line 3192 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 39: /* CMDID: tCHAR  */
#line 343 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("char"); }
#line 3198 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 40: /* CMDID: tCHARS  */
#line 344 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("chars"); }
#line 3204 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 41: /* CMDID: tDATE  */
#line 345 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("date"); }
#line 3210 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 42: /* CMDID: tDELETE  */
#line 346 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("delete"); }
#line 3216 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 43: /* CMDID: tDOWN  */
#line 347 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("down"); }
#line 3222 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 44: /* CMDID: tFIELD  */
#line 348 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("field"); }
#line 3228 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 45: /* CMDID: tFRAME  */
#line 349 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("frame"); }
#line 3234 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 46: /* CMDID: tHILITE  */
#line 350 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("hilite"); }
#line 3240 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 47: /* CMDID: tIN  */
#line 351 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("in"); }
#line 3246 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 48: /* CMDID: tINTERSECTS  */
#line 352 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("intersects"); }
#line 3252 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 49: /* CMDID: tINTO  */
#line 353 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("into"); }
#line 3258 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 50: /* CMDID: tITEM  */
#line 354 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("item"); }
#line 3264 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 51: /* CMDID: tITEMS  */
#line 355 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("items"); }
#line 3270 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 52: /* CMDID: tLAST  */
#line 356 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("last"); }
#line 3276 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 53: /* CMDID: tLINE  */
#line 357 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("line"); }
#line 3282 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 54: /* CMDID: tLINES  */
#line 358 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("lines"); }
#line 3288 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 55: /* CMDID: tLONG  */
#line 359 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("long"); }
#line 3294 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 56: /* CMDID: tMEMBER  */
#line 360 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("member"); }
#line 3300 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 57: /* CMDID: tMENU  */
#line 361 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("menu"); }
#line 3306 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 58: /* CMDID: tMENUITEM  */
#line 362 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("menuItem"); }
#line 3312 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 59: /* CMDID: tMENUITEMS  */
#line 363 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("menuItems"); }
#line 3318 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 60: /* CMDID: tMOVIE  */
#line 364 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("movie"); }
#line 3324 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 61: /* CMDID: tNEXT  */
#line 365 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("next"); }
#line 3330 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 62: /* CMDID: tNUMBER  */
#line 366 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("number"); }
#line 3336 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 63: /* CMDID: tOF  */
#line 367 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("of"); }
#line 3342 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 64: /* CMDID: tPREVIOUS  */
#line 368 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("previous"); }
#line 3348 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 65: /* CMDID: tREPEAT  */
#line 369 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("repeat"); }
#line 3354 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 66: /* CMDID: tSCRIPT  */
#line 370 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("script"); }
#line 3360 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 67: /* CMDID: tASSERTERROR  */
#line 371 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("scummvmAssertError"); }
#line 3366 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 68: /* CMDID: tSHORT  */
#line 372 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("short"); }
#line 3372 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 69: /* CMDID: tSOUND  */
#line 373 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("sound"); }
#line 3378 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 70: /* CMDID: tSPRITE  */
#line 374 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("sprite"); }
#line 3384 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 71: /* CMDID: tTHE  */
#line 375 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("the"); }
#line 3390 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 72: /* CMDID: tTIME  */
#line 376 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("time"); }
#line 3396 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 73: /* CMDID: tTO  */
#line 377 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("to"); }
#line 3402 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 74: /* CMDID: tWHILE  */
#line 378 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("while"); }
#line 3408 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 75: /* CMDID: tWINDOW  */
#line 379 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("window"); }
#line 3414 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 76: /* CMDID: tWITH  */
#line 380 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("with"); }
#line 3420 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 77: /* CMDID: tWITHIN  */
#line 381 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("within"); }
#line 3426 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 78: /* CMDID: tWORD  */
#line 382 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("word"); }
#line 3432 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 79: /* CMDID: tWORDS  */
#line 383 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("words"); }
#line 3438 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 81: /* ID: tELSE  */
#line 387 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("else"); }
#line 3444 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 82: /* ID: tENDCLAUSE  */
#line 388 "engines/director/lingo/lingo-gr.y"
                        { (yyval.s) = new Common::String("end"); delete (yyvsp[0].s); }
#line 3450 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 83: /* ID: tEXIT  */
#line 389 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("exit"); }
#line 3456 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 84: /* ID: tFACTORY  */
#line 390 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("factory"); }
#line 3462 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 85: /* ID: tGLOBAL  */
#line 391 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("global"); }
#line 3468 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 86: /* ID: tGO  */
#line 392 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("go"); }
#line 3474 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 87: /* ID: tIF  */
#line 393 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("if"); }
#line 3480 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 88: /* ID: tINSTANCE  */
#line 394 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("instance"); }
#line 3486 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 89: /* ID: tMACRO  */
#line 395 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("macro"); }
#line 3492 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 90: /* ID: tMETHOD  */
#line 396 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("method"); }
#line 3498 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 91: /* ID: tON  */
#line 397 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("on"); }
#line 3504 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 92: /* ID: tOPEN  */
#line 398 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("open"); }
#line 3510 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 93: /* ID: tPLAY  */
#line 399 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("play"); }
#line 3516 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 94: /* ID: tPROPERTY  */
#line 400 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("property"); }
#line 3522 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 95: /* ID: tPUT  */
#line 401 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("put"); }
#line 3528 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 96: /* ID: tRETURN  */
#line 402 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("return"); }
#line 3534 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 97: /* ID: tSET  */
#line 403 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("set"); }
#line 3540 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 98: /* ID: tTELL  */
#line 404 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("tell"); }
#line 3546 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 99: /* ID: tTHEN  */
#line 405 "engines/director/lingo/lingo-gr.y"
                                { (yyval.s) = new Common::String("then"); }
#line 3552 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 100: /* idlist: %empty  */
#line 408 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.idlist) = new IDList; }
#line 3558 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 103: /* nonemptyidlist: ID  */
#line 413 "engines/director/lingo/lingo-gr.y"
                                                                {
		Common::Array<Common::String *> *list = new IDList;
		list->push_back((yyvsp[0].s));
		(yyval.idlist) = list; }
#line 3567 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 104: /* nonemptyidlist: nonemptyidlist ',' ID  */
#line 417 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyvsp[-2].idlist)->push_back((yyvsp[0].s));
		(yyval.idlist) = (yyvsp[-2].idlist); }
#line 3575 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 106: /* stmt: tENDIF '\n'  */
#line 428 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = nullptr; }
#line 3581 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 116: /* proc: CMDID cmdargs '\n'  */
#line 444 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode((yyvsp[-2].s), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3587 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 117: /* proc: CMDID cmdargs error '\n'  */
#line 445 "engines/director/lingo/lingo-gr.y"
                                                        { TRIM_GARBAGE((yyval.node), new CmdNode((yyvsp[-3].s), (yyvsp[-2].nodelist), g_lingo->_compiler->_linenumber - 1)); }
#line 3593 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 118: /* proc: tPUT cmdargs '\n'  */
#line 446 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new CmdNode(new Common::String("put"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3599 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 119: /* proc: tPUT cmdargs error '\n'  */
#line 447 "engines/director/lingo/lingo-gr.y"
                                                                        { TRIM_GARBAGE((yyval.node), new CmdNode(new Common::String("put"), (yyvsp[-2].nodelist), g_lingo->_compiler->_linenumber - 1)); }
#line 3605 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 120: /* proc: tGO cmdargs '\n'  */
#line 448 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new CmdNode(new Common::String("go"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3611 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 121: /* proc: tGO cmdargs error '\n'  */
#line 449 "engines/director/lingo/lingo-gr.y"
                                                                        { TRIM_GARBAGE((yyval.node), new CmdNode(new Common::String("go"), (yyvsp[-2].nodelist), g_lingo->_compiler->_linenumber - 1)); }
#line 3617 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 122: /* proc: tGO frameargs '\n'  */
#line 450 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("go"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3623 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 123: /* proc: tGO frameargs error '\n'  */
#line 451 "engines/director/lingo/lingo-gr.y"
                                                                { TRIM_GARBAGE((yyval.node), new CmdNode(new Common::String("go"), (yyvsp[-2].nodelist), g_lingo->_compiler->_linenumber - 1)); }
#line 3629 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 124: /* proc: tPLAY cmdargs '\n'  */
#line 452 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("play"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3635 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 125: /* proc: tPLAY cmdargs error '\n'  */
#line 453 "engines/director/lingo/lingo-gr.y"
                                                                { TRIM_GARBAGE((yyval.node), new CmdNode(new Common::String("play"), (yyvsp[-2].nodelist), g_lingo->_compiler->_linenumber - 1)); }
#line 3641 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 126: /* proc: tPLAY frameargs '\n'  */
#line 454 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("play"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3647 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 127: /* proc: tPLAY frameargs error '\n'  */
#line 455 "engines/director/lingo/lingo-gr.y"
                                                                { TRIM_GARBAGE((yyval.node), new CmdNode(new Common::String("play"), (yyvsp[-2].nodelist), g_lingo->_compiler->_linenumber - 1)); }
#line 3653 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 128: /* proc: tOPEN cmdargs '\n'  */
#line 456 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new CmdNode(new Common::String("open"), (yyvsp[-1].nodelist), g_lingo->_compiler->_linenumber - 1); }
#line 3659 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 129: /* proc: tOPEN cmdargs error '\n'  */
#line 457 "engines/director/lingo/lingo-gr.y"
                                                                { TRIM_GARBAGE((yyval.node), new CmdNode(new Common::String("open"), (yyvsp[-2].nodelist), g_lingo->_compiler->_linenumber - 1)); }
#line 3665 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 130: /* proc: tOPEN expr tWITH expr '\n'  */
#line 458 "engines/director/lingo/lingo-gr.y"
                                                 {
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-3].node));
		args->push_back((yyvsp[-1].node));
		(yyval.node) = new CmdNode(new Common::String("open"), args, g_lingo->_compiler->_linenumber - 1); }
#line 3675 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 131: /* proc: tNEXT tREPEAT '\n'  */
#line 463 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new NextRepeatNode(); }
#line 3681 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 132: /* proc: tNEXT tREPEAT error '\n'  */
#line 464 "engines/director/lingo/lingo-gr.y"
                                                                { TRIM_GARBAGE((yyval.node), new NextRepeatNode()); }
#line 3687 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 133: /* proc: tEXIT tREPEAT '\n'  */
#line 465 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new ExitRepeatNode(); }
#line 3693 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 134: /* proc: tEXIT tREPEAT error '\n'  */
#line 466 "engines/director/lingo/lingo-gr.y"
                                                                { TRIM_GARBAGE((yyval.node), new ExitRepeatNode()); }
#line 3699 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 135: /* proc: tEXIT '\n'  */
#line 467 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new ExitNode(); }
#line 3705 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 136: /* proc: tEXIT error '\n'  */
#line 468 "engines/director/lingo/lingo-gr.y"
                                                                        { TRIM_GARBAGE((yyval.node), new ExitNode()); }
#line 3711 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 137: /* proc: tRETURN '\n'  */
#line 469 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new ReturnNode(nullptr); }
#line 3717 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 138: /* proc: tRETURN error '\n'  */
#line 470 "engines/director/lingo/lingo-gr.y"
                                                                        { TRIM_GARBAGE((yyval.node), new ReturnNode(nullptr)); }
#line 3723 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 139: /* proc: tRETURN expr '\n'  */
#line 471 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new ReturnNode((yyvsp[-1].node)); }
#line 3729 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 140: /* proc: tRETURN expr error '\n'  */
#line 472 "engines/director/lingo/lingo-gr.y"
                                                                        { TRIM_GARBAGE((yyval.node), new ReturnNode((yyvsp[-2].node))); }
#line 3735 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 141: /* proc: tDELETE chunk '\n'  */
#line 473 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new DeleteNode((yyvsp[-1].node)); }
#line 3741 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 142: /* proc: tDELETE chunk error '\n'  */
#line 474 "engines/director/lingo/lingo-gr.y"
                                                                { TRIM_GARBAGE((yyval.node), new DeleteNode((yyvsp[-2].node))); }
#line 3747 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 143: /* proc: tHILITE chunk '\n'  */
#line 475 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new HiliteNode((yyvsp[-1].node)); }
#line 3753 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 144: /* proc: tHILITE chunk error '\n'  */
#line 476 "engines/director/lingo/lingo-gr.y"
                                                                { TRIM_GARBAGE((yyval.node), new HiliteNode((yyvsp[-2].node))); }
#line 3759 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 145: /* proc: tASSERTERROR stmtoneliner  */
#line 477 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new AssertErrorNode((yyvsp[0].node)); }
#line 3765 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 146: /* cmdargs: %empty  */
#line 480 "engines/director/lingo/lingo-gr.y"
                                                                                        {
		// This matches `cmd`
		(yyval.nodelist) = new NodeList; }
#line 3773 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 147: /* cmdargs: expr trailingcomma  */
#line 483 "engines/director/lingo/lingo-gr.y"
                                                                                        {
		// This matches `cmd arg` and `cmd(arg)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-1].node));
		(yyval.nodelist) = args; }
#line 3783 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 148: /* cmdargs: expr ',' nonemptyexprlist trailingcomma  */
#line 488 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `cmd arg, ...)
		(yyvsp[-1].nodelist)->insert_at(0, (yyvsp[-3].node));
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3792 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 149: /* cmdargs: expr expr_nounarymath trailingcomma  */
#line 492 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `cmd arg arg`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-2].node));
		args->push_back((yyvsp[-1].node));
		(yyval.nodelist) = args; }
#line 3803 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 150: /* cmdargs: expr expr_nounarymath ',' nonemptyexprlist trailingcomma  */
#line 498 "engines/director/lingo/lingo-gr.y"
                                                                                {
		// This matches `cmd arg arg, ...`
		(yyvsp[-1].nodelist)->insert_at(0, (yyvsp[-3].node));
		(yyvsp[-1].nodelist)->insert_at(0, (yyvsp[-4].node));
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 3813 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 151: /* cmdargs: '(' ')'  */
#line 503 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `cmd()`
		(yyval.nodelist) = new NodeList; }
#line 3821 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 152: /* cmdargs: '(' expr ',' ')'  */
#line 506 "engines/director/lingo/lingo-gr.y"
                           {
		// This matches `cmd(arg,)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-2].node));
		(yyval.nodelist) = args; }
#line 3831 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 153: /* cmdargs: '(' expr ',' nonemptyexprlist trailingcomma ')'  */
#line 511 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `cmd(arg, ...)`
		(yyvsp[-2].nodelist)->insert_at(0, (yyvsp[-4].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 3840 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 154: /* cmdargs: '(' var expr_nounarymath trailingcomma ')'  */
#line 515 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `obj(method arg)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-3].node));
		args->push_back((yyvsp[-2].node));
		(yyval.nodelist) = args; }
#line 3851 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 155: /* cmdargs: '(' var expr_nounarymath ',' nonemptyexprlist trailingcomma ')'  */
#line 521 "engines/director/lingo/lingo-gr.y"
                                                                                        {
		// This matches `obj(method arg, ...)`
		(yyvsp[-2].nodelist)->insert_at(0, (yyvsp[-4].node));
		(yyvsp[-2].nodelist)->insert_at(0, (yyvsp[-5].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 3861 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 158: /* frameargs: tFRAME expr  */
#line 534 "engines/director/lingo/lingo-gr.y"
                                                                        {
		// This matches `play frame arg`
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3871 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 159: /* frameargs: tMOVIE expr  */
#line 539 "engines/director/lingo/lingo-gr.y"
                                                                                {
		// This matches `play movie arg`
		NodeList *args = new NodeList;
		args->push_back(new IntNode(1));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3882 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 160: /* frameargs: tFRAME expr tOF tMOVIE expr  */
#line 545 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `play frame arg of movie arg`
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[-3].node)));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3893 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 161: /* frameargs: expr tOF tMOVIE expr  */
#line 551 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `play arg of movie arg` (weird but valid)
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-3].node));
		args->push_back(new MovieNode((yyvsp[0].node)));
		(yyval.nodelist) = args; }
#line 3904 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 162: /* frameargs: tFRAME expr expr_nounarymath  */
#line 557 "engines/director/lingo/lingo-gr.y"
                                                        {
		// This matches `play frame arg arg` (also weird but valid)
		NodeList *args = new NodeList;
		args->push_back(new FrameNode((yyvsp[-1].node)));
		args->push_back((yyvsp[0].node));
		(yyval.nodelist) = args; }
#line 3915 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 163: /* asgn: tPUT expr tINTO varorchunk '\n'  */
#line 565 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new PutIntoNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3921 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 164: /* asgn: tPUT expr tINTO varorchunk error '\n'  */
#line 566 "engines/director/lingo/lingo-gr.y"
                                                { TRIM_GARBAGE((yyval.node), new PutIntoNode((yyvsp[-4].node), (yyvsp[-2].node))); }
#line 3927 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 165: /* asgn: tPUT expr tAFTER varorchunk '\n'  */
#line 567 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PutAfterNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3933 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 166: /* asgn: tPUT expr tAFTER varorchunk error '\n'  */
#line 568 "engines/director/lingo/lingo-gr.y"
                                                        { TRIM_GARBAGE((yyval.node), new PutAfterNode((yyvsp[-4].node), (yyvsp[-2].node))); }
#line 3939 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 167: /* asgn: tPUT expr tBEFORE varorchunk '\n'  */
#line 569 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PutBeforeNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3945 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 168: /* asgn: tPUT expr tBEFORE varorchunk error '\n'  */
#line 570 "engines/director/lingo/lingo-gr.y"
                                                        { TRIM_GARBAGE((yyval.node), new PutBeforeNode((yyvsp[-4].node), (yyvsp[-2].node))); }
#line 3951 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 169: /* asgn: tSET varorthe to expr '\n'  */
#line 571 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new SetNode((yyvsp[-3].node), (yyvsp[-1].node)); }
#line 3957 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 170: /* asgn: tSET varorthe to expr error '\n'  */
#line 572 "engines/director/lingo/lingo-gr.y"
                                                { TRIM_GARBAGE((yyval.node), new SetNode((yyvsp[-4].node), (yyvsp[-2].node))); }
#line 3963 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 173: /* definevars: tGLOBAL idlist '\n'  */
#line 577 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new GlobalNode((yyvsp[-1].idlist)); }
#line 3969 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 174: /* definevars: tGLOBAL idlist error '\n'  */
#line 578 "engines/director/lingo/lingo-gr.y"
                                                        { TRIM_GARBAGE((yyval.node), new GlobalNode((yyvsp[-2].idlist))); }
#line 3975 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 175: /* definevars: tPROPERTY idlist '\n'  */
#line 579 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new PropertyNode((yyvsp[-1].idlist)); }
#line 3981 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 176: /* definevars: tPROPERTY idlist error '\n'  */
#line 580 "engines/director/lingo/lingo-gr.y"
                                                                { TRIM_GARBAGE((yyval.node), new PropertyNode((yyvsp[-2].idlist))); }
#line 3987 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 177: /* definevars: tINSTANCE idlist '\n'  */
#line 581 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new InstanceNode((yyvsp[-1].idlist)); }
#line 3993 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 178: /* definevars: tINSTANCE idlist error '\n'  */
#line 582 "engines/director/lingo/lingo-gr.y"
                                                                { TRIM_GARBAGE((yyval.node), new InstanceNode((yyvsp[-2].idlist))); }
#line 3999 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 179: /* ifstmt: tIF expr tTHEN stmt  */
#line 585 "engines/director/lingo/lingo-gr.y"
                            {
		NodeList *stmtlist = new NodeList;
		stmtlist->push_back((yyvsp[0].node));
		(yyval.node) = new IfStmtNode((yyvsp[-2].node), stmtlist); }
#line 4008 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 180: /* ifstmt: tIF expr tTHEN '\n' stmtlist_insideif endif  */
#line 589 "engines/director/lingo/lingo-gr.y"
                                                      {
		(yyval.node) = new IfStmtNode((yyvsp[-4].node), (yyvsp[-1].nodelist)); }
#line 4015 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 181: /* ifstmt: tIF expr tTHEN error '\n' stmtlist_insideif endif  */
#line 591 "engines/director/lingo/lingo-gr.y"
                                                            {
		TRIM_GARBAGE((yyval.node), new IfStmtNode((yyvsp[-5].node), (yyvsp[-1].nodelist))); }
#line 4022 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 182: /* ifelsestmt: tIF expr tTHEN stmt tELSE stmt  */
#line 595 "engines/director/lingo/lingo-gr.y"
                                                         {
		NodeList *stmtlist1 = new NodeList;
		stmtlist1->push_back((yyvsp[-2].node));
		NodeList *stmtlist2 = new NodeList;
		stmtlist2->push_back((yyvsp[0].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-4].node), stmtlist1, stmtlist2); }
#line 4033 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 183: /* ifelsestmt: tIF expr tTHEN stmt tELSE '\n' stmtlist_insideif endif  */
#line 601 "engines/director/lingo/lingo-gr.y"
                                                                                   {
		NodeList *stmtlist1 = new NodeList;
		stmtlist1->push_back((yyvsp[-4].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-6].node), stmtlist1, (yyvsp[-1].nodelist)); }
#line 4042 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 184: /* ifelsestmt: tIF expr tTHEN '\n' stmtlist_insideif tELSE stmt  */
#line 605 "engines/director/lingo/lingo-gr.y"
                                                                             {
		NodeList *stmtlist2 = new NodeList;
		stmtlist2->push_back((yyvsp[0].node));
		(yyval.node) = new IfElseStmtNode((yyvsp[-5].node), (yyvsp[-2].nodelist), stmtlist2); }
#line 4051 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 185: /* ifelsestmt: tIF expr tTHEN '\n' stmtlist_insideif tELSE '\n' stmtlist_insideif endif  */
#line 609 "engines/director/lingo/lingo-gr.y"
                                                                                                         {
		(yyval.node) = new IfElseStmtNode((yyvsp[-7].node), (yyvsp[-4].nodelist), (yyvsp[-1].nodelist)); }
#line 4058 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 186: /* endif: %empty  */
#line 613 "engines/director/lingo/lingo-gr.y"
                        {
		LingoCompiler *compiler = g_lingo->_compiler;
		warning("LingoCompiler::parse: no end if at line %d col %d in %s id: %d",
			compiler->_linenumber, compiler->_colnumber, scriptType2str(compiler->_assemblyContext->_scriptType),
			compiler->_assemblyContext->_id);

		}
#line 4070 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 188: /* loop: tREPEAT tWHILE expr '\n' stmtlist tENDREPEAT '\n'  */
#line 622 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.node) = new RepeatWhileNode((yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 4077 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 189: /* loop: tREPEAT tWHILE expr error '\n' stmtlist tENDREPEAT '\n'  */
#line 624 "engines/director/lingo/lingo-gr.y"
                                                                  {
		TRIM_GARBAGE((yyval.node), new RepeatWhileNode((yyvsp[-5].node), (yyvsp[-2].nodelist))); }
#line 4084 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 190: /* loop: tREPEAT tWITH ID tEQ expr tTO expr '\n' stmtlist tENDREPEAT '\n'  */
#line 626 "engines/director/lingo/lingo-gr.y"
                                                                                       {
		(yyval.node) = new RepeatWithToNode((yyvsp[-8].s), (yyvsp[-6].node), false, (yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 4091 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 191: /* loop: tREPEAT tWITH ID tEQ expr tTO expr error '\n' stmtlist tENDREPEAT '\n'  */
#line 628 "engines/director/lingo/lingo-gr.y"
                                                                                             {
		TRIM_GARBAGE((yyval.node), new RepeatWithToNode((yyvsp[-9].s), (yyvsp[-7].node), false, (yyvsp[-5].node), (yyvsp[-2].nodelist))); }
#line 4098 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 192: /* loop: tREPEAT tWITH ID tEQ expr tDOWN tTO expr '\n' stmtlist tENDREPEAT '\n'  */
#line 630 "engines/director/lingo/lingo-gr.y"
                                                                                             {
		(yyval.node) = new RepeatWithToNode((yyvsp[-9].s), (yyvsp[-7].node), true, (yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 4105 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 193: /* loop: tREPEAT tWITH ID tEQ expr tDOWN tTO expr error '\n' stmtlist tENDREPEAT '\n'  */
#line 632 "engines/director/lingo/lingo-gr.y"
                                                                                                   {
		TRIM_GARBAGE((yyval.node), new RepeatWithToNode((yyvsp[-10].s), (yyvsp[-8].node), true, (yyvsp[-5].node), (yyvsp[-2].nodelist))); }
#line 4112 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 194: /* loop: tREPEAT tWITH ID tIN expr '\n' stmtlist tENDREPEAT '\n'  */
#line 634 "engines/director/lingo/lingo-gr.y"
                                                                  {
		(yyval.node) = new RepeatWithInNode((yyvsp[-6].s), (yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 4119 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 195: /* loop: tREPEAT tWITH ID tIN expr error '\n' stmtlist tENDREPEAT '\n'  */
#line 636 "engines/director/lingo/lingo-gr.y"
                                                                        {
		TRIM_GARBAGE((yyval.node), new RepeatWithInNode((yyvsp[-7].s), (yyvsp[-5].node), (yyvsp[-2].nodelist))); }
#line 4126 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 196: /* tell: tTELL expr tTO stmtoneliner  */
#line 640 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *stmtlist = new NodeList;
		stmtlist->push_back((yyvsp[0].node));
		(yyval.node) = new TellNode((yyvsp[-2].node), stmtlist); }
#line 4135 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 197: /* tell: tTELL expr '\n' stmtlist tENDTELL '\n'  */
#line 644 "engines/director/lingo/lingo-gr.y"
                                                        {
		(yyval.node) = new TellNode((yyvsp[-4].node), (yyvsp[-2].nodelist)); }
#line 4142 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 198: /* tell: tTELL expr error '\n' stmtlist tENDTELL '\n'  */
#line 646 "engines/director/lingo/lingo-gr.y"
                                                        {
		TRIM_GARBAGE((yyval.node), new TellNode((yyvsp[-5].node), (yyvsp[-2].nodelist))); }
#line 4149 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 199: /* when: tWHEN '\n'  */
#line 650 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new WhenNode((yyvsp[-1].w).eventName, (yyvsp[-1].w).stmt); }
#line 4155 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 200: /* stmtlist: %empty  */
#line 652 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.nodelist) = new NodeList; }
#line 4161 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 202: /* nonemptystmtlist: stmtlistline  */
#line 657 "engines/director/lingo/lingo-gr.y"
                                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 4172 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 203: /* nonemptystmtlist: nonemptystmtlist stmtlistline  */
#line 663 "engines/director/lingo/lingo-gr.y"
                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 4182 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 204: /* stmtlistline: '\n'  */
#line 670 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = nullptr; }
#line 4188 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 206: /* stmtlist_insideif: %empty  */
#line 674 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.nodelist) = new NodeList; }
#line 4194 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 208: /* nonemptystmtlist_insideif: stmtlistline_insideif  */
#line 679 "engines/director/lingo/lingo-gr.y"
                                                {
		NodeList *list = new NodeList;
		if ((yyvsp[0].node)) {
			list->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = list; }
#line 4205 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 209: /* nonemptystmtlist_insideif: nonemptystmtlist_insideif stmtlistline_insideif  */
#line 685 "engines/director/lingo/lingo-gr.y"
                                                                        {
		if ((yyvsp[0].node)) {
			(yyvsp[-1].nodelist)->push_back((yyvsp[0].node));
		}
		(yyval.nodelist) = (yyvsp[-1].nodelist); }
#line 4215 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 210: /* stmtlistline_insideif: '\n'  */
#line 692 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = nullptr; }
#line 4221 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 212: /* simpleexpr_nounarymath: tINT  */
#line 699 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new IntNode((yyvsp[0].i)); }
#line 4227 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 213: /* simpleexpr_nounarymath: tFLOAT  */
#line 700 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new FloatNode((yyvsp[0].f)); }
#line 4233 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 214: /* simpleexpr_nounarymath: tSYMBOL  */
#line 701 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new SymbolNode((yyvsp[0].s)); }
#line 4239 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 215: /* simpleexpr_nounarymath: tSTRING  */
#line 702 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new StringNode((yyvsp[0].s)); }
#line 4245 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 216: /* simpleexpr_nounarymath: tNOT simpleexpr  */
#line 703 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new UnaryOpNode(LC::c_not, (yyvsp[0].node)); }
#line 4251 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 217: /* simpleexpr_nounarymath: ID '(' ')'  */
#line 704 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new FuncNode((yyvsp[-2].s), new NodeList); }
#line 4257 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 218: /* simpleexpr_nounarymath: ID '(' nonemptyexprlist trailingcomma ')'  */
#line 705 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new FuncNode((yyvsp[-4].s), (yyvsp[-2].nodelist)); }
#line 4263 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 219: /* simpleexpr_nounarymath: ID '(' var expr_nounarymath trailingcomma ')'  */
#line 706 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `obj(method arg)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-3].node));
		args->push_back((yyvsp[-2].node));
		(yyval.node) = new FuncNode((yyvsp[-5].s), args); }
#line 4274 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 220: /* simpleexpr_nounarymath: ID '(' var expr_nounarymath ',' nonemptyexprlist trailingcomma ')'  */
#line 712 "engines/director/lingo/lingo-gr.y"
                                                                                                {
		// This matches `obj(method arg, ...)`
		(yyvsp[-2].nodelist)->insert_at(0, (yyvsp[-4].node));
		(yyvsp[-2].nodelist)->insert_at(0, (yyvsp[-5].node));
		(yyval.node) = new FuncNode((yyvsp[-7].s), (yyvsp[-2].nodelist)); }
#line 4284 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 221: /* simpleexpr_nounarymath: simpleexpr_nounarymath '.' ID  */
#line 717 "engines/director/lingo/lingo-gr.y"
                                                {
		// D5 dot syntax. `a.b` is the same thing as `the b of a`, so it builds
		// the very node that spelling builds and needs no codegen of its own:
		// visitTheOfNode() already emits c_objectproppush for D4 and up.
		// Left-recursive, so `a.b.c` nests as `(a.b).c`.
		(yyval.node) = new TheOfNode((yyvsp[0].s), (yyvsp[-2].node)); }
#line 4295 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 222: /* simpleexpr_nounarymath: '(' expr ')'  */
#line 723 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = (yyvsp[-1].node); }
#line 4301 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 228: /* var: ID  */
#line 731 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new VarNode((yyvsp[0].s)); }
#line 4307 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 233: /* chunk: tFIELD refargs  */
#line 741 "engines/director/lingo/lingo-gr.y"
                                { (yyval.node) = new FuncNode(new Common::String("field"), (yyvsp[0].nodelist)); }
#line 4313 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 234: /* chunk: tCAST simpleexpr tOF tCASTLIB simpleexpr  */
#line 742 "engines/director/lingo/lingo-gr.y"
                                                         {
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-3].node));
		args->push_back((yyvsp[0].node));
		(yyval.node) = new FuncNode(new Common::String("cast"), args);	}
#line 4323 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 235: /* chunk: tCAST refargs  */
#line 747 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new FuncNode(new Common::String("cast"), (yyvsp[0].nodelist)); }
#line 4329 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 236: /* chunk: tMEMBER simpleexpr tOF tCASTLIB simpleexpr  */
#line 748 "engines/director/lingo/lingo-gr.y"
                                                           {
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-3].node));
		args->push_back((yyvsp[0].node));
		(yyval.node) = new FuncNode(new Common::String("member"), args);	}
#line 4339 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 237: /* chunk: tMEMBER refargs  */
#line 753 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new FuncNode(new Common::String("member"), (yyvsp[0].nodelist)); }
#line 4345 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 238: /* chunk: tCASTLIB refargs  */
#line 754 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new FuncNode(new Common::String("castLib"), (yyvsp[0].nodelist)); }
#line 4351 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 239: /* chunk: tCHAR expr tOF simpleexpr  */
#line 755 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.node) = new ChunkExprNode(kChunkChar, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 4358 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 240: /* chunk: tCHAR expr tTO expr tOF simpleexpr  */
#line 757 "engines/director/lingo/lingo-gr.y"
                                                                {
		(yyval.node) = new ChunkExprNode(kChunkChar, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4365 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 241: /* chunk: tWORD expr tOF simpleexpr  */
#line 759 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.node) = new ChunkExprNode(kChunkWord, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 4372 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 242: /* chunk: tWORD expr tTO expr tOF simpleexpr  */
#line 761 "engines/director/lingo/lingo-gr.y"
                                                                {
		(yyval.node) = new ChunkExprNode(kChunkWord, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4379 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 243: /* chunk: tITEM expr tOF simpleexpr  */
#line 763 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.node) = new ChunkExprNode(kChunkItem, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 4386 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 244: /* chunk: tITEM expr tTO expr tOF simpleexpr  */
#line 765 "engines/director/lingo/lingo-gr.y"
                                                                {
		(yyval.node) = new ChunkExprNode(kChunkItem, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4393 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 245: /* chunk: tLINE expr tOF simpleexpr  */
#line 767 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyval.node) = new ChunkExprNode(kChunkLine, (yyvsp[-2].node), nullptr, (yyvsp[0].node)); }
#line 4400 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 246: /* chunk: tLINE expr tTO expr tOF simpleexpr  */
#line 769 "engines/director/lingo/lingo-gr.y"
                                                                {
		(yyval.node) = new ChunkExprNode(kChunkLine, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4407 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 247: /* chunk: tTHE tLAST chunktype inof simpleexpr  */
#line 771 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new TheLastNode((yyvsp[-2].chunktype), (yyvsp[0].node)); }
#line 4413 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 248: /* chunktype: tCHAR  */
#line 774 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.chunktype) = kChunkChar; }
#line 4419 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 249: /* chunktype: tWORD  */
#line 775 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.chunktype) = kChunkWord; }
#line 4425 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 250: /* chunktype: tITEM  */
#line 776 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.chunktype) = kChunkItem; }
#line 4431 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 251: /* chunktype: tLINE  */
#line 777 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.chunktype) = kChunkLine; }
#line 4437 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 252: /* object: tSCRIPT refargs  */
#line 780 "engines/director/lingo/lingo-gr.y"
                                { (yyval.node) = new FuncNode(new Common::String("script"), (yyvsp[0].nodelist)); }
#line 4443 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 253: /* object: tWINDOW refargs  */
#line 781 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new FuncNode(new Common::String("window"), (yyvsp[0].nodelist)); }
#line 4449 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 254: /* refargs: simpleexpr  */
#line 784 "engines/director/lingo/lingo-gr.y"
                                                                                {
		// This matches `ref arg` and `ref(arg)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[0].node));
		(yyval.nodelist) = args; }
#line 4459 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 255: /* refargs: '(' ')'  */
#line 789 "engines/director/lingo/lingo-gr.y"
                                                                                        {
		// This matches `ref()`
		(yyval.nodelist) = new NodeList; }
#line 4467 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 256: /* refargs: '(' expr ',' ')'  */
#line 792 "engines/director/lingo/lingo-gr.y"
                           {
		// This matches `ref(arg,)`
		NodeList *args = new NodeList;
		args->push_back((yyvsp[-2].node));
		(yyval.nodelist) = args; }
#line 4477 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 257: /* refargs: '(' expr ',' nonemptyexprlist trailingcomma ')'  */
#line 797 "engines/director/lingo/lingo-gr.y"
                                                                {
		// This matches `ref(arg, ...)`
		(yyvsp[-2].nodelist)->insert_at(0, (yyvsp[-4].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 4486 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 258: /* the: tTHE ID  */
#line 803 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheNode((yyvsp[0].s)); }
#line 4492 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 259: /* the: tTHE ID tOF theobj  */
#line 804 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheOfNode((yyvsp[-2].s), (yyvsp[0].node)); }
#line 4498 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 260: /* the: tTHE tNUMBER tOF theobj  */
#line 805 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheOfNode(new Common::String("number"), (yyvsp[0].node)); }
#line 4504 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 265: /* theobj: tMENUITEM simpleexpr tOF tMENU simpleexpr  */
#line 812 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new MenuItemNode((yyvsp[-3].node), (yyvsp[0].node)); }
#line 4510 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 266: /* theobj: tSOUND simpleexpr  */
#line 813 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SoundNode((yyvsp[0].node)); }
#line 4516 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 267: /* theobj: tSPRITE simpleexpr  */
#line 814 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SpriteNode((yyvsp[0].node)); }
#line 4522 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 268: /* menu: tMENU simpleexpr  */
#line 817 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new MenuNode((yyvsp[0].node)); }
#line 4528 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 269: /* thedatetime: tTHE tABBREVIATED tDATE  */
#line 819 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheDate); }
#line 4534 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 270: /* thedatetime: tTHE tABBREVIATED tTIME  */
#line 820 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheTime); }
#line 4540 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 271: /* thedatetime: tTHE tABBREV tDATE  */
#line 821 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheDate); }
#line 4546 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 272: /* thedatetime: tTHE tABBREV tTIME  */
#line 822 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheTime); }
#line 4552 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 273: /* thedatetime: tTHE tABBR tDATE  */
#line 823 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheDate); }
#line 4558 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 274: /* thedatetime: tTHE tABBR tTIME  */
#line 824 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheAbbr, kTheTime); }
#line 4564 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 275: /* thedatetime: tTHE tLONG tDATE  */
#line 825 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheLong, kTheDate); }
#line 4570 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 276: /* thedatetime: tTHE tLONG tTIME  */
#line 826 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheLong, kTheTime); }
#line 4576 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 277: /* thedatetime: tTHE tSHORT tDATE  */
#line 827 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheShort, kTheDate); }
#line 4582 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 278: /* thedatetime: tTHE tSHORT tTIME  */
#line 828 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheDateTimeNode(kTheShort, kTheTime); }
#line 4588 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 279: /* thenumberof: tTHE tNUMBER tOF tCHARS inof simpleexpr  */
#line 832 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfChars, (yyvsp[0].node)); }
#line 4594 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 280: /* thenumberof: tTHE tNUMBER tOF tWORDS inof simpleexpr  */
#line 833 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfWords, (yyvsp[0].node)); }
#line 4600 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 281: /* thenumberof: tTHE tNUMBER tOF tITEMS inof simpleexpr  */
#line 834 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfItems, (yyvsp[0].node)); }
#line 4606 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 282: /* thenumberof: tTHE tNUMBER tOF tLINES inof simpleexpr  */
#line 835 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfLines, (yyvsp[0].node)); }
#line 4612 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 283: /* thenumberof: tTHE tNUMBER tOF tMENUITEMS inof menu  */
#line 836 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfMenuItems, (yyvsp[0].node)); }
#line 4618 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 284: /* thenumberof: tTHE tNUMBER tOF tMENUS  */
#line 837 "engines/director/lingo/lingo-gr.y"
                                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfMenus, nullptr); }
#line 4624 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 285: /* thenumberof: tTHE tNUMBER tOF tXTRAS  */
#line 838 "engines/director/lingo/lingo-gr.y"
                                                                        { (yyval.node) = new TheNumberOfNode(kNumberOfXtras, nullptr); }
#line 4630 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 286: /* thenumberof: tTHE tNUMBER tOF tCASTLIBS  */
#line 839 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new TheNumberOfNode(kNumberOfCastlibs, nullptr); }
#line 4636 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 289: /* writablethe: tTHE ID  */
#line 844 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new TheNode((yyvsp[0].s)); }
#line 4642 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 290: /* writablethe: tTHE ID tOF writabletheobj  */
#line 845 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new TheOfNode((yyvsp[-2].s), (yyvsp[0].node)); }
#line 4648 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 291: /* writablethe: writabletheobj '.' ID  */
#line 846 "engines/director/lingo/lingo-gr.y"
                                                {
		// The assignment side of D5 dot syntax: `set a.b to c`. Same node as
		// the read, which is what visitSetNode() already unpacks to emit
		// c_objectpropassign.
		(yyval.node) = new TheOfNode((yyvsp[0].s), (yyvsp[-2].node)); }
#line 4658 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 293: /* writabletheobj: tMENU expr_noeq  */
#line 854 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new MenuNode((yyvsp[0].node)); }
#line 4664 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 294: /* writabletheobj: tMENUITEM expr_noeq tOF tMENU expr_noeq  */
#line 855 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new MenuItemNode((yyvsp[-3].node), (yyvsp[0].node)); }
#line 4670 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 295: /* writabletheobj: tSOUND expr_noeq  */
#line 856 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SoundNode((yyvsp[0].node)); }
#line 4676 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 296: /* writabletheobj: tSPRITE expr_noeq  */
#line 857 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new SpriteNode((yyvsp[0].node)); }
#line 4682 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 297: /* list: '[' exprlist ']'  */
#line 860 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new ListNode((yyvsp[-1].nodelist)); }
#line 4688 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 298: /* list: '[' ':' ']'  */
#line 861 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropListNode(new NodeList); }
#line 4694 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 299: /* list: '[' proplist ']'  */
#line 862 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropListNode((yyvsp[-1].nodelist)); }
#line 4700 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 300: /* proplist: proppair  */
#line 868 "engines/director/lingo/lingo-gr.y"
                                                        {
		NodeList *list = new NodeList;
		list->push_back((yyvsp[0].node));
		(yyval.nodelist) = list; }
#line 4709 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 301: /* proplist: proplist ',' proppair  */
#line 872 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 4717 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 302: /* proplist: proplist ',' expr  */
#line 875 "engines/director/lingo/lingo-gr.y"
                                        {
		(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 4725 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 303: /* proppair: tSYMBOL ':' expr  */
#line 880 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new PropPairNode(new SymbolNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 4731 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 304: /* proppair: ID ':' expr  */
#line 881 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropPairNode(new SymbolNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 4737 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 305: /* proppair: tSTRING ':' expr  */
#line 882 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new PropPairNode(new StringNode((yyvsp[-2].s)), (yyvsp[0].node)); }
#line 4743 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 306: /* proppair: tINT ':' expr  */
#line 883 "engines/director/lingo/lingo-gr.y"
                                    { (yyval.node) = new PropPairNode(new IntNode((yyvsp[-2].i)), (yyvsp[0].node)); }
#line 4749 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 307: /* proppair: tFLOAT ':' expr  */
#line 884 "engines/director/lingo/lingo-gr.y"
                                    { (yyval.node) = new PropPairNode(new FloatNode((yyvsp[-2].f)), (yyvsp[0].node)); }
#line 4755 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 308: /* unarymath: '+' simpleexpr  */
#line 887 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = (yyvsp[0].node); }
#line 4761 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 309: /* unarymath: '-' simpleexpr  */
#line 888 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.node) = new UnaryOpNode(LC::c_negate, (yyvsp[0].node)); }
#line 4767 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 314: /* expr: expr '+' expr  */
#line 898 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_add, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4773 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 315: /* expr: expr '-' expr  */
#line 899 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_sub, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4779 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 316: /* expr: expr '*' expr  */
#line 900 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_mul, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4785 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 317: /* expr: expr '/' expr  */
#line 901 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_div, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4791 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 318: /* expr: expr tMOD expr  */
#line 902 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_mod, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4797 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 319: /* expr: expr '>' expr  */
#line 903 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_gt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4803 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 320: /* expr: expr '<' expr  */
#line 904 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_lt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4809 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 321: /* expr: expr tEQ expr  */
#line 905 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_eq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4815 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 322: /* expr: expr tNEQ expr  */
#line 906 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_neq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4821 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 323: /* expr: expr tGE expr  */
#line 907 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_ge, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4827 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 324: /* expr: expr tLE expr  */
#line 908 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_le, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4833 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 325: /* expr: expr tAND expr  */
#line 909 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_and, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4839 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 326: /* expr: expr tOR expr  */
#line 910 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_or, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4845 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 327: /* expr: expr '&' expr  */
#line 911 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_ampersand, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4851 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 328: /* expr: expr tCONCAT expr  */
#line 912 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_concat, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4857 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 329: /* expr: expr tCONTAINS expr  */
#line 913 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_contains, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4863 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 330: /* expr: expr tSTARTS expr  */
#line 914 "engines/director/lingo/lingo-gr.y"
                                        { (yyval.node) = new BinaryOpNode(LC::c_starts, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4869 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 333: /* expr_nounarymath: expr_nounarymath '+' expr  */
#line 923 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_add, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4875 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 334: /* expr_nounarymath: expr_nounarymath '-' expr  */
#line 924 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_sub, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4881 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 335: /* expr_nounarymath: expr_nounarymath '*' expr  */
#line 925 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mul, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4887 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 336: /* expr_nounarymath: expr_nounarymath '/' expr  */
#line 926 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_div, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4893 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 337: /* expr_nounarymath: expr_nounarymath tMOD expr  */
#line 927 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mod, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4899 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 338: /* expr_nounarymath: expr_nounarymath '>' expr  */
#line 928 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_gt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4905 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 339: /* expr_nounarymath: expr_nounarymath '<' expr  */
#line 929 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_lt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4911 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 340: /* expr_nounarymath: expr_nounarymath tEQ expr  */
#line 930 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_eq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4917 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 341: /* expr_nounarymath: expr_nounarymath tNEQ expr  */
#line 931 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_neq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4923 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 342: /* expr_nounarymath: expr_nounarymath tGE expr  */
#line 932 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ge, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4929 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 343: /* expr_nounarymath: expr_nounarymath tLE expr  */
#line 933 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_le, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4935 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 344: /* expr_nounarymath: expr_nounarymath tAND expr  */
#line 934 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_and, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4941 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 345: /* expr_nounarymath: expr_nounarymath tOR expr  */
#line 935 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_or, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4947 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 346: /* expr_nounarymath: expr_nounarymath '&' expr  */
#line 936 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ampersand, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4953 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 347: /* expr_nounarymath: expr_nounarymath tCONCAT expr  */
#line 937 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_concat, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4959 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 348: /* expr_nounarymath: expr_nounarymath tCONTAINS expr  */
#line 938 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_contains, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4965 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 349: /* expr_nounarymath: expr_nounarymath tSTARTS expr  */
#line 939 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_starts, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4971 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 352: /* expr_noeq: expr_noeq '+' expr_noeq  */
#line 944 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_add, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4977 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 353: /* expr_noeq: expr_noeq '-' expr_noeq  */
#line 945 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_sub, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4983 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 354: /* expr_noeq: expr_noeq '*' expr_noeq  */
#line 946 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mul, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4989 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 355: /* expr_noeq: expr_noeq '/' expr_noeq  */
#line 947 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_div, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 4995 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 356: /* expr_noeq: expr_noeq tMOD expr_noeq  */
#line 948 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_mod, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 5001 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 357: /* expr_noeq: expr_noeq '>' expr_noeq  */
#line 949 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_gt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 5007 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 358: /* expr_noeq: expr_noeq '<' expr_noeq  */
#line 950 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_lt, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 5013 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 359: /* expr_noeq: expr_noeq tNEQ expr_noeq  */
#line 951 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_neq, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 5019 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 360: /* expr_noeq: expr_noeq tGE expr_noeq  */
#line 952 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ge, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 5025 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 361: /* expr_noeq: expr_noeq tLE expr_noeq  */
#line 953 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_le, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 5031 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 362: /* expr_noeq: expr_noeq tAND expr_noeq  */
#line 954 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_and, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 5037 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 363: /* expr_noeq: expr_noeq tOR expr_noeq  */
#line 955 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_or, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 5043 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 364: /* expr_noeq: expr_noeq '&' expr_noeq  */
#line 956 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_ampersand, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 5049 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 365: /* expr_noeq: expr_noeq tCONCAT expr_noeq  */
#line 957 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_concat, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 5055 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 366: /* expr_noeq: expr_noeq tCONTAINS expr_noeq  */
#line 958 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new BinaryOpNode(LC::c_contains, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 5061 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 367: /* expr_noeq: expr_noeq tSTARTS expr_noeq  */
#line 959 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new BinaryOpNode(LC::c_starts, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 5067 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 368: /* sprite: tSPRITE expr tINTERSECTS simpleexpr  */
#line 962 "engines/director/lingo/lingo-gr.y"
                                                { (yyval.node) = new IntersectsNode((yyvsp[-2].node), (yyvsp[0].node)); }
#line 5073 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 369: /* sprite: tSPRITE expr tWITHIN simpleexpr  */
#line 963 "engines/director/lingo/lingo-gr.y"
                                                        { (yyval.node) = new WithinNode((yyvsp[-2].node), (yyvsp[0].node)); }
#line 5079 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 370: /* exprlist: %empty  */
#line 966 "engines/director/lingo/lingo-gr.y"
                                                                { (yyval.nodelist) = new NodeList; }
#line 5085 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 372: /* nonemptyexprlist: expr  */
#line 970 "engines/director/lingo/lingo-gr.y"
                                                        {
		NodeList *list = new NodeList;
		list->push_back((yyvsp[0].node));
		(yyval.nodelist) = list; }
#line 5094 "engines/director/lingo/lingo-gr.cpp"
    break;

  case 373: /* nonemptyexprlist: nonemptyexprlist ',' expr  */
#line 974 "engines/director/lingo/lingo-gr.y"
                                                {
		(yyvsp[-2].nodelist)->push_back((yyvsp[0].node));
		(yyval.nodelist) = (yyvsp[-2].nodelist); }
#line 5102 "engines/director/lingo/lingo-gr.cpp"
    break;


#line 5106 "engines/director/lingo/lingo-gr.cpp"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      {
        yypcontext_t yyctx
          = {yyssp, yytoken};
        if (yyreport_syntax_error (&yyctx) == 2)
          YYNOMEM;
      }
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 979 "engines/director/lingo/lingo-gr.y"


int yyreport_syntax_error(const yypcontext_t *ctx) {
	int res = 0;

	Common::String msg = "syntax error, ";

	// Report the unexpected token.
	yysymbol_kind_t lookahead = yypcontext_token(ctx);
	if (lookahead != YYSYMBOL_YYEMPTY)
		msg += Common::String::format("unexpected %s", yysymbol_name(lookahead));

	// Report the tokens expected at this point.
	enum { TOKENMAX = 10 };
	yysymbol_kind_t expected[TOKENMAX];

	int n = yypcontext_expected_tokens(ctx, expected, TOKENMAX);
	if (n < 0)
		// Forward errors to yyparse.
		res = n;
	else
		for (int i = 0; i < n; ++i)
			msg += Common::String::format("%s %s", i == 0 ? ": expected" : " or", yysymbol_name(expected[i]));

	yyerror(msg.c_str());

	return res;
}
