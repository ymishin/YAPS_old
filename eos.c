/**
 * Copyright (c) 2005,2010 Yury Mishin <yury.mishin@gmail.com>
 * See the file COPYING for copying permission.
 *
 * $Id$
 */

#include <math.h>
#include "common.h"
#include "calc.h"
#include "eos.h"

/**********************************************************/

/* Batchelor EOS */
static void CalcPressByBatchelorEOS( void);

/* Desbrun EOS */
static void CalcPressByDesbrunEOS( void);

/**********************************************************/

/* All implemented equations of state */
struct StateEquation StateEquations[] =
{
    /* EOS suggested by Batchelor */
    "BATCHELOR", CalcPressByBatchelorEOS,
    /* EOS suggested by Desbrun   */
    "DESBRUN",   CalcPressByDesbrunEOS,
};

/* The number of all the equations of state */
int StateEquationsNum = sizeof(StateEquations) / 
                        sizeof(StateEquations[0]);

/**********************************************************/

/**
 * Calculate pressures at particles' positions using
 * the equation of state suggested by Batchelor: 
 * G.K.Batchelor, An Introduction to Fluid Dynamics, 
 * Cambridge Univ.Press, 2000.
 * In fact, the function implements a modified version
 * of this equation suggested by Monaghan:
 * J.J.Monaghan, Simulating Free Surface Flows with SPH, 
 * J.Comput.Phys., 110, 399-406, 1994.
 */
static void
CalcPressByBatchelorEOS( void)
{
    float B;
    float n;
    int i;

    /* Monaghan'94 */
    n = 7.0f;
    B = Density0 * SOS * SOS / n;
    
    /* Calculate pressures for all particles */
    for ( i = 0; i < ParticlesNumber; i++ )
    {
        Particles[i].Press = B * (pow( Particles[i].Dens / Density0, n) - 1.0f);
    }

    return;
} /* CalcPressByBatchelorEOS */

/**********************************************************/

/**
 * Calculate pressures at particles' positions using
 * the equation of state suggested by Desbrun and Gascuel:
 * M.Desbrun and M.Gascuel, Smoothed Particles: A new paradigm 
 * for animating highly deformable bodies, Proceedings of 6th 
 * Eurographics Workshop on Animation and Simulation, 61-76, 1996.
 */
static void
CalcPressByDesbrunEOS( void)
{
    float k;
    int i;
    
    /* Stiffness parameter */
    k = 30.0f;

    /* Calculate pressures for all particles */
    for ( i = 0; i < ParticlesNumber; i++ )
    {
        Particles[i].Press = k * ( Particles[i].Dens - Density0);
    }

    return;
} /* CalcPressByDesbrunEOS */
