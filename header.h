/* Header.h */
/* グローバル変数 */
#define iRIGHTMAX 5   /* 左辺のTermの最大値 */
#define iLINEMAX 1024 /* ファイル読み込み用のバッファの最大値 */
#define iRESNUM 5     /* いくつ求めるか     */
#define iTERMLENGTHMAX 40 /* Termの長さの最大値 */

/* TermList	       */
/* Term(char型)の配列  */
typedef struct termlist {
  char sTerm[iTERMLENGTHMAX];
  struct termlist *pNext;
} TermList;

/* Rule				  */
/*    1つの生成規則を表わす構造体 */
typedef struct rule {
  int iLeft;             /* 左辺のTerm(に対応する数値) */
  int iRightNum;         /* 右辺の数 */
  int iRight[iRIGHTMAX]; /*  右辺のTerm(に対応する数値)の配列 */
  double dProb;          /*   適用確率 */
} Rule;

/* Grammar  			      */
/*    確率文脈自由文法を表わす構造体　*/
typedef struct grammar {
  TermList *lTerm; /* Termのリストの先頭 */
  int iTermNum;    /* Termの数	         */
  Rule **rRuleH;   /* 生成規則の集合     */
  int iRuleNum;    /* 生成規則の数       */
  int iStartTerm;  /* 開始記号           */
  int iLock;       /* ロックしてあるかどうか */
} Grammar;

/* ParseElement */
/*     ParseListの最小単位の要素を表わす. 		 */
/*     同じ生成規則,ドットの位置の物がListで繋がれている */
/*	  (HeadはParseUnit)				 */
typedef struct parseelement {
  int iRuleNo;                    /* 生成規則番号      */
  double dProb;                   /* 確率値 		 */
  struct parseelement *pBackPtr1; /* バックポインタ */
  struct parseelement *pBackPtr2;
  struct parseelement *pNext; /* 次のElementへのポインタ */
} ParseElement;

/* ParseUnit */
/*    同じ生成規則番号,ドットの位置をもつParseElementのリストのヘッダ部 */
/*	ドットの次が同じ物がListで繋がれている				*/
typedef struct parseunit {
  ParseElement *pElement;  /* エレメントの集合の先頭要素  */
  int iElementNum;         /* エレメントの数　	       */
  int iRuleNo;             /* 生成規則の番号 	       */
  int iDotLoc;             /* ドットの位置　	       */
  struct parseunit *pNext; /*  次のUnitへのポインタ       */
} ParseUnit;

/* TermTable */
/*     ドットの次が同じParseUnitの集合　*/
/*     (i,j)ごとに存在する	        */
typedef struct termtable {
  ParseUnit **pParseUnit; /* ParseUnitへのポインタの配列 */
  int iSize;              /* 配列の大きさ */
} TermTable;

/* ParseList */
/*   パーズリスト．(n,n)のTermTableの配列である */
typedef struct parselist {
  TermTable **pTermTable; /* TermTableへのポインタの二次元配列 */
  int iSize;              /* 配列の大きさ */
} ParseList;

/* ProtoType宣言 */
/* Grammar.c */
int InitGrammar(Grammar *, FILE *);
int TermToNum(Grammar *, const char *);
int CountTerm(const TermList *);
void NumToTerm(char *, int, Grammar *);
int TermAfterDot(Grammar *, int, int);
void KillReturn(char *);
void NumToRule(char *, int, Grammar *);

/* ParseList.c */
void InitParseList(ParseList *, int, Grammar *);
void DelParseList(ParseList *);
void InitTermTable(TermTable *, int);
void DelTermTable(TermTable *);
void InitParseUnit(ParseUnit *, ParseElement *, int);
void DelParseUnit(ParseUnit *);
void InitParseElement(ParseElement *, int, double, ParseElement *,
                      ParseElement *);
void InsertParseList(ParseList *, int, int, int, int, ParseElement *);
void InsertParseUnit(ParseUnit *, ParseElement *);
ParseUnit *SearchParseList(ParseList *, int, int, int);
void NextElement(ParseUnit **, ParseElement **);
void PrintElement(ParseElement *, int, int, Grammar *);

/* Registration.c */
void Regist(ParseList *, Grammar *, int *);
void Step1(void);
void Step2(int, int *);
void Step3(int);
void BackTrace(Grammar *, ParseList *, void (*func)(ParseElement *, Grammar *));
void RutineR(ParseElement *);
