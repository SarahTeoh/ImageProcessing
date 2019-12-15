#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/*
 * マクロ定義
 */
#define min(A, B) ((A)<(B) ? (A) : (B))
#define max(A, B) ((A)>(B) ? (A) : (B))
#define COEFFICIENTSIZE 3
/*
 * 画像構造体の定義
 */
typedef struct
{
    int width;              /* 画像の横方向の画素数 */
    int height;             /* 画像の縦方向の画素数 */
    int maxValue;           /* 画素の値(明るさ)の最大値 */
    unsigned char *data;    /* 画像の画素値データを格納する領域を指す */
                            /* ポインタ */
} image_t;


/*======================================================================
 * このプログラムに与えられた引数の解析
 *======================================================================
 */
void
parseArg(int argc, char **argv, FILE **infp, FILE **outfp, int *typeOfFilter, int *typeOfEquation)
{
    FILE *fp;

    /* 引数の個数をチェック */
    if (argc<4)
    {
        goto usage;
    }

    *infp = fopen(argv[1], "rb"); /* 入力画像ファイルをバイナリモードで */
                                /* オープン */

    if (*infp==NULL)		/* オープンできない時はエラー */
    {
        fputs("Opening the input file was failend\n", stderr);
        goto usage;
    }

    *outfp = fopen(argv[2], "wb"); /* 出力画像ファイルをバイナリモードで */
                                /* オープン */

    *typeOfFilter = atoi(argv[3]);

    if(argv[4]== NULL){
      *typeOfEquation = 0;
    }else{
      *typeOfEquation = atoi(argv[4]);
    };

    if (*outfp==NULL)		/* オープンできない時はエラー */
    {
        fputs("Opening the output file was failend\n", stderr);
        goto usage;
    }

    return;

/* このプログラムの使い方の説明 */
usage:
    fprintf(stderr, "usage : %s <input pgm file> <output pgm file> <type of filter> [which equation: 2 or 3]\n", argv[0]);
    exit(1);
}


/*======================================================================
 * 画像構造体の初期化
 *======================================================================
 * 画像構造体 image_t *ptImage の画素数(width × height)、階調数
 * (maxValue)を設定し、画素値データを格納するのに必要なメモリ領域を確
 * 保する。
 */
void
initImage(image_t *ptImage, int width, int height, int maxValue)
{
    ptImage->width = width;
    ptImage->height = height;
    ptImage->maxValue = maxValue;

    /* メモリ領域の確保 */
    ptImage->data = (unsigned char *)malloc((size_t)(width * height));

    if (ptImage->data==NULL)    /* メモリ確保ができなかった時はエラー */
    {
        fputs("out of memory\n", stderr);
        exit(1);
    }
}

/*======================================================================
 * 文字列一行読み込み関数
 *======================================================================
 *   FILE *fp から、改行文字'\n'が表れるまで文字を読み込んで、char型の
 * メモリ領域 char *buf に格納する。1行の長さが n 文字以上の場合は、先
 * 頭から n-1 文字だけを読み込む。
 *   読み込んだ文字列の先頭が '#' の場合は、さらに次の行を読み込む。
 *   正常に読み込まれた場合は、ポインタ buf を返し、エラーや EOF (End
 * Of File) の場合は NULL を返す。
 */
char *
readOneLine(char *buf, int n, FILE *fp)
{
    char *fgetsResult;

    do
    {
        fgetsResult = fgets(buf, n, fp);
    } while(fgetsResult!=NULL && buf[0]=='#');
            /* エラーや EOF ではなく、かつ、先頭が '#' の時は、次の行 */
            /* を読み込む */

    return fgetsResult;
}


/*======================================================================
 * PGM-RAW フォーマットのヘッダ部分の読み込みと画像構造体の初期化
 *======================================================================
 *   PGM-RAW フォーマットの画像データファイル FILE *fp から、ヘッダ部
 * 分を読み込んで、その画像の画素数、階調数を調べ、その情報に従って、
 * 画像構造体 image_t *ptImage を初期化する。
 *   画素値データを格納するメモリ領域も確保し、この領域の先頭を指すポ
 * インタを ptImage->data に格納する。
 *
 * !! 注意 !!
 *   この関数は、ほとんどの場合、正しく動作するが、PGM-RAWフォーマット
 * の正確な定義には従っておらず、正しいPGM-RAWフォーマットのファイルに
 * 対して、不正な動作をする可能性がある。なるべく、本関数をそのまま使
 * 用するのではなく、正しく書き直して利用せよ。
 */
void
readPgmRawHeader(FILE *fp, image_t *ptImage)
{
    int width, height, maxValue;
    char buf[128];

    /* マジックナンバー(P5) の確認 */
    if(readOneLine(buf, 128, fp)==NULL)
    {
        goto error;
    }
    if (buf[0]!='P' || buf[1]!='5')
    {
        goto error;
    }

    /* 画像サイズの読み込み */
    if (readOneLine(buf, 128, fp)==NULL)
    {
        goto error;
    }
    if (sscanf(buf, "%d %d", &width, &height) != 2)
    {
        goto error;
    }
    if ( width<=0 || height<=0)
    {
        goto error;
    }

    /* 最大画素値の読み込み */
    if (readOneLine(buf, 128, fp)==NULL)
    {
        goto error;
    }
    if (sscanf(buf, "%d", &maxValue) != 1)
    {
        goto error;
    }
    if ( maxValue<=0 || maxValue>=256 )
    {
        goto error;
    }

    /* 画像構造体の初期化 */
    initImage(ptImage, width, height, maxValue);

    return;

/* エラー処理 */
error:
    fputs("Reading PGM-RAW header was failed\n", stderr);
    exit(1);
}


/*======================================================================
 * PGM-RAWフォーマットの画素値データの読み込み
 *======================================================================
 *   入力ファイル FILE *fp から総画素数分の画素値データを読み込んで、
 * 画像構造体 image_t *ptImage の data メンバーが指す領域に格納する
 */
void
readPgmRawBitmapData(FILE *fp, image_t *ptImage)
{
    /* ファイルfpからサイズsizeof(unsigned char)バイトのデータをptImage->width * ptImage->height個読み込み、ptImage->dataに格納 */
    if( fread(ptImage->data, sizeof(unsigned char),
            ptImage->width * ptImage->height, fp)
            != ptImage->width * ptImage->height )
    {
        /* エラー */
        fputs("Reading PGM-RAW bitmap data was failed\n", stderr);
        exit(1);
    }
}

/*======================================================================
 * フィルタリング(ネガポジ反転)
 *======================================================================
 *   画像構造体 image_t *originalImage の画像をフィルタリング(ネガポジ
 * 反転)して、image_t *resultImage に格納する
 */
void
filteringImage(image_t *resultImage, image_t *originalImage, int width, int height)
{
    int     x, y;

    for(y=0; y<height; y++)
    {
        for(x=0; x<width; x++)
        {
          /* ネガポジ反転*/
            resultImage->data[x+resultImage->width*y]
                    = ( originalImage->maxValue
                    -originalImage->data[x+originalImage->width*y] )
                    *resultImage->maxValue/originalImage->maxValue;
        }
    }
}

/* Robertsフィルタ */
void
roberts(image_t *resultImage, image_t *originalImage, int width, int height)
{
    int x, y, l, m;
    float g1, g2;
    float tmpPixel;

    for(y=0; y<height; y++)
    {
        for(x=0; x<width; x++)
        {
          /* x+1 か y+1 が画像の外にはみ出した場合の対策 */
          /* はみ出した場合、画像の端の階調値とする */
          l = ((x+1)>width-1) ? width-1 : x+1;
          m = ((y+1)>height-1) ? height-1 : y+1;

          /* Robertsフィルタの式 */
          g1 = sqrt(originalImage->data[x+originalImage->width*y]) - sqrt(originalImage->data[l+originalImage->width*m]);
          g2 = sqrt(originalImage->data[x+originalImage->width*m]) - sqrt(originalImage->data[l+originalImage->width*y]);

          /* Robertsフィルタの式からルート(2乗根)を取り除いた式 */
          //g1 = originalImage->data[x+originalImage->width*y] - originalImage->data[l+originalImage->width*m];
          //g2 = originalImage->data[x+originalImage->width*m] - originalImage->data[l+originalImage->width*y];

          tmpPixel = sqrt(g1*g1 + g2*g2);

          /* 階調値が0~255の範囲に収まらない場合、255にする */
          /* そうでなければ、出力画像の階調値とする */
          tmpPixel = (tmpPixel<0) ? 0 : ((tmpPixel>=255) ? 255 : tmpPixel );
          resultImage->data[x+resultImage->width*y] = tmpPixel;

        }
    }
}

/*  PrewittフィルタかSobelフィルタ */
void
prewittOrSobel(image_t *resultImage, image_t *originalImage, int *typeOfFilter, int *typeOfEquation, int width, int height)
{
  double tmpX, tmpY;
  int o, p, j, k, x, y;
  double tmpPixel;

  //Prewittフィルタ
  int prewittX[9] = {-1, 0, 1, //水平方向
                    -1, 0, 1,
                    -1, 0, 1};
  int prewittY[9] = {-1,-1,-1, //垂直方向
                    0, 0, 0,
                    1, 1, 1};

  //Sobelフィルタ
  int sobelX[9] = {-1, 0, 1, //水平方向
                    -2, 0, 2,
                    -1, 0, 1};
  int sobelY[9] = {-1, -2, -1, //垂直方向
                    0,  0,  0,
                    1,  2,  1};

  for(y=0; y<height; y++)
  {
    for(x=0; x<width; x++)
    {
      /* 初期化 */
      tmpX = 0.0;
      tmpY = 0.0;

      for(o=-1; o<=1; o++){
        for(p=-1; p<=1; p++){
          j = ((x+o)<0) ? 0 : (((x+o)>width-1) ? width-1 : x+o);
          k = ((y+p)<0) ? 0 : (((y+p)>height-1) ? height-1 : y+p);
          if(*typeOfFilter==2){ /* コマンドラインでPrewittフィルタを指定した場合 */
            tmpX += prewittX[(o+1)+COEFFICIENTSIZE*(p+1)]*originalImage->data[j+originalImage->width*k];
            tmpY += prewittY[(o+1)+COEFFICIENTSIZE*(p+1)]*originalImage->data[j+originalImage->width*k];
          }else if(*typeOfFilter==3){ /* コマンドラインでSoberフィルタを指定した場合 */
            tmpX += sobelX[(o+1)+COEFFICIENTSIZE*(p+1)]*originalImage->data[j+originalImage->width*k];
            tmpY += sobelY[(o+1)+COEFFICIENTSIZE*(p+1)]*originalImage->data[j+originalImage->width*k];
          }
        }
      }

      /* コマンドラインで式(2)を指定した場合 */
      if(*typeOfEquation==2){
        tmpPixel = sqrt(tmpX*tmpX + tmpY*tmpY);
      }else{ /* 特に指定しなければ式(3)を使う */
        tmpPixel = fabs(tmpX) + fabs(tmpY);
      }
      tmpPixel = (tmpPixel<0) ? 0 : ((tmpPixel>=255) ? 255 : tmpPixel ); /* 階調値が0から255の範囲に収るようにする*/
      resultImage->data[x+resultImage->width*y] = tmpPixel; /* 新しい階調値を設定する*/
    }
  }
}

/* 各階調値の画素の数 */
void
calcTotalPixelsInClass(image_t *originalImage, int n[], int width, int height){
  int x, y, index;

  for(y=0; y<height; y++)
  {
    for(x=0; x<width; x++)
    {
      index = originalImage->data[x+originalImage->width*y];
      n[index] ++;
    }
  }
}

/* 大津の方法 */
void
otsu(image_t *resultImage, image_t *originalImage, int n[], int totalPixel, int width, int height){
  int total0, total1, sigma0, sigma1, sigma3;
  double w_0, w_1, mean0, mean1, meanT;
  int i, j, k, l, x, y;
  int interclassVariance = 0;
  double currentInterclassVariance;
  int tonalValue, threshold;

  for(i=0; i<=255; i++){
    /* 初期化 */
    w_0 = 0;
    w_1 = 0;
    total0 = 0;
    total1 = 0;
    mean0 = 0;
    mean1 = 0;
    meanT = 0;
    sigma0 = 0;
    sigma1 = 0;
    sigma3 = 0;
    currentInterclassVariance = 0;

    for(j=0; j<=i ; j++){
      total0 += n[j];
      sigma0 += j*n[j];
    }

    for(k=i+1; k<=255; k++){
        total1 += n[k];
        sigma1 += k*n[k];
    }

    for(l=0; l<=255; l++){
      sigma3 += l*n[l];
    }

    w_0 = (double)total0/totalPixel; /* ω_1 */
    w_1 = (double)total1/totalPixel; /* ω_0 */
    mean0 = (double)sigma0/total0; /* μ_0 */
    mean1 = (double)sigma1/total1; /* μ_1 */
    meanT = (double)sigma3/totalPixel;
    currentInterclassVariance = w_0*pow((mean0-meanT),2)+w_1*pow((mean1-meanT),2);
    if(currentInterclassVariance >= interclassVariance){
      interclassVariance = currentInterclassVariance;
      threshold = i;
    }
  }
  printf("threshold: %d\n", threshold );

  /* 階調値によって0か1をresultImageに書き込む */
  for(y=0; y<height; y++)
  {
    for(x=0; x<width; x++)
    {
      tonalValue = originalImage->data[x+originalImage->width*y];
      resultImage->data[x+resultImage->width*y] = (tonalValue>threshold) ? 255 : 0;
    }
  }
}


/*======================================================================
 * PGM-RAW フォーマットのヘッダ部分の書き込み
 *======================================================================
 *   画像構造体 image_t *ptImage の内容に従って、出力ファイル FILE *fp
 * に、PGM-RAW フォーマットのヘッダ部分を書き込む。
 */
void
writePgmRawHeader(FILE *fp, image_t *ptImage)
{
    /* マジックナンバー(P5) の書き込み */
    if(fputs("P5\n", fp)==EOF)
    {
        goto error;
    }

    /* 画像サイズの書き込み */
    if (fprintf(fp, "%d %d\n", ptImage->width, ptImage->height)==EOF)
    {
        goto error;
    }

    /* 画素値の最大値を書き込む */
    if (fprintf(fp, "%d\n", ptImage->maxValue)==EOF)
    {
        goto error;
    }

    return;

error:
    fputs("Writing PGM-RAW header was failed\n", stderr);
    exit(1);
}


/*======================================================================
 * PGM-RAWフォーマットの画素値データの書き込み
 *======================================================================
 *   画像構造体 image_t *ptImage の内容に従って、出力ファイル FILE *fp
 * に、PGM-RAW フォーマットの画素値データを書き込む
 */
void
writePgmRawBitmapData(FILE *fp, image_t *ptImage)
{
    if( fwrite(ptImage->data, sizeof(unsigned char),
            ptImage->width * ptImage->height, fp)
            != ptImage->width * ptImage->height )
    {
        /* エラー */
        fputs("Writing PGM-RAW bitmap data was failed\n", stderr);
        exit(1);
    }
}

/*
 * メイン
 */
int main(int argc, char **argv)
{
    image_t originalImage, resultImage;
    FILE *infp, *outfp;
    int typeOfFilter, typeOfEquation;
    int n[256];
    int totalPixel, i;
    int threshold;
    int width, height;

    for(i=0; i<=255; i++){
      n[i] = 0;
    }

    /* 引数の解析 */
    parseArg(argc, argv, &infp, &outfp, &typeOfFilter, &typeOfEquation);

    /* 元画像の画像ファイルのヘッダ部分を読み込み、画像構造体を初期化 */
    /* する */
    readPgmRawHeader(infp, &originalImage);

    /* 元画像の画像ファイルのビットマップデータを読み込む */
    readPgmRawBitmapData(infp, &originalImage);

    /* 結果画像の画像構造体を初期化する。画素数、階調数は元画像と同じ */
    initImage(&resultImage, originalImage.width, originalImage.height,
            originalImage.maxValue);

    /* originalImage と resultImage のサイズが違う場合は、共通部分のみ */
    /* を処理する。*/
    width = min(originalImage.width, resultImage.width);
    height = min(originalImage.height, resultImage.height);

    /* 全画素数 */
    totalPixel = width*height;

    /* コマンドライン引数で指定されたフィルタでフィルタリングを行う */
    /* 1: Robertsフィルタ 2: Prewittフィルタ 3: Sobelフィルタ */
    if (typeOfFilter==0){
      filteringImage(&resultImage, &originalImage, width, height);
    }else if(typeOfFilter==1){
      roberts(&resultImage, &originalImage, width, height);
    }else if(typeOfFilter==2 | typeOfFilter==3){
      prewittOrSobel(&resultImage, &originalImage, &typeOfFilter, &typeOfEquation, width, height);
    }

    /* 2値化する */
    /* 各階調値の画素の数を数える */
    //calcTotalPixelsInClass(&originalImage, n, width, height);

    /* 大津の方法で2値化する */
    //otsu(&resultImage, &originalImage, n, totalPixel, width, height);

    /* 画像ファイルのヘッダ部分の書き込み */
    writePgmRawHeader(outfp, &resultImage);

    /* 画像ファイルのビットマップデータの書き込み */
    writePgmRawBitmapData(outfp, &resultImage);

    return 0;
}
