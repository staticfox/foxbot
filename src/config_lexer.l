/*
 *   config_lexer.l -- April 27 2016 18:42:54 EDT
 *
 *   This file is part of the foxbot IRC bot
 *   Copyright (C) 2016 Matt Ullman (staticfox at staticfox dot net)
 *
 *   This program is FREE software. You can redistribute it and/or
 *   modify it under the terms of the GNU General Public License
 *   as published by the Free Software Foundation; either version 2
 *   of the License, or (at your option) any later version.
 *
 *   This program is distributed in the HOPE that it will be USEFUL,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *   See the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, write to the Free Software Foundation,
 *   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

%option case-insensitive
%option noyywrap
%option nounput
%option never-interactive

%{

#include "config.h"

#include <foxbot/conf.h>
#include <foxbot/foxbot.h>

#include "config_parser.h"

#undef YY_INPUT
#define YY_INPUT(buf, result, max_size) \
    result = conf_yy_input(buf, max_size)

static void ccomment(void);

static int
conf_yy_input(char *lbuf, unsigned int max_size)
{
    return !fgets(lbuf, max_size, conf_parser_ctx.conf_file) ? 0 : strlen(lbuf);
}

%}

T_WS        [[:space:]]*
DIGIT       [[:digit:]]+
T_COMMENT   ("//"|"#").*
T_STRING    \"[^\"\n]*[\"\n]
%%

"/*"            { ccomment(); }
{T_WS}          ;
{T_COMMENT}     ;
{DIGIT}         { yylval.number = atoi(yytext); return T_NUMBER; }
{T_STRING}      {   if (yytext[yyleng - 2] == '\\')
                    {
                        yyless(yyleng - 1);  /* Return last quote */
                        yymore();  /* Append next string */
                    } else {
                        yylval.string = yytext + 1;
                        if (yylval.string[yyleng - 2] == '"')
                        {
                            size_t i = 0, j = 0;

                            /* Remove close quote */
                            yylval.string[yyleng - 2] = '\0';
                            for (; yylval.string[i]; ++i, ++j) {
                                if (yylval.string[i] == '\\') ++i;
                                yylval.string[j] = yylval.string[i];
                            }

                            yylval.string[j] = '\0';
                            return T_STRING;
                        }
                    }
                }

access        { return T_ACCESS;        }
admin         { return T_ADMIN;         }
bot           { return T_BOT;           }
host          { return T_HOST;          }
ident         { return T_IDENT;         }
nick          { return T_NICK;          }
nickserv      { return T_NICKSERV;      }
password      { return T_PASSWORD;      }
port          { return T_PORT;          }
realname      { return T_REALNAME;      }
plugin        { return T_PLUGIN;        }

channel       { return T_CHANNEL;       }
name          { return T_NAME;          }
key           { return T_KEY;           }

debug         { return T_DEBUG;         }

yes           { yylval.number = 1; return T_BOOL; }
no            { yylval.number = 0; return T_BOOL; }

.             { return yytext[0];       }
<<EOF>>       { yyterminate();          }
%%

static void
ccomment(void)
{
    int c;
    do {
        c = input();
        while (c == '*')
        if ((c = input()) == '/')
            return;
    } while (c != EOF);
    yy_fatal_error("EOF in comment");
}
