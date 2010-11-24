/**
 * Copyright (c) 2005,2010 Yury Mishin <yury.mishin@gmail.com>
 * See the file COPYING for copying permission.
 *
 * $Id$
 */

#ifndef YAPS_COMMON_H
#define YAPS_COMMON_H

/**********************************************************/

/* Dimension of the simulation */
int Dimension;

/**********************************************************/

/* Initial particle distribution */
float ParticlesDistrib;

/* Smoothing particle */
struct Particle
{
    float Pos[3];        /* Particle's position (x,y,z) */
    float Vel[3];        /* Particle's velocity vector (Vx,Vy,Vz) */
    float IvalVel[3];    /* Velocity vector (Vx,Vy,Vz) at (t-dt/2) */
    float Accel[3];      /* Acceleration (Ax,Ay,Az) of the particle */
    float Dens;          /* Density at the location of the partile */
    float IvalDens;      /* Density at (t-dt/2) */
    float DervDens;      /* The rate of change of the density (dro/dt) */
    float Press;         /* Pressure at the location of the particle */
    float Mass;          /* The mass carried by the particle */
};

/* The array of all smoothing particles in the scene */
struct Particle *Particles;

/* Number of all smoothing particles in the scene */
int ParticlesNumber;

/**********************************************************/

/* Initial boundary particle distribution */
float BParticlesDistrib;

/* Boundary particle */
struct BParticle
{
    float Pos[3];        /* Boundary particle's position (x,y,z) */
};

/* The array of all boundary particles in the scene */
struct BParticle *BParticles;

/* Number of all boundary particles in the scene */
int BParticlesNumber;

/**********************************************************/

/* Segment-obstacle (is used in 2D simulation) */
struct ObstacleSegment
{
    float Vrtx1[3];  /* Vertex 1 */
    float Vrtx2[3];  /* Vertex 2 */
};

/* Triangle-obstacle (is used in 3D simulation) */
struct ObstacleTriangle
{
    float Vrtx1[3];      /* Vertex 1 */
    float Vrtx2[3];      /* Vertex 2 */
    float Vrtx3[3];      /* Vertex 3 */
};

/* The array of all the obstacles in the scene */
struct Obstacle *Obstacles;

/* Number of all the obstacles in the scene */
int ObstaclesNumber;

/**********************************************************/

#endif /* YAPS_COMMON_H */
