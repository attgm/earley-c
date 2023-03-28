/* main */
/*  mainプログラム(の一例) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 1.header.hをincludeすること */
#include "header.h"

#define FILENAME "rule.txt"       /* 生成規則のファイル名 */
#define INPUTFILENAME "input.txt" /* 入力ファイル名       */
#define iINPUTNUM 30              /* 入力文字列のTerm名   */

int main() {
  FILE *fp;
  Grammar grammar;
  ParseList parselist;
  int input[iINPUTNUM];
  char tmp[iLINEMAX];
  char *token;
  int inputnum, errcode;

  if ((fp = fopen(FILENAME, "r")) == NULL) {
    fprintf(stderr, "Error: Cannot open file:%s\n", FILENAME);
    exit(0);
  }
  /* 2.開いた文法ファイルのファイルポインタから */
  /*   文法のデータベースを作成する             */
  if ((errcode = InitGrammar(&grammar, fp)) != 0) {
    fprintf(stderr, "Error: Rule file error %s:%d\n", FILENAME, errcode);
    exit(0);
  }
  fclose(fp);

  /* 3.入力文字列を読み込む */
  if ((fp = fopen(INPUTFILENAME, "r")) == NULL) {
    fprintf(stderr, "Error: Cannot open file:%s", INPUTFILENAME);
    exit(0);
  }
  if (fgets(tmp, iLINEMAX, fp) == NULL) {
    fprintf(stderr, "Error: Input File %s is something wrong\n", INPUTFILENAME);
  }
  KillReturn(tmp); /* 改行コードを消去するのを忘れないように */

  /* 4.入力文字列をトークンに分ける */
  token = tmp;
  inputnum = 0;
  while ((token = strtok(token, " ")) != NULL) {
    if ((input[inputnum] = TermToNum(&grammar, token)) == -1) {
      printf("Unknown Term:%s\n", token);
      exit(0);
    }
    token = NULL;
    inputnum++;
  }

  /* 5.入力文字列の長さ + 1のParseListを作成する */
  InitParseList(&parselist, inputnum + 1, &grammar);
  /* 6.ParseList登録プログラムを行う */
  Regist(&parselist, &grammar, input);
  /* 7.バックトレースを行う */
  BackTrace(&grammar, &parselist, NULL);
  /* 8.ParseListのメモリを開放する */
  DelParseList(&parselist);
};
