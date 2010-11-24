/**
 * Copyright (c) 2005,2010 Yury Mishin <yury.mishin@gmail.com>
 * See the file COPYING for copying permission.
 *
 * $Id$
 */

#include <math.h>
#include "common.h"
#include "vector.h"

/**********************************************************/

/**
 * The function adds vector <Vec2> to vector <Vec1> and 
 * stores the result in vector <ResVec>: <ResVec> = <Vec1> + <Vec2>.
 */
void
VectorAddition( float *ResVec,   /* Resulting vector */
                float *Vec1,     /* Vector 1 */
                float *Vec2)     /* Vector 2 */
{
    int d;
    
    for ( d = 0; d < Dimension; d++ )
        ResVec[d] = Vec1[d] + Vec2[d];
    
    return;
} /* VectorAddition */

/**********************************************************/

/** 
 * The function substracts vector <Vec2> from vector <Vec1> and 
 * stores the result in vector <ResVec>: <ResVec> = <Vec1> - <Vec2>.
 */
void
VectorSubstraction( float *ResVec,   /* Resulting vector */
                    float *Vec1,     /* Vector 1 */
                    float *Vec2)     /* Vector 2 */
{
    int d;
    
    for ( d = 0; d < Dimension; d++ )
        ResVec[d] = Vec1[d] - Vec2[d];
    
    return;
} /* VectorSubstraction */

/**********************************************************/

/**
 * The function computes and returns the innerproduct 
 * of two vectors - (<Vec1>, <Vec2>).
 */
float
VectorInnerproduct( float *Vec1,   /* Vector 1 */
                    float *Vec2)   /* Vector 2 */
{
    float Res;
    int d;

    Res = 0;
    for ( d = 0; d < Dimension; d++ )
        Res += Vec1[d] * Vec2[d];

    return Res;
} /* VectorInnerproduct */

/**********************************************************/

/**
 * The function computes and returns 
 * the norm of the vector - |<Vec>|.
 */
float
VectorNorm( float *Vec)   /* Vector */
{
    float Res;
    int d;

    Res = 0;
    for ( d = 0; d < Dimension; d++ )
        Res += Vec[d] * Vec[d];

    Res = sqrt( Res );

    return Res;
} /* VectorNorm */
