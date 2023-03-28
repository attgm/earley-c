#include "header.h"
#include <stdio.h>
#include <stdlib.h>

/* 大域変数 */
static Grammar *g;
static ParseList *pl;

/* Regist */
/*     文法grammarで入力文字列inputを構文解析し， */
/*	  parse listに登録するルーチン		  */
void Regist(ParseList *parselist, Grammar *grammar, int *input) {
  int i;

  /* 大域変数の設定 */
  g = grammar;
  pl = parselist;

  Step1();
  for (i = 1; i < parselist->iSize; i++) {
    Step2(i, input);
    Step3(i);
  }
};

/* Step1 */
/*    A ->α => A->・α,i,i,0 			*/
/*    (i,i)はすべて同じテーブルを参照するので	*/
/*    (0,0)にだけ挿入すればよい		        */
void Step1(void) {
  int i;
  ParseElement *pe;
  int term;

  /* すべての生成規則に対して */
  for (i = 0; i < g->iRuleNum; i++) {
    /* 新しくElementをつくり，挿入する*/
    pe = malloc(sizeof(ParseElement));
    InitParseElement(pe, i, g->rRuleH[i]->dProb, NULL, NULL);
    term = TermAfterDot(g, i, 0);
    InsertParseList(pl, 0, 0, term, 0, pe);
  }
};

/* Step2 */
/*  [ * -> * ・a(i-1)*,j,i-1] =>[ * -> * a(i-1)・*,j,i] */
void Step2(int i, int *input) {
  int j, dotloc, term;
  ParseUnit *pu;
  ParseElement *pe, *newElement;

  for (j = 0; j < i; j++) {
    /* [ * -> * .a(i-1)*,j,i-1] となるElementを探す*/
    pu = SearchParseList(pl, j, i - 1, input[i - 1]);

    if (pu != NULL) {
      pe = pu->pElement;
      while (pu != NULL) {
        newElement = malloc(sizeof(ParseElement));
        InitParseElement(newElement, pe->iRuleNo, pe->dProb, pe, NULL);
        dotloc = pu->iDotLoc + 1;
        term = TermAfterDot(g, pe->iRuleNo, dotloc);
        InsertParseList(pl, j, i, term, dotloc, newElement);

        NextElement(&pu, &pe);
      }
    }
  }
};

/* Step3            */
/*    [ A -> * .,k.i],[ * ->  *・A *,j,k] => [ * ->  *A .*,j,k] */
void Step3(int i) {
  int j, k;
  ParseUnit *pu1, *pu2;
  ParseElement *pe1, *pe2, *newElement;
  int leftterm, dotloc, term;

  for (j = i - 1; j >= 0; j--) {
    for (k = i - 1; k >= j; k--) {
      /* [ A -> * ・,k,i] となるElementを探す*/
      pu1 = SearchParseList(pl, k, i, 0);
      if (pu1 != NULL) {
        pe1 = pu1->pElement;
        /*　検索したそれぞれの要素に対して */
        while (pu1 != NULL) {
          /* 生成規則の左辺をとりだす */
          leftterm = g->rRuleH[pe1->iRuleNo]->iLeft;
          /* [ * ->  *.A *,j,k]となるElementを探す */
          pu2 = SearchParseList(pl, j, k, leftterm);

          if (pu2 != NULL) {
            pe2 = pu2->pElement;
            /*　検索したそれぞれの要素に対して */
            while (pu2 != NULL) {
              /* [ * ->  *A .*,j,k]となるElementを挿入する */
              newElement = malloc(sizeof(ParseElement));
              InitParseElement(newElement, pe2->iRuleNo,
                               (pe2->dProb * pe1->dProb), pe2, pe1);
              dotloc = pu2->iDotLoc + 1;
              term = TermAfterDot(g, pe2->iRuleNo, dotloc);
              InsertParseList(pl, j, i, term, dotloc, newElement);
              /* if j==i && term==0... */
              NextElement(&pu2, &pe2);
            }
          }
          NextElement(&pu1, &pe1);
        }
      }
    }
  }
};

/* BackTrace               */
/*  先頭のElementを取って  */
/*  バックトレースを行なう */
void BackTrace(Grammar *grammar, ParseList *parselist,
               void (*func)(ParseElement *, Grammar *)) {
  ParseElement **root1, **root2, **root;
  ParseUnit *pu;
  ParseElement *pe;
  int i, counter;

  /* 開始記号から始めて使われる生成規則を格納する領域の確保 */
  root1 = calloc(sizeof(ParseElement *), iRESNUM + 1);
  root2 = calloc(sizeof(ParseElement *), iRESNUM + 1);
  for (i = 0; i <= iRESNUM; i++) {
    root1[i] = NULL;
  }

  /* [S->γ.,0,n]なる要素を検索する*/
  /*   (実際は[*->*.,0,n] )	   */
  pu = SearchParseList(parselist, 0, (parselist->iSize - 1), 0);
  /* マージソートでk個だけ取り出す */
  while (pu != NULL) {
    /* 開始記号と右辺が同じものだけに対して */
    if (grammar->rRuleH[pu->iRuleNo]->iLeft == grammar->iStartTerm) {
      /* root1[]とpu->pElement(リスト)をマージして */
      /* root2[]に入れていく                       */
      pe = pu->pElement;
      root = root1;
      for (i = 0; i < iRESNUM; i++) {
        /* マージするリストが既に空の場合 */
        if (pe == NULL) {
          root2[i] = *root;
          root++;
        } else {
          /* マージするリストがまだある場合 */
          /* 元の配列がもう空であった場合   */
          if (*root == NULL) {
            root2[i] = pe;
            pe = pe->pNext;
          } else {
            /* 配列とリストの確率値を比較する */
            if ((*root)->dProb > pe->dProb) {
              /* 配列の方が大きい場合 */
              root2[i] = *root;
              root++;
            } else {
              /* リストの方が大きい場合 */
              PrintElement(pe, 0, (parselist->iSize - 1), g);
              root2[i] = pe;
              pe = pe->pNext;
            }
          }
        }
      }
      /* 配列のいれかえ */
      root = root2;
      root2 = root1;
      root1 = root;
    }
    pu = pu->pNext;
  }

  /* 得た構文木の先頭それぞれからバックトレースを始める */
  if (func == NULL) {
    for (i = 0; root1[i] != NULL; i++) {
      RutineR(root1[i]);
      printf("--\n");
    }
  } else {
    for (i = 0; root1[i] != NULL; i++) {
      func(root1[i], grammar);
    }
  }
  free(root1);
  free(root2);
};

/* RutineR */
/*   実際にバックトレースを行なうルーチン */
/*   再帰的に呼び出される                 */
void RutineR(ParseElement *pe) {
  static char rule[50];

  if (pe->pBackPtr1 == NULL) {
    NumToRule(rule, pe->iRuleNo, g);
    printf("%s\n", rule);
  } else {
    if (pe->pBackPtr2 != NULL) {
      RutineR(pe->pBackPtr2);
    }
    RutineR(pe->pBackPtr1);
  }
};
