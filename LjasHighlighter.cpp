/*
* Copyright 2019 Rochus Keller <mailto:me@rochus-keller.ch>
*
* This file is part of the LjAsmEditor application.
*
* The following is the license that applies to this copy of the
* library. For a license to use the library under conditions
* other than those described here, please email to me@rochus-keller.ch.
*
* GNU General Public License Usage
* This file may be used under the terms of the GNU General Public
* License (GPL) versions 2.0 or 3.0 as published by the Free Software
* Foundation and appearing in the file LICENSE.GPL included in
* the packaging of this file. Please review the following information
* to ensure GNU General Public Licensing requirements will be met:
* http://www.fsf.org/licensing/licenses/info/GPLv2.html and
* http://www.gnu.org/copyleft/gpl.html.
*/

#include "LjasHighlighter.h"
#include "LjasLexer.h"
#include <QBuffer>
using namespace Ljas;

Highlighter::Highlighter(QTextDocument* parent) :
    QSyntaxHighlighter(parent),d_enableExt(false)
{
    for( int i = 0; i < C_Max; i++ )
    {
        d_format[i].setFontWeight(QFont::Normal);
        d_format[i].setForeground(Qt::black);
        d_format[i].setBackground(Qt::transparent);
    }
    d_format[C_Num].setForeground(QColor(0, 153, 153));
    d_format[C_Str].setForeground(QColor(208, 16, 64));
    d_format[C_Cmt].setForeground(QColor(153, 153, 136));
    d_format[C_Kw].setForeground(QColor(68, 85, 136));
    d_format[C_Kw].setFontWeight(QFont::Bold);
    d_format[C_Op].setForeground(QColor(153, 0, 0));
    d_format[C_Op].setFontWeight(QFont::Bold);
    d_format[C_Type].setForeground(QColor(153, 0, 115));
    d_format[C_Type].setFontWeight(QFont::Bold);
    d_format[C_Pp].setForeground(QColor(0, 134, 179));
    d_format[C_Pp].setFontWeight(QFont::Bold);

    d_format[C_Section].setForeground(QColor(0, 128, 0));
    d_format[C_Section].setBackground(QColor(230, 255, 230));

}

void Highlighter::setEnableExt(bool b)
{
    const bool old = d_enableExt;
    d_enableExt = b;
}

QTextCharFormat Highlighter::formatForCategory(int c) const
{
    return d_format[c];
}

void Highlighter::highlightBlock(const QString& text)
{
    const int previousBlockState_ = previousBlockState();
    int lexerState = 0, initialBraceDepth = 0;
    if (previousBlockState_ != -1) {
        lexerState = previousBlockState_ & 0xff;
        initialBraceDepth = previousBlockState_ >> 8;
    }

    int braceDepth = initialBraceDepth;


    int start = 0;
    if( lexerState > 0 )
    {
        // wir sind in einem Multi Line Comment
        // suche das Ende
        QTextCharFormat f = formatForCategory(C_Cmt);
        f.setProperty( TokenProp, int(Tok_Comment) );
        int pos = text.indexOf("]--");
        if( pos == -1 )
        {
            // the whole block ist part of the comment
            setFormat( start, text.size(), f );
            setCurrentBlockState( (braceDepth << 8) | lexerState);
            return;
        }else
        {
            // End of Comment found
            pos += 3;
            setFormat( start, pos , f );
            lexerState = 0;
            braceDepth--;
            start = pos;
        }
    }


    Lexer lex;
    lex.setIgnoreComments(false);
    lex.setPackComments(false);
    //lex.setEnableExt(d_enableExt);

    QList<Token> tokens =  lex.tokens(text.mid(start));
    for( int i = 0; i < tokens.size(); ++i )
    {
        Token &t = tokens[i];
        t.d_colNr += start;

        QTextCharFormat f;
        if( t.d_type == Tok_Comment )
            f = formatForCategory(C_Cmt); // one line comment
        else if( t.d_type == Tok_2MinusLbrack )
        {
            braceDepth++;
            f = formatForCategory(C_Cmt);
            lexerState = 1;
        }else if( t.d_type == Tok_Rbrack2Minus )
        {
            braceDepth--;
            f = formatForCategory(C_Cmt);
            lexerState = 0;
        }else if( t.d_type == Tok_string )
            f = formatForCategory(C_Str);
        else if( t.d_type == Tok_real || t.d_type == Tok_posint || t.d_type == Tok_negint )
            f = formatForCategory(C_Num);
        else if( tokenTypeIsLiteral(t.d_type) )
        {
            f = formatForCategory(C_Op);
        }else if( tokenTypeIsKeyword(t.d_type) )
        {
            f = formatForCategory(C_Kw);
        }else if( t.d_type == Tok_ident )
        {
            f = formatForCategory(C_Ident);
        }

        if( f.isValid() )
        {
            setFormat( t.d_colNr-1, t.d_len, f );
        }
    }

    setCurrentBlockState((braceDepth << 8) | lexerState );
}



LogPainter::LogPainter(QTextDocument* parent):QSyntaxHighlighter(parent)
{

}

void LogPainter::highlightBlock(const QString& text)
{
    QColor c = Qt::black;
    if( text.startsWith("WRN:") )
        c = Qt::blue;
    else if( text.startsWith("ERR:") )
        c = Qt::red;

    setFormat( 0, text.size(), c );
}
