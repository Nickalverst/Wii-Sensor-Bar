/*==========================================================================================*/
/* FUN��ES PARA O PROBLEMA DO WII SENSOR BAR                                                */
/*------------------------------------------------------------------------------------------*/
/* Autores: Nicolas Barbieri Sousa, R�mulo Barbosa da Silva e Sebasti�o Ara�jo Neto         */
/*==========================================================================================*/
/** Fun��es que permitem binarizar (tornar a imagem bicolor para filtrar o ru�do), rotular
 *  (identificar aglomerados de pixels de mesma cor), encontrar o angulo entre dois pontos
 *  no plano e detectar o �ngulo entre os centros dos dois maiores aglomerados da imagem. **/
/*==========================================================================================*/

#include "projeto5.h"
#include <stdlib.h>
#include <math.h>

#define LIMIAR 128 // Valor m�nimo para que um pixel na imagem assuma o valor 255

/*==========================================================================================*/
/* Fun��es internas do m�dulo. */

void binariza (Imagem1C* img, int limiar);
void rotula (int** rotulada, int altura, int largura, int rotulo, int l, int k);
double encontraAngulo (Coordenada l, Coordenada r);

/*==========================================================================================*/
/** Fun��o central do detector, retorna o �ngulo entre os centros.
 *
 * Par�metros: Imagem1C* img: a imagem que ser� analisada.
 *             Coordenada* l: ponteiro para a vari�vel em que ser�o armazenadas
 *                 as coordenadas do centro do c�rculo esquerdo.
 *             Coordenada* r: ponteiro para a vari�vel em que ser�o armazenadas
 *                 as coordenadas do centro do c�rculo direito.
 *
 * Valor de retorno: o �ngulo, em radianos, entre os centros dos aglomerados de mesma cor  */

double detectaSensorBar (Imagem1C* img, Coordenada* l, Coordenada* r)
{
    int rotulo = -1;

    binariza(img, LIMIAR);

    /* ALOCA��O DA MATRIZ DE R�TULOS */
    int** matriz_rotulos = (int**) malloc (img->altura * sizeof(int*));

    int i, j;
    for (i = 0; i < img->altura; i++)
    {
        matriz_rotulos[i] = (int *) malloc (img->largura * sizeof(int));
    }

    /* C�PIA DA IMAGEM (char) PARA A MATRIZ (int) */
    for (i = 0; i < img->altura; i++)
    {
        for (j = 0; j < img->largura; j++)
        {
            matriz_rotulos[i][j] = img->dados[i][j];
        }
    }

    /* ROTULA��O */
    for (i = 1; i < img->altura - 1; i++)
    {
        for (j = 1; j < img->largura - 1; j++)
        {
            if (matriz_rotulos[i][j] > 0)
            {
                rotula(matriz_rotulos, img->altura, img->largura, rotulo, i, j);
                rotulo--;
            }
        }
    }

    /* ALOCA��O, INICIALIZA��O E PREENCHIMENTO DO VETOR ACUMULADOR DE R�TULOS */
    int* vetor = (int*) malloc (-1 * rotulo * sizeof(int));

    for (i = 0; i < -1 * rotulo; i++)
    {
        vetor[i] = 0;
    }

    for (i = 0; i < img->altura; i++)
    {
        for (j = 0; j < img->largura; j++)
        {
            if (matriz_rotulos[i][j] == 255)
            {
                matriz_rotulos[i][j] = 0;
            }
            vetor[matriz_rotulos[i][j] * -1]++;
        }
    }

    int blob1_pixels = 0, // N�mero de pixels no blob1
        blob2_pixels = 0, // N�mero de pixels no blob2
        pilha_i = 0, // Soma dos n�meros das linhas do blob1
        pilha_j = 0, // Soma dos n�meros das colunas do blob1
        pilha_k = 0, // Soma dos n�meros das linhas do blob2
        pilha_l = 0, // Soma dos n�meros das colunas do blob2
        media_x1 = 0, // Coordenada horizontal do centro do blob1
        media_y1 = 0, // Coordenada vertical do centro do blob1
        media_y2 = 0, // Coordenada vertical do centro do blob2
        media_x2 = 0; // Coordenada horizontal do centro do blob2


    /* BUSCA PELO MAIOR BLOB */
    int blob1 = 0;
    int blob2 = 0; // Em pixels
    for (i = 1; i < -rotulo; i++)
    {
        if (vetor[i] > blob1_pixels)
        {
            blob2 = blob1;
            blob2_pixels = blob1_pixels;
            blob1 = i; // R�tulo do maior blob = i
            blob1_pixels = vetor[i]; // N�mero de pixels do maior blob
        }
        else if (vetor[i] > blob2_pixels)
        {
            blob2 = i; // R�tulo do segundo maior blob
            blob2_pixels = vetor[i]; // N�mero de pixels do segundo maior blob
        }
    }

    /* C�LCULOS PARA A M�DIA PONDERADA */
    for (i = 1; i < img->altura; i++)
    {
        for (j = 1; j < img->largura; j++)
        {
            if (matriz_rotulos[i][j] == -blob1)
            {
                pilha_i += i;
                pilha_j += j;
            }

            if (matriz_rotulos[i][j] == -blob2)
            {
                pilha_k += i;
                pilha_l += j;
            }
        }
    }

    media_y1 = pilha_i / blob1_pixels;
    media_x1 = pilha_j / blob1_pixels;

    media_y2 = pilha_k / blob2_pixels;
    media_x2 = pilha_l / blob2_pixels;

    /* ATRIBUI��O �S COORDENADAS */
    Coordenada left;
    Coordenada right;

    if (media_x2 < media_x1)
    {
        left.x = media_x2;
        left.y = media_y2;
        right.x = media_x1;
        right.y = media_y1;
    } else
    {
        left.x = media_x1;
        left.y = media_y1;
        right.x = media_x2;
        right.y = media_y2;
    }

    *l = left;
    *r = right;

    /* LIBERA��O DE MEM�RIA */
    free(matriz_rotulos);
    free(vetor);

    return encontraAngulo(left, right);
}

/*------------------------------------------------------------------------------------------*/
/** Fun��o que torna a imagem bicolor para filtrar o ru�do.
 *
 * Par�metros: Imagem1C* img: a imagem que ser� processada.
 *             int limiar: o valor m�nimo (0-255) que um pixel deve ter
 *                 para ser separado.
 *
 * Valor de retorno: nenhum. */

void binariza (Imagem1C* img, int limiar)
{
    int i, j;
    for (i = 0; i < img->altura; i++)
    {
        for (j = 0; j < img->largura; j++)
        {
            if ( img->dados[i][j] <= limiar )
            {
                img->dados[i][j] = 0;
            } else
            {
                img->dados[i][j] = 255;
            }
        }
    }
}

/*------------------------------------------------------------------------------------------*/
/** Fun��o que rotula os aglomerados de pixels de mesma cor.
 *
 * Par�metros: int** rotulada: imagem rotulada.
 *             int altura: altura, em pixels, da imagem que ser� rotulada.
 *             int largura: largura, em pixels, da imagem que ser� rotulada.
 *             int rotulo: r�tulo que ser� aplicado ao pixel e aos adjacentes de mesma cor.
 *             int l: abscissa do pixel.
 *             int k: ordenada do pixel.
 *
 * Valor de retorno: nenhum. */

void rotula (int** rotulada, int altura, int largura, int rotulo, int l, int k)
{
    rotulada[l][k] = rotulo; // Insere o r�tulo na primeira posi��o branca

    char flag = 1;
    int i, j;

    /* Enquanto houver modifica��es na matriz, o loop ser� repetido */
    while (flag == 1)
    {
        flag = 0;
        for (i = 1; i < altura - 1; i++)
        {
            for (j = 1; j < largura - 1; j++)
            {
                if (rotulada[i][j - 1] < 0 && rotulada[i][j] == 255) // Pixel oeste
                {
                    rotulada[i][j] = rotulada[i][j - 1];
                    flag = 1;
                }
                else if (rotulada[i - 1][j - 1] < 0 && rotulada[i][j] == 255) // Pixel noroeste
                {
                    rotulada[i][j] = rotulada[i - 1][j - 1];
                    flag = 1;
                }
                else if (rotulada[i - 1][j] < 0 && rotulada[i][j] == 255) // Pixel norte
                {
                    rotulada[i][j] = rotulada[i - 1][j];
                    flag = 1;
                }
                else if (rotulada[i - 1][j + 1] < 0 && rotulada[i][j] == 255) // Pixel nordeste
                {
                    rotulada[i][j] = rotulada[i - 1][j + 1];
                    flag = 1;
                }
                else if (rotulada[i][j + 1] < 0 && rotulada[i][j] == 255) // Pixel leste
                {
                    rotulada[i][j] = rotulada[i][j + 1];
                    flag = 1;
                }
                else if (rotulada[i + 1][j + 1] < 0 && rotulada[i][j] == 255) //Pixel sudeste
                {
                    rotulada[i][j] = rotulada[i + 1][j + 1];
                    flag = 1;
                }
                else if (rotulada[i + 1][j] < 0  && rotulada[i][j] == 255) // Pixel sul
                {
                    rotulada[i][j] = rotulada[i + 1][j];
                    flag = 1;
                }
                else if (rotulada[i + 1][j - 1] < 0  && rotulada[i][j] == 255) // Pixel sudoeste
                {
                    rotulada[i][j] = rotulada[i + 1][j - 1];
                    flag = 1;
                }
            }
        }
    }
}

/*------------------------------------------------------------------------------------------*/
/** Fun��o que rotula os aglomerados de pixels de mesma cor.
 *
 * Par�metros: Coordenada l: ponto mais � esquerda (A).
 *             Coordenada r: ponto mais � direita (B).
 *
 * Valor de retorno: �ngulo, em radianos, entre os pontos A e B num plano cartesiano. */

double encontraAngulo (Coordenada l, Coordenada r)
{
    double x, y;
    double angulo;

    x = abs((l.x) - (r.x));
    y = abs((l.y) - (r.y));
    angulo = atan2(sqrt(y), sqrt(x));

    if (l.y > r.y)
    {
        angulo *= -1;
    }

    return angulo;
}
