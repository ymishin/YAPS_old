/**
 * Copyright (c) 2005,2010 Yury Mishin <yury.mishin@gmail.com>
 * See the file COPYING for copying permission.
 *
 * $Id$
 */

#ifndef YAPS_VECTOR_H
#define YAPS_VECTOR_H

/**********************************************************/

/* <ResVec> = <Vec1> + <Vec2>   */
extern void  VectorAddition     ( float *ResVec, 
                                  float *Vec1, 
                                  float *Vec2);

/* <ResVec> = <Vec1> - <Vec2>   */
extern void  VectorSubstraction ( float *ResVec, 
                                  float *Vec1, 
                                  float *Vec2);

/* (<Vec1>, <Vec2>) is returned */
extern float VectorInnerproduct ( float *Vec1, 
                                  float *Vec2);

/* |<Vec>| is returned          */
extern float VectorNorm         ( float *Vec);

/**********************************************************/

#endif /* YAPS_VECTOR_H */
