/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qbs.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QMLJSLEXER_P_H
#define QMLJSLEXER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qmljsglobal_p.h"
#include "qmljsgrammar_p.h"
#include <QtCore/QString>

namespace QbsQmlJS {

class Engine;

class QML_PARSER_EXPORT Directives {
public:
    virtual ~Directives() {}

    virtual void pragmaLibrary()
    {
    }

    virtual void importFile(const QString &jsfile, const QString &module)
    {
        Q_UNUSED(jsfile);
        Q_UNUSED(module);
    }

    virtual void importModule(const QString &uri, const QString &version, const QString &module)
    {
        Q_UNUSED(uri);
        Q_UNUSED(version);
        Q_UNUSED(module);
    }
};

class QML_PARSER_EXPORT Lexer: public QmlJSGrammar
{
public:
    enum {
        T_ABSTRACT = T_RESERVED_WORD,
        T_BOOLEAN = T_RESERVED_WORD,
        T_BYTE = T_RESERVED_WORD,
        T_CHAR = T_RESERVED_WORD,
        T_CLASS = T_RESERVED_WORD,
        T_DOUBLE = T_RESERVED_WORD,
        T_ENUM = T_RESERVED_WORD,
        T_EXPORT = T_RESERVED_WORD,
        T_EXTENDS = T_RESERVED_WORD,
        T_FINAL = T_RESERVED_WORD,
        T_FLOAT = T_RESERVED_WORD,
        T_GOTO = T_RESERVED_WORD,
        T_IMPLEMENTS = T_RESERVED_WORD,
        T_INT = T_RESERVED_WORD,
        T_INTERFACE = T_RESERVED_WORD,
        T_LET = T_RESERVED_WORD,
        T_LONG = T_RESERVED_WORD,
        T_NATIVE = T_RESERVED_WORD,
        T_PACKAGE = T_RESERVED_WORD,
        T_PRIVATE = T_RESERVED_WORD,
        T_PROTECTED = T_RESERVED_WORD,
        T_SHORT = T_RESERVED_WORD,
        T_STATIC = T_RESERVED_WORD,
        T_SUPER = T_RESERVED_WORD,
        T_SYNCHRONIZED = T_RESERVED_WORD,
        T_THROWS = T_RESERVED_WORD,
        T_TRANSIENT = T_RESERVED_WORD,
        T_VOLATILE = T_RESERVED_WORD,
        T_YIELD = T_RESERVED_WORD
    };

    enum Error {
        NoError,
        IllegalCharacter,
        UnclosedStringLiteral,
        IllegalEscapeSequence,
        IllegalUnicodeEscapeSequence,
        UnclosedComment,
        IllegalExponentIndicator,
        IllegalIdentifier
    };

    enum RegExpBodyPrefix {
        NoPrefix,
        EqualPrefix
    };

    enum RegExpFlag {
        RegExp_Global     = 0x01,
        RegExp_IgnoreCase = 0x02,
        RegExp_Multiline  = 0x04
    };

public:
    Lexer(Engine *engine);

    bool qmlMode() const;

    QString code() const;
    void setCode(const QString &code, int lineno, bool qmlMode = true);

    int lex();

    bool scanRegExp(RegExpBodyPrefix prefix = NoPrefix);
    bool scanDirectives(Directives *directives);

    int regExpFlags() const { return _patternFlags; }
    QString regExpPattern() const { return _tokenText; }

    int tokenKind() const { return _tokenKind; }
    int tokenOffset() const { return _tokenStartPtr - _code.unicode(); }
    int tokenLength() const { return _tokenLength; }

    int tokenStartLine() const { return _tokenLine; }
    int tokenStartColumn() const { return _tokenStartPtr - _tokenLinePtr + 1; }

    int tokenEndLine() const;
    int tokenEndColumn() const;

    inline QStringRef tokenSpell() const { return _tokenSpell; }
    double tokenValue() const { return _tokenValue; }
    QString tokenText() const;

    Error errorCode() const;
    QString errorMessage() const;

    bool prevTerminator() const;
    bool followsClosingBrace() const;
    bool canInsertAutomaticSemicolon(int token) const;

    enum ParenthesesState {
        IgnoreParentheses,
        CountParentheses,
        BalancedParentheses
    };

protected:
    int classify(const QChar *s, int n, bool qmlMode);

private:
    inline void scanChar();
    int scanToken();
    int scanNumber(QChar ch);

    bool isLineTerminator() const;
    static bool isIdentLetter(QChar c);
    static bool isDecimalDigit(ushort c);
    static bool isHexDigit(QChar c);
    static bool isOctalDigit(ushort c);
    static bool isUnicodeEscapeSequence(const QChar *chars);

    void syncProhibitAutomaticSemicolon();
    QChar decodeUnicodeEscapeCharacter(bool *ok);

private:
    Engine *_engine;

    QString _code;
    QString _tokenText;
    QString _errorMessage;
    QStringRef _tokenSpell;

    const QChar *_codePtr;
    const QChar *_lastLinePtr;
    const QChar *_tokenLinePtr;
    const QChar *_tokenStartPtr;

    QChar _char;
    Error _errorCode;

    int _currentLineNumber;
    double _tokenValue;

    // parentheses state
    ParenthesesState _parenthesesState;
    int _parenthesesCount;

    int _stackToken;

    int _patternFlags;
    int _tokenKind;
    int _tokenLength;
    int _tokenLine;

    bool _validTokenText;
    bool _prohibitAutomaticSemicolon;
    bool _restrictedKeyword;
    bool _terminator;
    bool _followsClosingBrace;
    bool _delimited;
    bool _qmlMode;
};

} // namespace QbsQmlJS

#endif // LEXER_H
