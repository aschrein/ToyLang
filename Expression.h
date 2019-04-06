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
  eMINUS
};

struct SExpression {
  EOperationType type;
  char *name;
  int value;
  SExpression *left;
  SExpression *right;
  SExpression *cond;
};

SExpression *createNumber(int value);
SExpression *createOperation(EOperationType type, SExpression *left,
                             SExpression *right);
void deleteExpression(SExpression *b);
SExpression *createDef(char const *name, SExpression *exp);
SExpression *createDefun(char const *name, SExpression *exp);
SExpression *createCall(char const *name, SExpression *exp);
SExpression *createRef(char const *name);
SExpression *createColon(SExpression *lexp, SExpression *rexp);
SExpression *createIF(SExpression *, SExpression *, SExpression *);
#endif /* __EXPRESSION_H__ */
