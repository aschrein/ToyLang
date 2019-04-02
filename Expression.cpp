#include "Expression.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static SExpression *allocateExpression() {
  SExpression *b = (SExpression *)malloc(sizeof(SExpression));

  if (b == NULL)
    return NULL;

  b->type = eVALUE;
  b->value = 0;
  b->name = NULL;
  b->left = NULL;
  b->right = NULL;

  return b;
}

SExpression *createNumber(int value) {
  SExpression *b = allocateExpression();

  if (b == NULL)
    return NULL;

  b->type = eVALUE;
  b->value = value;
  return b;
}

SExpression *createRef(char const *name) {
  SExpression *out = allocateExpression();

  if (out == NULL)
    return NULL;
  out->type = eREF;
  out->name = (char *)malloc(strlen(name) + 1);
  strncpy(out->name, name, strlen(name) + 1);
  return out;
}

SExpression *createDef(char const *name, SExpression *exp) {
  SExpression *out = allocateExpression();

  if (out == NULL)
    return NULL;
  out->type = eDEF;
  out->name = (char *)malloc(strlen(name) + 1);
  strncpy(out->name, name, strlen(name) + 1);
  out->left = exp;
  return out;
}

SExpression *createDefun(char const *name, SExpression *exp) {
  SExpression *out = allocateExpression();

  if (out == NULL)
    return NULL;
  out->type = eDEFUN;
  out->name = (char *)malloc(strlen(name) + 1);
  strncpy(out->name, name, strlen(name) + 1);
  out->left = exp;
  return out;
}

SExpression *createCall(char const *name, SExpression *exp) {
  SExpression *out = allocateExpression();

  if (out == NULL)
    return NULL;
  out->type = eCALL;
  out->name = (char *)malloc(strlen(name) + 1);
  strncpy(out->name, name, strlen(name) + 1);
  out->left = exp;
  return out;
}

SExpression *createColon(SExpression *lexp, SExpression *rexp) {
  SExpression *out = allocateExpression();

  if (out == NULL)
    return NULL;
  out->type = eCOLON;
  out->left = lexp;
  out->right = rexp;
  return out;
}

SExpression *createOperation(EOperationType type, SExpression *left,
                             SExpression *right) {
  SExpression *b = allocateExpression();

  if (b == NULL)
    return NULL;

  b->type = type;
  b->left = left;
  b->right = right;
  // printf("createOperation is called\n");
  return b;
}

void deleteExpression(SExpression *b) {
  if (b == NULL)
    return;
  if (b->name != NULL)
    free(b->name);
  deleteExpression(b->left);
  deleteExpression(b->right);

  free(b);
}
