/*
 *   config_lexer.l -- April 27 2016 18:42:54 EST
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
#include "conf.h"
#include "config_parser.h"
#include "foxbot.h"

#undef YY_INPUT
#define YY_INPUT(buf, result, max_size) \
    result = conf_yy_input(buf, max_size)

static void ccomment(void);

static int
conf_yy_input(char *lbuf, unsigned int max_size) {
    return !fgets(lbuf, max_size, conf_parser_ctx.conf_file) ? 0 : strlen(lbuf);
}

%}

WS        [[:space:]]*
COMMENT   ("//"|"#").*
STRING   \"[^\"\n]*[\"\n]
%%

"/*"            { ccomment(); }
{WS}            ;
{COMMENT}       ;
{STRING}        {   if (yytext[yyleng - 2] == '\\')
                    {
                        yyless(yyleng - 1);  /* Return last quote */
                        yymore();  /* Append next string */
                    } else {
                        yylval.string = yytext + 1;
                        if (yylval.string[yyleng - 2] == '"')
                        {
                            unsigned int i = 0, j = 0;

                            /* Remove close quote */
                            yylval.string[yyleng - 2] = '\0';
                            for (; yylval.string[i] != '\0'; ++i, ++j) {
                                if (yylval.string[i] == '\\') ++i;
                                yylval.string[j] = yylval.string[i];
                            }

                            yylval.string[j] = '\0';
                            return STRING;
                        }
                    }
                }

bot      { return BOT;       }
channel  { return CHANNEL;   }
host     { return HOST;      }
ident    { return IDENT;     }
nick     { return NICK;      }
port     { return PORT;      }
realname { return REALNAME;  }

.        { return yytext[0]; }
<<EOF>>  { yyterminate();    }
%%

static void ccomment(void) {
    int c;
    do {
        c = input();
        while (c == '*')
        if ((c = input()) == '/')
            return;
    } while (c != EOF);
    yy_fatal_error("EOF in comment");
}
