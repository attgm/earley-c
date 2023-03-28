/* Rules.c     			  */
/*   生成規則を保持するRuleの実装 */
#include "header.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* InitGrammar */
/*	   何も問題がなければ 0を  */
/*  　何か問題があればその行を返す */
int InitGrammar(Grammar *g, FILE *fp) {
  char sTmp[iLINEMAX]; /* 読み込み用のテンポラリ領域 */
  int i;
  char *token; /* 切り出したTokenへのポインタ */
  Rule *rule;

  g->lTerm = NULL;
  g->iLock = 0; /* ロック解除 */
  /* 1行目を読み取って，生成規則の数を得る */
  if (fgets(sTmp, iLINEMAX, fp) == NULL) {
    return 1;
  }
  g->iRuleNum = strtol(sTmp, NULL, 0);
  if (g->iRuleNum < 1) {
    return 1;
  }

  /* 2行目を読み取って，開始記号を得る */
  if (fgets(sTmp, iLINEMAX, fp) == NULL) {
    return 2;
  }
  KillReturn(sTmp);
  g->iStartTerm = TermToNum(g, sTmp);

  /* 生成規則(へのポインタ)の領域を確保する */
  g->rRuleH = calloc(sizeof(Rule *), g->iRuleNum);

  for (i = 0; i < g->iRuleNum; i++) {
    if (fgets(sTmp, iLINEMAX, fp) == NULL) {
      /* +2 = +1(lineは0から始まらないので)+1(生成規則の数の分) */
      return (i + 2);
    }
    KillReturn(sTmp);
    rule = g->rRuleH[i] = malloc(sizeof(Rule));
    /* 確率値を読み取る */
    if ((token = strtok(sTmp, " ")) == NULL) {
      return (i + 2);
    };
    if (sscanf(token, "%lf", &(rule->dProb)) == EOF) {
      return (i + 2);
    }
    /* 左辺を読み取る */
    if ((token = strtok(NULL, " ")) == NULL) {
      return (i + 2);
    };
    rule->iLeft = TermToNum(g, token);
    /* 右辺を読み取る */
    rule->iRightNum = 0;
    while ((token = strtok(NULL, " ")) != NULL) {
      rule->iRight[rule->iRightNum] = TermToNum(g, token);
      rule->iRightNum++;
      if (rule->iRightNum >= iRIGHTMAX) {
        return (i + 2);
      }
    }
    if (rule->iRightNum == 0) {
      return (i + 2);
    }
  }
  g->iLock = 1; /* ロック */
  g->iTermNum = CountTerm(g->lTerm);
  return 0;
};

/* CountTerm */
/*    Termの数を数える */
int CountTerm(const TermList *tl) {
  int i;

  for (i = 1; tl != NULL; i++) {
    tl = tl->pNext;
  };
  return i;
};

/* Termを対応する番号に変換する 			    */
/*  もしGrammarがロックされていて，変換が不可能なら-1を返す */
/*  それ以外は変換した番号を返す			    */
int TermToNum(Grammar *g, const char *term) {
  TermList *tl;
  TermList *pre;
  int i;

  tl = g->lTerm;
  i = 1;
  /* 既存のTermListにtermがないかどうか検索する */
  while (tl != NULL) {
    if (strcmp(tl->sTerm, term) == 0) {
      return i;
    }
    pre = tl;
    tl = tl->pNext;
    i++;
  }
  /* なかった場合 */
  if (g->iLock == 0) {
    if (tl == g->lTerm) {
      g->lTerm = malloc(sizeof(TermList));
      tl = g->lTerm;
    } else {
      pre->pNext = malloc(sizeof(TermList));
      tl = pre->pNext;
    }
    strcpy(tl->sTerm, term);
    tl->pNext = NULL;
    return i;
  } else {
    return -1; /* ロックしてあったら-1を返す */
  }
};

/* NumToTerm */
/* Term Numberを 対応するTermに変換する  */
/*  εもしくは該当なしの場合，""を返す   */
void NumToTerm(char *term, int termno, Grammar *g) {
  TermList *tl;

  /* もし0なら""を返す */
  if (termno == 0) {
    *term = '\0';
    return;
  }
  /* termnoの数だけリストを辿る */
  tl = g->lTerm;
  for (; termno > 1; termno--) {
    tl = tl->pNext;
    if (tl == NULL) {
      *term = '\0';
      return;
    }
  }
  strcpy(term, tl->sTerm);
};

/* TermAfterDot */
/*    ドットの次のTerm(に対応する番号)を返す */
int TermAfterDot(Grammar *gra, int ruleno, int dotloc) {
  Rule *rule;

  rule = gra->rRuleH[ruleno];
  if (rule->iRightNum > dotloc) {
    /* もし後に文字があるならその文字番号を返す */
    return rule->iRight[dotloc];
  } else {
    /* もし最後なら0(ε)を返す */
    return 0;
  }
};

/* KillReturn            */
/*  改行コードを消去する */
void KillReturn(char *s) {
  while (s != '\0') {
    if (*s == '\n') {
      *s = '\0';
      return;
    }
    s++;
  }
};

/* NumberToRule */
/*  生成規則番号を生成規則の文字列に変換する */
void NumToRule(char *rule, int ruleno, Grammar *g) {
  char term[iTERMLENGTHMAX];
  int i;
  Rule *r;

  r = g->rRuleH[ruleno];
  /* 左辺をいれる */
  NumToTerm(rule, r->iLeft, g);
  strcat(rule, " -> ");
  /* 右辺をいれる */
  for (i = 0; i < r->iRightNum; i++) {
    NumToTerm(term, r->iRight[i], g);
    strcat(rule, term);
    strcat(rule, " ");
  }
};
