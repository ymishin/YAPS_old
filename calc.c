/**
 * Copyright (c) 2005,2010 Yury Mishin <yury.mishin@gmail.com>
 * See the file COPYING for copying permission.
 *
 * $Id$
 */

#include <string.h>
#include <math.h>
#include <omp.h>
#include "common.h"
#include "vector.h"
#include "kernel.h"
#include "eos.h"
#include "calc.h"

/**********************************************************/

/* The function to calculate the particles' pressures */
static void  (*CalcPressByEOS)   ( void);

/* The function to calculate the kernel's gradient */
static int   (*GetGradKernel)    ( float *Grad, float *Rij);

/* 'leap-frog' integration scheme */
static void  LeapfrogIntegration ( void);

/**********************************************************/

/* External force field */
static float ExternalForce[] = { 0.0f, -0.00981f, 0.0f };

/* D factor to calculate repulsive Lennard-Jones forces */
static float LenJonD  = 10.0f;

/* P1 power to calculate repulsive Lennard-Jones forces */
static float LenJonP1 = 4.0f;

/* P2 power to calculate repulsive Lennard-Jones forces */
static float LenJonP2 = 2.0f;

/**********************************************************/

/* Equation of state to calculate pressures */
char  EOSType[20];

/* Kernel to use in the calculations */
char  KernelType[20];

/* Kernel smoothing length */
float SmoothR;

/* Rest density */
float Density0;

/* Speed of sound */
float SOS;

/* Alpha factor to calculate viscosity */
float ViscAlpha;

/* Beta factor to calculate viscosity */
float ViscBeta;

/* Time step of integration */
float TimeStep;

/**********************************************************/

/**
 * Initialize calculation module - choose appropriate 
 * kernel(s), equation of state, etc. according to 
 * parameters in the scene description file.
 */
void
InitCalc( void)
{
    int i;

    /* Choose the kernel for calculations */
    for ( i = 0; i < KernelsNum; i++ )
    {
        /* Search for the required kernel */
        if ( strcmp( KernelType, Kernels[i].Name) )
            continue;
        /* Initialize the kernel */
        Kernels[i].Init();
        /* The function to calculate the kernel's gradient */
        GetGradKernel = Kernels[i].GetGrad;
        break;
    }

    /* Choose the equation of state for calculations */
    for ( i = 0; i < StateEquationsNum; i++ )
    {
        /* Search for the required EOS */
        if ( strcmp(EOSType, StateEquations[i].Name) )
            continue;
        /* The function to calculate the particles' pressures */
        CalcPressByEOS = StateEquations[i].CalcPress;
        break;
    }
    
    return;
} /* InitCalc */

/**********************************************************/

/**
 * Do one calculation step.
 */
void
DoCalcStep( void)
{
    float GradKernel[3];
    float PressTerm;
    float ViscTerm;
    float ViscNu;
    float Vij[3];
    float Rij[3];
    float tmp1, tmp2;
    int   i, j, d;
    
    /* Nu factor to calculate viscosity */
    ViscNu = 0.01f * SmoothR * SmoothR;
    
    /* Calculate the particles' pressures */
    CalcPressByEOS();

    /* Calculate the rates of change of velocities and the 
     * rates of change of densities for all the particles
     * J.J.Monaghan, Simulating Free Surface Flows with SPH, 
     * J.Comput.Phys., 110, 399-406, 1994.
     */
#pragma omp parallel for schedule(dynamic,50) private(GradKernel,PressTerm,ViscTerm,Vij,Rij,tmp1,tmp2,j,d)
    for ( i = 0; i < ParticlesNumber; i++ )
    {
        /* Take into account the external force field */
        memcpy( Particles[i].Accel, ExternalForce, sizeof(ExternalForce));
        
        Particles[i].DervDens = 0.0f;

        /* Calculate forces between smoothing particles 
         * and update the rate of change of the density */
        for ( j = 0; j < ParticlesNumber; j++ )
        {
            if ( j == i )
                continue;

            VectorSubstraction( Rij, Particles[i].Pos, Particles[j].Pos);

            /* Get the kernel's gradient at the point Rij */
            if ( GetGradKernel( GradKernel, Rij) )
                continue;
            
            /* Take into account the viscocity of the medium */
            VectorSubstraction( Vij, Particles[i].Vel, Particles[j].Vel);
            tmp1 = VectorInnerproduct( Rij, Vij);
            if ( tmp1 < 0.0f )
            {
                tmp2 = VectorInnerproduct( Rij, Rij);
                tmp1 = SmoothR * tmp1 / (tmp2 + ViscNu);
                ViscTerm = 2.0f * tmp1 * (-ViscAlpha * SOS + ViscBeta * tmp1) / 
                          (Particles[i].Dens + Particles[j].Dens);
            }
            else
            {
                ViscTerm = 0.0f;
            }

            /* Take into account the difference of the particles' pressures */
            PressTerm = Particles[i].Press / (Particles[i].Dens * Particles[i].Dens) +
                        Particles[j].Press / (Particles[j].Dens * Particles[j].Dens);
            
            /* Update the acceleration of the particle */
            tmp1 = Particles[j].Mass * (PressTerm + ViscTerm);
            for ( d = 0; d < Dimension; d++ )
                Particles[i].Accel[d] -= tmp1 * GradKernel[d];

            /* Update the rate of change of the density for the particle */
            tmp1 = VectorInnerproduct( Vij, GradKernel);
            Particles[i].DervDens += Particles[j].Mass * tmp1;
        }

        /* Calculate the Lennard-Jones forces between 
         * the particle and the boundary particles */
        for ( j = 0; j < BParticlesNumber; j++ )
        {
            VectorSubstraction( Rij, Particles[i].Pos, BParticles[j].Pos);
            tmp1 = VectorInnerproduct( Rij, Rij);
            tmp2 = ParticlesDistrib / sqrt( tmp1);
            /* Only repulsive forces are taken into account */
            if ( tmp2 > 1.0f )
            {
                tmp1 = (pow( tmp2, LenJonP1) - pow( tmp2, LenJonP2)) * 
                       LenJonD / tmp1;
                for ( d = 0; d < Dimension; d++ )
                    Particles[i].Accel[d] += Rij[d] * tmp1;
            }
        }
    }

    /* Time integration */
    LeapfrogIntegration();
    
    return;
} /* DoCalcStep */

/**********************************************************/

/**
 * 'leap-frog' integration scheme
 * M.P.Allen and D.J.Tildesley, Computer Simulation 
 * of Liquids, Oxford Univ.Press, 1987.
 */
static void
LeapfrogIntegration( void)
{
    int i;
    int d;
    
    /* Calculate new positions, velocities and densities for all the particles */
    for ( i = 0; i < ParticlesNumber; i++ )
    {
        for ( d = 0; d < Dimension; d++ )
        {
            /* New interval velocity (t+dt/2) */
            Particles[i].IvalVel[d] += Particles[i].Accel[d] * TimeStep;
            /* New position (t+dt) */
            Particles[i].Pos[d] += Particles[i].IvalVel[d] * TimeStep;
            /* New velocity (t+dt) */
            Particles[i].Vel[d] = Particles[i].IvalVel[d] + 
                                  Particles[i].Accel[d] * TimeStep / 2.0f;
        }
        /* New interval density (t+dt/2) */
        Particles[i].IvalDens += Particles[i].DervDens * TimeStep;
        /* New density (t+dt) */
        Particles[i].Dens = Particles[i].IvalDens +
                            Particles[i].DervDens * TimeStep / 2.0f;
    }
    
    return;
} /* LeapfrogIntegration */
