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
  eADD
};

struct SExpression {
  EOperationType type;
  char *name;
  int value;
  SExpression *left;
  SExpression *right;
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
#endif /* __EXPRESSION_H__ */
