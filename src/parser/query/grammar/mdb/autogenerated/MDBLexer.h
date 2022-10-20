
// Generated from MDBLexer.g4 by ANTLR 4.9.3

#pragma once


#include "antlr4-runtime.h"




class  MDBLexer : public antlr4::Lexer {
public:
  enum {
    K_ANY = 1, K_AND = 2, K_AVG = 3, K_ALL = 4, K_ASC = 5, K_BY = 6, K_BOOL = 7, 
    K_COUNT = 8, K_DESCRIBE = 9, K_DESC = 10, K_DISTINCT = 11, K_EDGE = 12, 
    K_INTEGER = 13, K_INSERT = 14, K_IS = 15, K_FALSE = 16, K_FLOAT = 17, 
    K_GROUP = 18, K_LABEL = 19, K_LIMIT = 20, K_MAX = 21, K_MATCH = 22, 
    K_MIN = 23, K_OPTIONAL = 24, K_ORDER = 25, K_OR = 26, K_PROPERTY = 27, 
    K_NOT = 28, K_NULL = 29, K_SET = 30, K_SUM = 31, K_STRING = 32, K_RETURN = 33, 
    K_TRUE = 34, K_WHERE = 35, TRUE_PROP = 36, FALSE_PROP = 37, ANON_ID = 38, 
    EDGE_ID = 39, KEY = 40, TYPE = 41, TYPE_VAR = 42, VARIABLE = 43, STRING = 44, 
    UNSIGNED_INTEGER = 45, UNSIGNED_FLOAT = 46, NAME = 47, LEQ = 48, GEQ = 49, 
    EQ = 50, NEQ = 51, LT = 52, GT = 53, SINGLE_EQ = 54, PATH_SEQUENCE = 55, 
    PATH_ALTERNATIVE = 56, PATH_NEGATION = 57, STAR = 58, PERCENT = 59, 
    QUESTION_MARK = 60, PLUS = 61, MINUS = 62, L_PAR = 63, R_PAR = 64, LCURLY_BRACKET = 65, 
    RCURLY_BRACKET = 66, LSQUARE_BRACKET = 67, RSQUARE_BRACKET = 68, COMMA = 69, 
    COLON = 70, WHITE_SPACE = 71, SINGLE_LINE_COMMENT = 72, UNRECOGNIZED = 73
  };

  enum {
    WS_CHANNEL = 2
  };

  explicit MDBLexer(antlr4::CharStream *input);
  ~MDBLexer();

  virtual std::string getGrammarFileName() const override;
  virtual const std::vector<std::string>& getRuleNames() const override;

  virtual const std::vector<std::string>& getChannelNames() const override;
  virtual const std::vector<std::string>& getModeNames() const override;
  virtual const std::vector<std::string>& getTokenNames() const override; // deprecated, use vocabulary instead
  virtual antlr4::dfa::Vocabulary& getVocabulary() const override;

  virtual const std::vector<uint16_t> getSerializedATN() const override;
  virtual const antlr4::atn::ATN& getATN() const override;

private:
  static std::vector<antlr4::dfa::DFA> _decisionToDFA;
  static antlr4::atn::PredictionContextCache _sharedContextCache;
  static std::vector<std::string> _ruleNames;
  static std::vector<std::string> _tokenNames;
  static std::vector<std::string> _channelNames;
  static std::vector<std::string> _modeNames;

  static std::vector<std::string> _literalNames;
  static std::vector<std::string> _symbolicNames;
  static antlr4::dfa::Vocabulary _vocabulary;
  static antlr4::atn::ATN _atn;
  static std::vector<uint16_t> _serializedATN;


  // Individual action functions triggered by action() above.

  // Individual semantic predicate functions triggered by sempred() above.

  struct Initializer {
    Initializer();
  };
  static Initializer _init;
};

