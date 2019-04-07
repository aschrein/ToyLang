#ifndef __EXPRESSION_H__
#define __EXPRESSION_H__

enum EOperationType {
  eVALUE,
  eMULTIPLY,
  eCOLON,
  eSEM,
  eDEF,
  eDEFUN,
  eDIV,
  eREF,
  eCALL,
  eIF,
  eADD,
  eLESS,
  eEQEQ,
  eCOMMA,
  eMINUS
};

struct Arglist {
  char *name;
  Arglist *next;
};

struct SExpression {
  EOperationType type;
  char *name;
  int value;
  SExpression *left;
  SExpression *right;
  SExpression *cond;
  Arglist *arglist;
};

Arglist *appendArglist(char const *name, Arglist *arglist);

SExpression *createNumber(int value);
SExpression *createOperation(EOperationType type, SExpression *left,
                             SExpression *right);
void deleteExpression(SExpression *b);
SExpression *createDef(char const *name, SExpression *exp);
SExpression *createDefun(char const *name, Arglist *arglist, SExpression *exp);
SExpression *createCall(char const *name, SExpression *exp);
SExpression *createRef(char const *name);
SExpression *createColon(SExpression *lexp, SExpression *rexp);
SExpression *createIF(SExpression *, SExpression *, SExpression *);
#endif /* __EXPRESSION_H__ */
