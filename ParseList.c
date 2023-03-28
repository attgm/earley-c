/* ParseList.c */
#include <stdio.h>
#include <stdlib.h>
#include "header.h"

/* InitParseList */
/*    大きさnのParseListを作成し，初期化する */
void InitParseList(ParseList *pl, int n, Grammar *gr) {
  int i, j;
  TermTable **termtable;

  pl->iSize = n;
  pl->pTermTable = calloc(sizeof(TermTable *), (n * n));
  termtable = pl->pTermTable;

  /* 配列の左下三角形部にTermTableを割り当てる */
  for (j = 1; j < n; j++) {
    for (i = 0; i < j; i++) {
      termtable[j * n + i] = malloc(sizeof(TermTable));
      InitTermTable(termtable[j * n + i], gr->iTermNum);
    }
  }

  /* 対角線成分には同じものが入る */
  termtable[0] = malloc(sizeof(TermTable));
  InitTermTable(termtable[0], gr->iTermNum);
  for (i = 0; i < n; i++) {
    termtable[i * n + i] = termtable[0];
  }
};

/* DelParselist */
/*   ParseListのメモリ空間を解放する */
void DelParseList(ParseList *pl) {
  int i, j;
  for (j = 0; j < pl->iSize; j++) {
    for (i = 0; i < j; i++) {
      DelTermTable(pl->pTermTable[j * pl->iSize + i]);
      free(pl->pTermTable[j * pl->iSize + i]);
    }
  }
  DelTermTable(pl->pTermTable[0]);
  free(pl->pTermTable[j * pl->iSize + i]);
  free(pl->pTermTable);
};

/* InitTermTable */
/*   TermTableの配列用メモリ空間を確保し， */
/*   初期化する								*/
void InitTermTable(TermTable *tt, int size) {
  int i;

  tt->iSize = size + 1; /* εのの分+1する */
  /* メモリ領域の確保 */
  tt->pParseUnit = calloc(sizeof(ParseUnit *), tt->iSize);
  for (i = 0; i < tt->iSize; i++) {
    tt->pParseUnit[i] = NULL;
  }
};

/* DelTermTable */
/*	TermTableのメモリ空間を解放する　*/
void DelTermTable(TermTable *tt) {
  int i;
  ParseUnit *pe;
  ParseUnit *next;

  for (i = 0; i < tt->iSize; i++) {
    pe = tt->pParseUnit[i];
    while (pe) {
      next = pe->pNext;
      DelParseUnit(pe);
      free(pe);
      pe = next;
    }
  }
  free(tt->pParseUnit);
};

/* InitParseUnit */
/*   ParseUnitを初期化する　*/
void InitParseUnit(ParseUnit *pu, ParseElement *pe, int loc) {
  pu->pElement = pe;
  pu->iElementNum = 1;
  pu->iRuleNo = pe->iRuleNo;
  pu->iDotLoc = loc;
  pu->pNext = NULL;
};

/* DelParseUnit */
/*    ParseUnitのメモリ領域を解放する */
void DelParseUnit(ParseUnit *pu) {
  ParseElement *p, *next;

  p = pu->pElement;
  while (p) {
    next = p->pNext;
    free(p);
    p = next;
  }
};

/* InitParseElement */
/*	ParseElementの中味を設定する */
void InitParseElement(ParseElement *pe, int ruleno, double d, ParseElement *p1,
                      ParseElement *p2) {
  pe->iRuleNo = ruleno;
  pe->dProb = d;
  pe->pBackPtr1 = p1;
  pe->pBackPtr2 = p2;
  pe->pNext = NULL;
};

/* InsertParseList */
/*   ParseListに(x,y) ,ドットの次のTermがtermno,ドットの位置loc */
/*   のParseElement peを挿入する                                     */
void InsertParseList(ParseList *pl, int x, int y, int termno, int loc,
                     ParseElement *pe) {
  TermTable *tt;
  ParseUnit *ptr, *pre;

  if (x > y) {
    /* debug */
    fprintf(stderr, "Error:Insert ParseElement in PL(%d,%d)\n", x, y);
  } else {
    /* (x,y)のTermTableを得る */
    tt = pl->pTermTable[y * (pl->iSize) + x];
    /* ドットの次の番号がTermNoのParseUnitを得る */
    ptr = tt->pParseUnit[termno];
    /*  もしNULLなら新しいParseUnitを作成する       */
    if (ptr == NULL) {
      ptr = malloc(sizeof(ParseUnit));
      InitParseUnit(ptr, pe, loc);
      tt->pParseUnit[termno] = ptr;
      return;
    } else {
      /* もし既にParseUnitが存在するなら，           */
      /*  リストをたどり，適切な場所に格納する */
      while (ptr) {
        if ((ptr->iRuleNo == pe->iRuleNo) && (ptr->iDotLoc == loc)) {
          InsertParseUnit(ptr, pe);
          return;
        }
        pre = ptr;
        ptr = ptr->pNext;
      }
      /* もし適切な場所がなかったら　*/
      /*  新しくParseUnitを作成する	  */
      ptr = malloc(sizeof(ParseUnit));
      InitParseUnit(ptr, pe, loc);
      pre->pNext = ptr;
    }
  }
};

/* InsertParseUnit */
/*    ParseUnitにElementを挿入する　*/
void InsertParseUnit(ParseUnit *pu, ParseElement *pe) {
  ParseElement *ptr, *pre;

  ptr = pu->pElement;
  pre = NULL;
  while (ptr != NULL) {
    if (ptr->dProb < pe->dProb) { /*もし，挿入する確率値の方が大きければ */
      pe->pNext = ptr;     /* 直前に挿入する */
      if (pre == NULL) {   /* もし先頭の要素なら */
        pu->pElement = pe; /* Headを入れ替える */
      } else {
        pre->pNext = pe; /* 前の要素と今の要素の間に挿入する */
      }
      if (pu->iElementNum >= iRESNUM) {
        /*もし挿入することによってk個以上になるなら */
        /* リストの最後の要素を削除する */
        while (ptr->pNext->pNext != NULL) {
          ptr = ptr->pNext;
        }
        free(ptr->pNext);
        ptr->pNext = NULL;
      } else {
        pu->iElementNum++;
      }
      return;
    }
    pre = ptr;
    ptr = ptr->pNext;
  }
  /* 最後の要素よりも確率値が小さかったとき */
  if (pu->iElementNum < iRESNUM) { /* もしk個以上リストに存在しないならば */
    pre->pNext = pe;               /*　最後に挿入する */
    pu->iElementNum++;
  }
};

/* SearchParseList */
/*   ParseListに(x,y)ドットの次のTermがtermnoの要素をParseListから */
/*   探索する        						   */
ParseUnit *SearchParseList(ParseList *pl, int x, int y, int termno) {
  TermTable *tt;
  ParseUnit *ptr;

  /* (x,y)のTermTableを得る */
  tt = pl->pTermTable[y * (pl->iSize) + x];
  /* ドットの次の番号がTermNoのParseUnitを得る */
  ptr = tt->pParseUnit[termno];
  return ptr;
};

/* NextElement */
/*     次のElementを得る　				 */
/*     もし，Elementが終ったら，Unitをインクリメントして */
/*      次のUnitのElementを得る				 */
void NextElement(ParseUnit **pu, ParseElement **pe) {
  *pe = (*pe)->pNext;
  if (*pe == NULL) {
    *pu = (*pu)->pNext;
    if (*pu != NULL) {
      *pe = (*pu)->pElement;
    }
  }
};

/* PrintParseElement */
/*   4つ組を出力する */
void PrintElement(ParseElement *pe, int i, int j, Grammar *g) {
  char rule[iLINEMAX];

  NumToRule(rule, pe->iRuleNo, g);
  printf("[%s,(%d,%d),%f]\n", rule, i, j, pe->dProb);
}
