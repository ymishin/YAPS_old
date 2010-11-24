/**
 * Copyright (c) 2005,2010 Yury Mishin <yury.mishin@gmail.com>
 * See the file COPYING for copying permission.
 *
 * $Id$
 */

#include "common.h"
#include "calc.h"
#include "vector.h"
#include "kernel.h"

/**********************************************************/

#define PI 3.1415926535f

/**********************************************************/

/* Cubic spline kernel */
static void InitWspline    ( void);
static int  GetGradWspline ( float *Grad, float *Rij);

/* Spiky kernel */
static void InitWspiky     ( void);
static int  GetGradWspiky  ( float *Grad, float *Rij);

/**********************************************************/

/* All implemented kernels */
struct Kernel Kernels[] =
{
    /* Cubic spline kernel */
    "SPLINE", InitWspline, GetGradWspline,
    /* Spiky kernel        */
    "SPIKY",  InitWspiky,  GetGradWspiky,
};

/* The number of all the kernels */
int KernelsNum = sizeof(Kernels) / sizeof(Kernels[0]);

/**********************************************************/

/**************************************************
 * Cubic spline kernel                            *
 * J.J.Monaghan, Smoothed Particle Hydrodynamics, *
 * Annu.Rev.Astron.Astrophys., 30, 543-574, 1992. *
 **************************************************/

/* Factor to calculate the kernel's gradient */
static float GradFactorWspline;

/**
 * Initialize the kernel.
 */
static void
InitWspline( void)
{
    float NormFactor;
    
    /* Kernel's normalization factor */
    if ( Dimension == 2 )
        NormFactor = 10.0f / (7.0f * PI * SmoothR * SmoothR);
    else if ( Dimension == 3 )
        NormFactor = 1.0f / (PI * SmoothR * SmoothR * SmoothR);
    
    /* Factor to calculate the kernel's gradient */
    GradFactorWspline = NormFactor / (SmoothR * SmoothR);

    return;
} /* InitWspline */

/**
 * Calculate the kernel's gradient at the point <Rij> with 
 * respect to Ri, the resulting gradient vector is return 
 * through <Grad>. The function returns -1 if the gradient 
 * vector is equal to zero and 0 if it's meaning.
 */
static int
GetGradWspline( float *Grad,   /* Result (gradient vector) */
                float *Rij)    /* Vector Rij = Ri - Rj */
{
    float s;
    int d;
    
    s = VectorNorm( Rij) / SmoothR;
    
    if ( s > 2.0f )
    {
        return -1;
    }
    else if ( s > 1.0f )
    {
        for ( d = 0; d < Dimension; d++ )
            Grad[d] = GradFactorWspline * Rij[d] * 
                      -0.75f * (2.0f - s) * (2.0f - s) / s;
    }
    else
    {
        for ( d = 0; d < Dimension; d++ )
            Grad[d] = GradFactorWspline * Rij[d] * 
                      (2.25f * s - 3.0f);
    }

    return 0;
} /* GetGradWspline */

/**********************************************************/

/*******************************************************************
 * Spiky kernel                                                    *
 * M.Desbrun and M.Gascuel, Smoothed Particles: A new paradigm     *
 * for animating highly deformable bodies, Proceedings of 6th      *
 * Eurographics Workshop on Animation and Simulation, 61-76, 1996. *
 *******************************************************************/

/* Factor to calculate the kernel's gradient */
static float GradFactorWspiky;

/**
 * Initialize the kernel.
 */
static void
InitWspiky( void)
{
    float NormFactor;
    
    /* Kernel's normalization factor */
    if ( Dimension == 2 )
        NormFactor = 5.0f / (16.0f * PI * SmoothR * SmoothR);
    else if ( Dimension == 3 )
        NormFactor = 15.0f / (64.0f * PI * SmoothR * SmoothR * SmoothR);
    
    /* Factor to calculate the kernel's gradient */
    GradFactorWspiky = NormFactor * (-3.0f / (SmoothR * SmoothR));

    return;
} /* InitWspiky */

/**
 * Calculate the kernel's gradient at the point <Rij> with 
 * respect to Ri, the resulting gradient vector is return 
 * through <Grad>. The function returns -1 if the gradient 
 * vector is equal to zero and 0 if it's meaning.
 */
static int
GetGradWspiky( float *Grad,   /* Result (gradient vector) */
               float *Rij)    /* Vector Rij = Ri - Rj */
{
    float s;
    int d;
    
    s = VectorNorm( Rij) / SmoothR;

    if ( s > 2.0f )
    {
        return -1;
    }
    else
    {
        for ( d = 0; d < Dimension; d++ )
            Grad[d] = GradFactorWspiky * Rij[d] * 
                      (2.0f - s) * (2.0f - s) / s;
    }

    return 0;
} /* GetGradWspiky */
