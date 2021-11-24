/*==========================================================================================*/
/* FUNÇÕES PARA O PROBLEMA DO WII SENSOR BAR                                                */
/*------------------------------------------------------------------------------------------*/
/* Autores: Nicolas Barbieri Sousa, Rômulo Barbosa da Silva e Sebastião Araújo Neto         */
/*==========================================================================================*/
/** Funções que permitem binarizar (tornar a imagem bicolor para filtrar o ruído), rotular
 *  (identificar aglomerados de pixels de mesma cor), encontrar o angulo entre dois pontos
 *  no plano e detectar o ângulo entre os centros dos dois maiores aglomerados da imagem. **/
/*==========================================================================================*/

#include "projeto5.h"
#include <stdlib.h>
#include <math.h>

#define LIMIAR 128 // Valor mínimo para que um pixel na imagem assuma o valor 255

/*==========================================================================================*/
/* Funções internas do módulo. */

void binariza (Imagem1C* img, int limiar);
void rotula (int** rotulada, int altura, int largura, int rotulo, int l, int k);
double encontraAngulo (Coordenada l, Coordenada r);

/*==========================================================================================*/
/** Função central do detector, retorna o ângulo entre os centros.
 *
 * Parâmetros: Imagem1C* img: a imagem que será analisada.
 *             Coordenada* l: ponteiro para a variável em que serão armazenadas
 *                 as coordenadas do centro do círculo esquerdo.
 *             Coordenada* r: ponteiro para a variável em que serão armazenadas
 *                 as coordenadas do centro do círculo direito.
 *
 * Valor de retorno: o ângulo, em radianos, entre os centros dos aglomerados de mesma cor  */

double detectaSensorBar (Imagem1C* img, Coordenada* l, Coordenada* r)
{
    int rotulo = -1;

    binariza(img, LIMIAR);

    /* ALOCAÇÃO DA MATRIZ DE RÓTULOS */
    int** matriz_rotulos = (int**) malloc (img->altura * sizeof(int*));

    int i, j;
    for (i = 0; i < img->altura; i++)
    {
        matriz_rotulos[i] = (int *) malloc (img->largura * sizeof(int));
    }

    /* CÓPIA DA IMAGEM (char) PARA A MATRIZ (int) */
    for (i = 0; i < img->altura; i++)
    {
        for (j = 0; j < img->largura; j++)
        {
            matriz_rotulos[i][j] = img->dados[i][j];
        }
    }

    /* ROTULAÇÃO */
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

    /* ALOCAÇÃO, INICIALIZAÇÃO E PREENCHIMENTO DO VETOR ACUMULADOR DE RÓTULOS */
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

    int blob1_pixels = 0, // Número de pixels no blob1
        blob2_pixels = 0, // Número de pixels no blob2
        pilha_i = 0, // Soma dos números das linhas do blob1
        pilha_j = 0, // Soma dos números das colunas do blob1
        pilha_k = 0, // Soma dos números das linhas do blob2
        pilha_l = 0, // Soma dos números das colunas do blob2
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
            blob1 = i; // Rótulo do maior blob = i
            blob1_pixels = vetor[i]; // Número de pixels do maior blob
        }
        else if (vetor[i] > blob2_pixels)
        {
            blob2 = i; // Rótulo do segundo maior blob
            blob2_pixels = vetor[i]; // Número de pixels do segundo maior blob
        }
    }

    /* CÁLCULOS PARA A MÉDIA PONDERADA */
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

    /* ATRIBUIÇÃO ÀS COORDENADAS */
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

    /* LIBERAÇÃO DE MEMÓRIA */
    free(matriz_rotulos);
    free(vetor);

    return encontraAngulo(left, right);
}

/*------------------------------------------------------------------------------------------*/
/** Função que torna a imagem bicolor para filtrar o ruído.
 *
 * Parâmetros: Imagem1C* img: a imagem que será processada.
 *             int limiar: o valor mínimo (0-255) que um pixel deve ter
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
/** Função que rotula os aglomerados de pixels de mesma cor.
 *
 * Parâmetros: int** rotulada: imagem rotulada.
 *             int altura: altura, em pixels, da imagem que será rotulada.
 *             int largura: largura, em pixels, da imagem que será rotulada.
 *             int rotulo: rótulo que será aplicado ao pixel e aos adjacentes de mesma cor.
 *             int l: abscissa do pixel.
 *             int k: ordenada do pixel.
 *
 * Valor de retorno: nenhum. */

void rotula (int** rotulada, int altura, int largura, int rotulo, int l, int k)
{
    rotulada[l][k] = rotulo; // Insere o rótulo na primeira posição branca

    char flag = 1;
    int i, j;

    /* Enquanto houver modificações na matriz, o loop será repetido */
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
/** Função que rotula os aglomerados de pixels de mesma cor.
 *
 * Parâmetros: Coordenada l: ponto mais à esquerda (A).
 *             Coordenada r: ponto mais à direita (B).
 *
 * Valor de retorno: ângulo, em radianos, entre os pontos A e B num plano cartesiano. */

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
