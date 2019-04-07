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
  b->cond = NULL;
  b->arglist = NULL;
  return b;
}

SExpression *createIF(SExpression *cond, SExpression *left,
                      SExpression *right) {
  SExpression *out = allocateExpression();

  out->type = eIF;
  out->cond = cond;
  out->left = left;
  out->right = right;
  return out;
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

SExpression *createDefun(char const *name, Arglist *arglist, SExpression *exp) {
  SExpression *out = allocateExpression();

  if (out == NULL)
    return NULL;
  out->type = eDEFUN;
  out->arglist = arglist;
  out->name = (char *)malloc(strlen(name) + 1);
  strncpy(out->name, name, strlen(name) + 1);
  out->left = exp;
  return out;
}

Arglist *appendArglist(char const *name, Arglist *arglist) {
  Arglist *newArg = (Arglist *)malloc(sizeof(Arglist));
  newArg->name = (char *)malloc(strlen(name) + 1);
  strncpy(newArg->name, name, strlen(name) + 1);
  newArg->next = arglist;
  return newArg;
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

void deleteArglist(Arglist *arglist) {
  if (!arglist)
    return;
  auto next = arglist->next;
  if (arglist->name)
    free(arglist->name);
  free(arglist);
  deleteArglist(next);
}

void deleteExpression(SExpression *b) {
  if (b == NULL)
    return;
  if (b->name != NULL)
    free(b->name);
  if (b->arglist)
    deleteArglist(b->arglist);
  deleteExpression(b->left);
  deleteExpression(b->right);

  free(b);
}
