/**********************************************************************
                          $B%M%,%]%8H?E>%U%#%k%?(B
                               sample.c

                          '94/09/01 by K.Miura
 **********************************************************************/

#include <stdio.h>
#include <stdlib.h>

/*
 * $B%^%/%mDj5A(B
 */
#define min(A, B) ((A)<(B) ? (A) : (B))
#define max(A, B) ((A)>(B) ? (A) : (B))

/*
 * $B2hA|9=B$BN$NDj5A(B
 */
typedef struct
{
    int width;              /* $B2hA|$N2#J}8~$N2hAG?t(B */
    int height;             /* $B2hA|$N=DJ}8~$N2hAG?t(B */
    int maxValue;           /* $B2hAG$NCM(B($BL@$k$5(B)$B$N:GBgCM(B */
    unsigned char *data;    /* $B2hA|$N2hAGCM%G!<%?$r3JG<$9$kNN0h$r;X$9(B */
                            /* $B%]%$%s%?(B */
} image_t;


/*======================================================================
 * $B$3$N%W%m%0%i%`$KM?$($i$l$?0z?t$N2r@O(B
 *======================================================================
 */
void
parseArg(int argc, char **argv, FILE **infp, FILE **outfp)
{     
    FILE *fp;

    /* $B0z?t$N8D?t$r%A%'%C%/(B */
    if (argc!=3)
    {
        goto usage;
    }

    *infp = fopen(argv[1], "rb"); /* $BF~NO2hA|%U%!%$%k$r%P%$%J%j%b!<%I$G(B */
                                /* $B%*!<%W%s(B */

    if (*infp==NULL)		/* $B%*!<%W%s$G$-$J$$;~$O%(%i!<(B */
    {
        fputs("Opening the input file was failend\n", stderr);
        goto usage;
    }

    *outfp = fopen(argv[2], "wb"); /* $B=PNO2hA|%U%!%$%k$r%P%$%J%j%b!<%I$G(B */
                                /* $B%*!<%W%s(B */

    if (*outfp==NULL)		/* $B%*!<%W%s$G$-$J$$;~$O%(%i!<(B */
    {
        fputs("Opening the output file was failend\n", stderr);
        goto usage;
    }

    return;

/* $B$3$N%W%m%0%i%`$N;H$$J}$N@bL@(B */
usage:
    fprintf(stderr, "usage : %s <input pgm file> <output pgm file>\n", argv[0]);
    exit(1);
}


/*======================================================================
 * $B2hA|9=B$BN$N=i4|2=(B
 *======================================================================
 * $B2hA|9=B$BN(B image_t *ptImage $B$N2hAG?t(B(width $B!_(B height)$B!"3,D4?t(B
 * (maxValue)$B$r@_Dj$7!"2hAGCM%G!<%?$r3JG<$9$k$N$KI,MW$J%a%b%jNN0h$r3N(B
 * $BJ]$9$k!#(B 
 */
void
initImage(image_t *ptImage, int width, int height, int maxValue)
{
    ptImage->width = width;
    ptImage->height = height;
    ptImage->maxValue = maxValue;

    /* $B%a%b%jNN0h$N3NJ](B */
    ptImage->data = (unsigned char *)malloc((size_t)(width * height));

    if (ptImage->data==NULL)    /* $B%a%b%j3NJ]$,$G$-$J$+$C$?;~$O%(%i!<(B */
    {
        fputs("out of memory\n", stderr);
        exit(1);
    }
}


/*======================================================================
 * $BJ8;zNs0l9TFI$_9~$_4X?t(B
 *======================================================================
 *   FILE *fp $B$+$i!"2~9TJ8;z(B'\n'$B$,I=$l$k$^$GJ8;z$rFI$_9~$s$G!"(Bchar$B7?$N(B
 * $B%a%b%jNN0h(B char *buf $B$K3JG<$9$k!#(B1$B9T$ND9$5$,(B n $BJ8;z0J>e$N>l9g$O!"@h(B
 * $BF,$+$i(B n-1 $BJ8;z$@$1$rFI$_9~$`!#(B
 *   $BFI$_9~$s$@J8;zNs$N@hF,$,(B '#' $B$N>l9g$O!"$5$i$K<!$N9T$rFI$_9~$`!#(B
 *   $B@5>o$KFI$_9~$^$l$?>l9g$O!"%]%$%s%?(B buf $B$rJV$7!"%(%i!<$d(B EOF (End
 * Of File) $B$N>l9g$O(B NULL $B$rJV$9!#(B
 */
char *
readOneLine(char *buf, int n, FILE *fp)
{
    char *fgetsResult;

    do
    {
        fgetsResult = fgets(buf, n, fp);
    } while(fgetsResult!=NULL && buf[0]=='#');
            /* $B%(%i!<$d(B EOF $B$G$O$J$/!"$+$D!"@hF,$,(B '#' $B$N;~$O!"<!$N9T(B */
            /* $B$rFI$_9~$`(B */

    return fgetsResult;
}   


/*======================================================================
 * PGM-RAW $B%U%)!<%^%C%H$N%X%C%@ItJ,$NFI$_9~$_$H2hA|9=B$BN$N=i4|2=(B
 *======================================================================
 *   PGM-RAW $B%U%)!<%^%C%H$N2hA|%G!<%?%U%!%$%k(B FILE *fp $B$+$i!"%X%C%@It(B
 * $BJ,$rFI$_9~$s$G!"$=$N2hA|$N2hAG?t!"3,D4?t$rD4$Y!"$=$N>pJs$K=>$C$F!"(B
 * $B2hA|9=B$BN(B image_t *ptImage $B$r=i4|2=$9$k!#(B
 *   $B2hAGCM%G!<%?$r3JG<$9$k%a%b%jNN0h$b3NJ]$7!"$3$NNN0h$N@hF,$r;X$9%](B
 * $B%$%s%?$r(B ptImage->data $B$K3JG<$9$k!#(B
 *
 * !! $BCm0U(B !!
 *   $B$3$N4X?t$O!"$[$H$s$I$N>l9g!"@5$7$/F0:n$9$k$,!"(BPGM-RAW$B%U%)!<%^%C%H(B
 * $B$N@53N$JDj5A$K$O=>$C$F$*$i$:!"@5$7$$(BPGM-RAW$B%U%)!<%^%C%H$N%U%!%$%k$K(B
 * $BBP$7$F!"IT@5$JF0:n$r$9$k2DG=@-$,$"$k!#$J$k$Y$/!"K\4X?t$r$=$N$^$^;H(B
 * $BMQ$9$k$N$G$O$J$/!"@5$7$/=q$-D>$7$FMxMQ$;$h!#(B
 */
void
readPgmRawHeader(FILE *fp, image_t *ptImage)
{
    int width, height, maxValue;
    char buf[128];

    /* $B%^%8%C%/%J%s%P!<(B(P5) $B$N3NG'(B */
    if(readOneLine(buf, 128, fp)==NULL)
    {
        goto error;
    }
    if (buf[0]!='P' || buf[1]!='5')
    {
        goto error;
    }

    /* $B2hA|%5%$%:$NFI$_9~$_(B */
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

    /* $B:GBg2hAGCM$NFI$_9~$_(B */
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

    /* $B2hA|9=B$BN$N=i4|2=(B */
    initImage(ptImage, width, height, maxValue);

    return;

/* $B%(%i!<=hM}(B */
error:
    fputs("Reading PGM-RAW header was failed\n", stderr);
    exit(1);
}
     

/*======================================================================
 * PGM-RAW$B%U%)!<%^%C%H$N2hAGCM%G!<%?$NFI$_9~$_(B
 *======================================================================
 *   $BF~NO%U%!%$%k(B FILE *fp $B$+$iAm2hAG?tJ,$N2hAGCM%G!<%?$rFI$_9~$s$G!"(B
 * $B2hA|9=B$BN(B image_t *ptImage $B$N(B data $B%a%s%P!<$,;X$9NN0h$K3JG<$9$k(B
 */
void
readPgmRawBitmapData(FILE *fp, image_t *ptImage)
{
    if( fread(ptImage->data, sizeof(unsigned char),
            ptImage->width * ptImage->height, fp)
            != ptImage->width * ptImage->height )
    {
        /* $B%(%i!<(B */
        fputs("Reading PGM-RAW bitmap data was failed\n", stderr);
        exit(1);
    }
}


/*======================================================================
 * $B%U%#%k%?%j%s%0(B($B%M%,%]%8H?E>(B)
 *======================================================================
 *   $B2hA|9=B$BN(B image_t *originalImage $B$N2hA|$r%U%#%k%?%j%s%0(B($B%M%,%]%8(B
 * $BH?E>(B)$B$7$F!"(Bimage_t *resultImage $B$K3JG<$9$k(B
 */
void
filteringImage(image_t *resultImage, image_t *originalImage)
{
    int     x, y;
    int     width, height;

    /* originalImage $B$H(B resultImage $B$N%5%$%:$,0c$&>l9g$O!"6&DLItJ,$N$_(B */
    /* $B$r=hM}$9$k!#(B*/
    width = min(originalImage->width, resultImage->width);
    height = min(originalImage->height, resultImage->height);

    for(y=0; y<height; y++)
    {
        for(x=0; x<width; x++)
        {
            resultImage->data[x+resultImage->width*y]
                    = ( originalImage->maxValue
                    -originalImage->data[x+originalImage->width*y] )
                    *resultImage->maxValue/originalImage->maxValue;
        }
    }
}

/*======================================================================
 * PGM-RAW $B%U%)!<%^%C%H$N%X%C%@ItJ,$N=q$-9~$_(B
 *======================================================================
 *   $B2hA|9=B$BN(B image_t *ptImage $B$NFbMF$K=>$C$F!"=PNO%U%!%$%k(B FILE *fp
 * $B$K!"(BPGM-RAW $B%U%)!<%^%C%H$N%X%C%@ItJ,$r=q$-9~$`!#(B
 */
void
writePgmRawHeader(FILE *fp, image_t *ptImage)
{
    /* $B%^%8%C%/%J%s%P!<(B(P5) $B$N=q$-9~$_(B */
    if(fputs("P5\n", fp)==EOF)
    {
        goto error;
    }

    /* $B2hA|%5%$%:$N=q$-9~$_(B */
    if (fprintf(fp, "%d %d\n", ptImage->width, ptImage->height)==EOF)
    {
        goto error;
    }

    /* $B2hAGCM$N:GBgCM$r=q$-9~$`(B */
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
 * PGM-RAW$B%U%)!<%^%C%H$N2hAGCM%G!<%?$N=q$-9~$_(B
 *======================================================================
 *   $B2hA|9=B$BN(B image_t *ptImage $B$NFbMF$K=>$C$F!"=PNO%U%!%$%k(B FILE *fp
 * $B$K!"(BPGM-RAW $B%U%)!<%^%C%H$N2hAGCM%G!<%?$r=q$-9~$`(B
 */
void
writePgmRawBitmapData(FILE *fp, image_t *ptImage)
{
    if( fwrite(ptImage->data, sizeof(unsigned char),
            ptImage->width * ptImage->height, fp)
            != ptImage->width * ptImage->height )
    {
        /* $B%(%i!<(B */
        fputs("Writing PGM-RAW bitmap data was failed\n", stderr);
        exit(1);
    }
}
 

/*
 * $B%a%$%s(B
 */
int
main(int argc, char **argv)
{
    image_t originalImage, resultImage;
    FILE *infp, *outfp;
  
    /* $B0z?t$N2r@O(B */
    parseArg(argc, argv, &infp, &outfp);

    /* $B852hA|$N2hA|%U%!%$%k$N%X%C%@ItJ,$rFI$_9~$_!"2hA|9=B$BN$r=i4|2=(B */
    /* $B$9$k(B */
    readPgmRawHeader(infp, &originalImage);

    /* $B852hA|$N2hA|%U%!%$%k$N%S%C%H%^%C%W%G!<%?$rFI$_9~$`(B */
    readPgmRawBitmapData(infp, &originalImage);

    /* $B7k2L2hA|$N2hA|9=B$BN$r=i4|2=$9$k!#2hAG?t!"3,D4?t$O852hA|$HF1$8(B */
    initImage(&resultImage, originalImage.width, originalImage.height,
            originalImage.maxValue);

    /* $B%U%#%k%?%j%s%0(B */
    filteringImage(&resultImage, &originalImage);

    /* $B2hA|%U%!%$%k$N%X%C%@ItJ,$N=q$-9~$_(B */
    writePgmRawHeader(outfp, &resultImage);

    /* $B2hA|%U%!%$%k$N%S%C%H%^%C%W%G!<%?$N=q$-9~$_(B */
    writePgmRawBitmapData(outfp, &resultImage);

    return 0;
}
