/**
 * Copyright (c) 2005,2010 Yury Mishin <yury.mishin@gmail.com>
 * See the file COPYING for copying permission.
 *
 * $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "common.h"
#include "calc.h"
#include "render.h"
#include "vector.h"
#include "scene.h"

/**********************************************************/

/* The name of the file with scene description */
#define SCENE_FILE_NAME     "scene"

/* The maximum length of the file's string (with line feed) */
#define FILE_LINE_LENGTH    1024

/**********************************************************/

/* Section info */
struct Section;

/* Read and process the obstacles section from scene file */ 
static int   ReadObstaclesSection       ( char **Scene, 
                                          struct Section *Info);

/* Read and process the clouds section from scene file */ 
static int   ReadCloudsSection          ( char **Scene, 
                                          struct Section *Info);

/* Read and process the section containing parameters from scene file */ 
static int   ReadParamsSection          ( char **Scene, 
                                          struct Section *Info);

/* Unification of array of points */
static void  UnifyPoints                ( int UnifiedPart,
                                          float ***Pnts, 
                                          int *PntsNum);

/* Fill triangle given by three vertices with points */
static void  FillTriangleWithPoints     ( float *Vrtx1, float *Vrtx2, 
                                          float *Vrtx3, float ***Pnts, 
                                          int *PntsNum, float Ival);

/* Fill parallelepiped given by origin and three vectors with points */ 
static void  FillParlpipedWithPoints    ( float *Vrtx, float *Vec1, 
                                          float *Vec2, float *Vec3, 
                                          float ***Pnts, int *PntsNum, 
                                          float Ival);

/* Fill parallelogram given by origin and two vectors with points */ 
static void  FillParlgramWithPoints     ( float *Vrtx, float *Vec1, 
                                          float *Vec2, float ***Pnts, 
                                          int *PntsNum, float Ival);

/* Fill segment given by origin and vector with points */ 
static void  FillSegmentWithPoints      ( float *Vrtx, float *Vec, 
                                          float ***Pnts, int *PntsNum, 
                                          float Ival);

/* Get point on segment given by origin and vector by parameter */
static void  GetPointOnSegmentByParam   ( float *Vrtx, float *Vec, 
                                          float *Pnt, float Param);

/**********************************************************/

/* Section info */
struct Section
{
    char *Name;                   /* Name in the scene description file */
    int FirstLine;                /* The first line of the section */
    int EndLine;                  /* The end line of the section */
    int (*Func)( char **Scene, 
                 struct Section 
                        *Info);   /* The function to read the section */
};

/* The maximum length of a section's name */
#define SECTION_NAME_LENGTH    16

/* The keyword to end a section */
#define SECTION_END           "$END"

/* Possible sections in the scene description file, 
 * they are processed in the order they follow here */
static struct Section Sections[] =
{
    /* Parameters section - contains simulator's parameters */
    "$PARAMS",    -1, -1, ReadParamsSection,
    /* Clouds section - set up clouds of particles          */
    "$CLOUDS",    -1, -1, ReadCloudsSection,
    /* Obstacles section - set up obstacles in the scene    */
    "$OBSTACLES", -1, -1, ReadObstaclesSection,
};

/* The size of this array */
static int SectsNum = sizeof(Sections) / sizeof(Sections[0]);

/**
 * Initialize the scene - the function reads the scene description 
 * file and initializes all variables and data structures used 
 * in simulation and rendering.
 */
void
InitScene( void)
{
    FILE *File;
    char **Scene;
    char Str[FILE_LINE_LENGTH + 1];
    char Name[SECTION_NAME_LENGTH + 1];
    char Fmt[6];
    int LinesNum;
    char SearchEnd;
    int i, j, n;

    File = fopen( SCENE_FILE_NAME, "r");
    
    if ( File == NULL )
    {
        /* There is no scene file */
        return;
    }

    /* Read scene description file */
    LinesNum = 0;
    Scene = NULL;
    while ( feof( File) == 0 )
    {
        /* Read next string from the file */
        fgets( Str, FILE_LINE_LENGTH, File);
        n = strlen( Str);

        /* An empty strings or strings with comments 
         * aren't stored (comments begin with '#') */
        for ( i = 0; i < n; i++ )
        {
            if ( isgraph( Str[i]) )
                break;
        }
            if ( i == n || Str[i] == '#' )
                continue;
            
        /* Allocate memory and store the string */
        i = LinesNum++;
        Scene = (char **)realloc( Scene, LinesNum * sizeof(char *));
        Scene[i] = (char *)malloc( (n + 1) * sizeof(char));
        strcpy( Scene[i], Str);
    }

    /* Find sections in the scene file */
    SearchEnd = 0;
    sprintf( Fmt, "%%%ds", SECTION_NAME_LENGTH);
    for ( i = 0; i < LinesNum; i++ )
    {
        /* Read next string from the scene description */
        sscanf( Scene[i], Fmt, Name);

        /* Section starts/ends with '$' */
        if ( Name[0] != '$' )
            continue;

        if ( SearchEnd )
        {
            /* The first line has been found already -
             * it's necessary to find the section's end */
            if ( strcmp( SECTION_END, Name) )
                continue;
            Sections[j].EndLine = i;
            SearchEnd = 0;
            continue;
        }
        
        for ( j = 0; j < SectsNum; j++ )
        {
            /* Some section has been found in the 
             * scene file - check if it is valid */
            if ( strcmp( Sections[j].Name, Name) )
                continue;
            Sections[j].FirstLine = i + 1;
            /* Now it's necessary to find the end */
            SearchEnd = 1;
            break;
        }
    }
    
    /* Read and process the sections of the scene file */
    for ( i = 0; i < SectsNum; i++ )
        Sections[i].Func( Scene, &Sections[i]);

    /* Close the file and free the memory */
    if ( File != NULL )
    {
        fclose( File);
        for ( i = 0; i < LinesNum; i++ )
            free( Scene[i]);
        free( Scene);
    }

    return;
} /* InitScene */

/**********************************************************/

/**
 * Read obstacles section which is specified by <Info> from array with 
 * scene description <Scene>, initialize corresponding data structures
 * and create boundary particles. The function returns 0 if succeeded 
 * and the number of string containing an error otherwise.
 */
static int
ReadObstaclesSection( char **Scene,           /* Array with scene description */
                      struct Section *Info)   /* Section's info */ 
{
    float **Pnts;
    int PntsNum;
    int Res;
    int i, n;
    
    Pnts = NULL;
    PntsNum = 0;
    Res = 0;

    /* Obstacle's specification occupies one string */
    ObstaclesNumber = Info->EndLine - Info->FirstLine;

    if ( Dimension == 2 )
    {
        struct ObstacleSegment *Segments;
        float Vec[2];
        
        /* Allocate memory for obstacles */
        Segments = (struct ObstacleSegment *)
                   malloc( ObstaclesNumber * sizeof(struct ObstacleSegment));
        Obstacles = (void *)Segments;
        
        for ( i = 0; i < ObstaclesNumber; i++ )
        {
            /* In 2D simulation segment-obstacles are used */
            memset( &Segments[i], 0, sizeof(struct ObstacleSegment));
            n = sscanf( Scene[Info->FirstLine + i], "%f %f %f %f", 
                        &(Segments[i].Vrtx1[0]), &(Segments[i].Vrtx1[1]), 
                        &(Segments[i].Vrtx2[0]), &(Segments[i].Vrtx2[1]));
            /* An error has occured */
            if ( n != 4 )
                break;
            /* Fill segment with points */
            VectorSubstraction( Vec, Segments[i].Vrtx2, Segments[i].Vrtx1);
            FillSegmentWithPoints( Segments[i].Vrtx1, Vec, &Pnts, 
                                   &PntsNum, BParticlesDistrib);
        }
        /* An error has occured */
        if ( i != ObstaclesNumber )
            free( Segments);
    }
    else if ( Dimension == 3 )
    {
        struct ObstacleTriangle *Triangles;
        
        /* Allocate memory for obstacles */
        Triangles = (struct ObstacleTriangle *)
                    malloc( ObstaclesNumber * sizeof(struct ObstacleTriangle));
        Obstacles = (void *)Triangles;
        
        for ( i = 0; i < ObstaclesNumber; i++ )
        {
            /* In 3D simulation triangle-obstacles are used */
            memset( &Triangles[i], 0, sizeof(struct ObstacleTriangle));
            n = sscanf( Scene[Info->FirstLine + i], "%f %f %f %f %f %f %f %f %f", 
                        &(Triangles[i].Vrtx1[0]), &(Triangles[i].Vrtx1[1]), 
                        &(Triangles[i].Vrtx1[2]), &(Triangles[i].Vrtx2[0]), 
                        &(Triangles[i].Vrtx2[1]), &(Triangles[i].Vrtx2[2]), 
                        &(Triangles[i].Vrtx3[0]), &(Triangles[i].Vrtx3[1]), 
                        &(Triangles[i].Vrtx3[2]));
            /* An error has occured */
            if ( n != 9 )
                break;
            /* Fill triangle with points */
            FillTriangleWithPoints( Triangles[i].Vrtx1, Triangles[i].Vrtx2, 
                                    Triangles[i].Vrtx3, &Pnts, &PntsNum, 
                                    BParticlesDistrib);
        }
        /* An error has occured */
        if ( i != ObstaclesNumber )
            free( Triangles);
    }

    if ( i != ObstaclesNumber )
    {
        /* An error has occured */
        Res = i;
    }
    else
    {
        /* Create boundary particles using coordinates of 
         * points which the obstacles have been filled with */
        UnifyPoints( 0, &Pnts, &PntsNum);
        BParticles = (struct BParticle *)malloc( PntsNum * sizeof(struct BParticle));
        for ( i = 0; i < PntsNum; i++ )
        {
            memset( &BParticles[i], 0, sizeof(struct BParticle));
            /* Boundary particle's position */
            memcpy( BParticles[i].Pos, Pnts[i], Dimension * sizeof(float));
        }
        /* Update the number of the boundary particles */
        BParticlesNumber = PntsNum;
    }

    /* Delete points */
    for ( i = 0; i < PntsNum; i++ )
        free( Pnts[i]);
    free( Pnts);

    return Res;
} /* ReadObstaclesSection */

/**********************************************************/

/**
 * Read clouds section which is specified by <Info> from array with 
 * scene description <Scene>, initialize corresponding data structures
 * and create smoothing particles. The function returns 0 if succeeded 
 * and the number of string containing an error otherwise.
 */
static int
ReadCloudsSection( char **Scene,           /* Array with scene description */
                   struct Section *Info)   /* Section's info */
{
    float **Pnts;
    int PntsNum;
    int Res;
    int i, j, n;
    
    Pnts = NULL;
    PntsNum = 0;
    Res = 0;
    
    if ( Dimension == 2 )
    {
        float Vrtx1[2], Vrtx2[2], Vrtx3[2];
        float Vel[2];
        
        for ( i = Info->FirstLine; i < Info->EndLine; i++ )
        {
            /* In 2D simulation cloud of particle 
             * has the form of a parallelogram */
            n = sscanf( Scene[i], "%f %f  %f %f  %f %f  %f %f", 
                                   &Vrtx1[0], &Vrtx1[1], 
                                   &Vrtx2[0], &Vrtx2[1], 
                                   &Vrtx3[0], &Vrtx3[1], 
                                   &Vel[0], &Vel[1]);
            /* An error has occured */
            if ( n != 8 )
                break;
            /* Fill parallelogram with points */
            n = PntsNum;
            FillParlgramWithPoints( Vrtx1, Vrtx2, Vrtx3, &Pnts, 
                                    &PntsNum, ParticlesDistrib);
            /* Create new particles using coordinates of points 
             * which the parallelogram has been filled with */
            UnifyPoints( n, &Pnts, &PntsNum);
            Particles = (struct Particle *)
                        realloc( Particles, PntsNum * sizeof(struct Particle));
            for ( j = n; j < PntsNum; j++ )
            {
                memset( &Particles[j], 0, sizeof(struct Particle));
                /* Particle's position */
                memcpy( Particles[j].Pos, Pnts[j], Dimension * sizeof(float));
                /* Particle's velocity */
                memcpy( Particles[j].Vel, Vel, Dimension * sizeof(float));
                memcpy( Particles[j].IvalVel, Vel, Dimension * sizeof(float));
            }
        }
    }
    else if ( Dimension == 3 )
    {
        float Vrtx1[3], Vrtx2[3], Vrtx3[3], Vrtx4[3];
        float Vel[3];
        
        for ( i = Info->FirstLine; i < Info->EndLine; i++ )
        {
            /* In 3D simulation cloud of particle 
             * has the form of a parallelepiped */
            n = sscanf( Scene[i], "%f %f %f  %f %f %f  %f %f %f \
                                   %f %f %f  %f %f %f", 
                                   &Vrtx1[0], &Vrtx1[1], &Vrtx1[2], 
                                   &Vrtx2[0], &Vrtx2[1], &Vrtx2[2], 
                                   &Vrtx3[0], &Vrtx3[1], &Vrtx3[2], 
                                   &Vrtx4[0], &Vrtx4[1], &Vrtx4[2], 
                                   &Vel[0], &Vel[1], &Vel[2]);
            /* An error has occured */
            if ( n != 15 )
                break;
            /* Fill parallelepiped with points */
            n = PntsNum;
            FillParlpipedWithPoints( Vrtx1, Vrtx2, Vrtx3, Vrtx4, 
                                     &Pnts, &PntsNum, ParticlesDistrib);
            /* Create new particles using coordinates of points 
             * which the parallelepiped has been filled with */
            UnifyPoints( n, &Pnts, &PntsNum);
            Particles = (struct Particle *)
                        realloc( Particles, PntsNum * sizeof(struct Particle));
            for ( j = n; j < PntsNum; j++ )
            {
                memset( &Particles[j], 0, sizeof(struct Particle));
                /* Particle's position */
                memcpy( Particles[j].Pos, Pnts[j], Dimension * sizeof(float));
                /* Particle's velocity */
                memcpy( Particles[j].Vel, Vel, Dimension * sizeof(float));
                memcpy( Particles[j].IvalVel, Vel, Dimension * sizeof(float));
            }
        }
    }

    if ( i != Info->EndLine )
    {
        /* An error has occured */
        free( Particles);
        Res = i;
    }
    else
    {
        /* Set other particles' parameters */
        for ( i = 0; i < PntsNum; i++ )
        {
            /* Particle's density */
            Particles[i].Dens = Density0;
            Particles[i].IvalDens = Density0;
            /* Particle's mass */
            Particles[i].Mass = pow( ParticlesDistrib, 3) * Density0;
        }
        /* Update the number of the particles */
        ParticlesNumber = PntsNum;
    }
    
    /* Delete points */
    for ( i = 0; i < PntsNum; i++ )
        free( Pnts[i]);
    free( Pnts);

    return Res;
} /* ReadCloudsSection */

/**********************************************************/

/* Parameter's info */
struct Param
{
    char *Name;   /* Name of the parameter         */
    int Type;     /* Type of the parameter         */
    void *Var;    /* Pointer to a variable the 
                     parameter's value to store in */
};

/* Possible types of parameters */
enum ParamsTypes
{
    INT_PARAM,       /* int    */
    FLOAT_PARAM,     /* float  */
    STRING_PARAM,    /* char*  */
};

/* The maximum length of a parameter-string */
#define STRING_PARAM_LENGTH  16

/* The maximum length of a parameter's name */
#define PARAM_NAME_LENGTH  16

/* Possible parameters in the parameters section */
static struct Param Params[] =
{
    /* Dimension of the simulation                 */
    "DIM",           INT_PARAM,     (void *)(&Dimension),
    /* Initial particle distribution               */
    "PRTS_DISTR",    FLOAT_PARAM,   (void *)(&ParticlesDistrib),
    /* Initial boundary particle distribution      */
    "BPRTS_DISTR",   FLOAT_PARAM,   (void *)(&BParticlesDistrib),
    /* Rest density                                */
    "DENS0",         FLOAT_PARAM,   (void *)(&Density0),
    /* Speed of sound                              */
    "SOS",           FLOAT_PARAM,   (void *)(&SOS),
    /* Kernel to use in the calculations           */
    "KERNEL",        STRING_PARAM,  (void *)(KernelType),
    /* Kernel's smoothing length                   */
    "SMOOTH_LEN",    FLOAT_PARAM,   (void *)(&SmoothR),
    /* Equation of state to calculate pressures    */
    "EOS",           STRING_PARAM,  (void *)(EOSType),
    /* Alpha factor to calculate viscosity         */
    "VISC_ALPHA",    FLOAT_PARAM,   (void *)(&ViscAlpha),
    /* Beta factor to calculate viscosity          */
    "VISC_BETA",     FLOAT_PARAM,   (void *)(&ViscBeta),
    /* Time step of integration                    */
    "TIME_STEP",     FLOAT_PARAM,   (void *)(&TimeStep),
    /* Clipping volume (the area to render)        */
    "CLIP_VOL",      FLOAT_PARAM,   (void *)(&ClipVolume),
};

/* The size of this array */
static int ParamsNum = sizeof(Params) / sizeof(Params[0]);

/**
 * Read parameters section which is specified by <Info> from array with 
 * scene description <Scene>, and initialize corresponding variables and 
 * data structures. The function returns 0 if succeeded and the number 
 * of string containing an error otherwise.
 */
static int
ReadParamsSection( char **Scene,           /* Array with scene description */
                   struct Section *Info)   /* Section's info */
{
    char Name[PARAM_NAME_LENGTH + 1];
    char Fmt[10];
    int i, j, n;
    int Res;

    Res = 0;

    /* Read parameters from the parameters section */
    for ( i = Info->FirstLine; i < Info->EndLine; i++ )
    {
        sprintf( Fmt, "%%%ds %%n", PARAM_NAME_LENGTH);
        sscanf( Scene[i], Fmt, Name, &n);

        for ( j = 0; j < ParamsNum; j++ )
        {
            if ( strcmp( Params[j].Name, Name) )
                continue;

            /* Some parameter has been found - read and store its value */
            if ( Params[j].Type == INT_PARAM )
            {
                /* The type of the parameter is integer */
                sscanf( Scene[i] + n, "%d", (int *)Params[j].Var);
                break;
            }
            else if ( Params[j].Type == FLOAT_PARAM )
            {
                /* The type of the parameter is float */
                sscanf( Scene[i] + n, "%f", (float *)Params[j].Var);
                break;
            }
            else if ( Params[j].Type == STRING_PARAM )
            {
                /* The type of the parameter is string (char *) */
                sprintf( Fmt, "%%%ds", STRING_PARAM_LENGTH);
                sscanf( Scene[i] + n, Fmt, (char *)Params[j].Var);
                break;
            }
        }
        
        if ( j == ParamsNum )
        {
            /* The parameter isn't valid */
            Res = i;
            break;
        }
    }

    return Res;
} /* ReadParamsSection */

/**********************************************************/

/**
 * Unification of array of points - the function searches for identical 
 * points in the array <Pnts> of the size <PntsNum>, eliminates all of 
 * them but one, and shift the array. New size of the array is returned 
 * through <PntsNum>. The size of the part of the array which is already 
 * unified is set through <UnifiedPart>.
 */
static void
UnifyPoints( int UnifiedPart, /* Unified part of the array */
             float ***Pnts,   /* Array of points */
             int *PntsNum)    /* Size of the array */
{
    float **p;
    int i, j, k;
    int n;

    /* Array of points */
    p = *Pnts;
    /* Current size of the array */
    n = *PntsNum;
    
    /* Unify the array */
    for ( i = 0; i < n - 1; i++ )
    {
        /* Search from the current position for 
         * point which is identical to p[i] */
        for ( j = ((UnifiedPart > i) ? UnifiedPart : i) + 1; j < n; j++ )
        {
            /* Is point identical to p[i]? */
            if ( memcmp( p[i], p[j], Dimension * sizeof(float)) )
                continue;
            /* Eliminate p[j] */
            free (p[j]);
            n--;
            /* Search from the end of the array 
             * for point to replace p[j] */
            for ( k = n; k > j; k-- )
            {
                /* Is point identical to p[j]? */
                if ( memcmp( p[i], p[k], Dimension * sizeof(float)) )
                {
                    /* Replace */
                    p[j] = p[k];
                    p[k] = NULL;
                    break;
                }
                /* Eliminate p[k] */
                free (p[k]);
                n--;
            }
        }
    }
    
    /* New size of the array */
    *PntsNum = n;
    /* Reallocate the array */
    p = (float **)realloc( p, n * sizeof(float *));
    *Pnts = p;

    return;
} /* UnifyPoints */

/**********************************************************/

/**
 * The function fills triangle given by <Vrtx1>, <Vrtx2> and <Vrtx3> with 
 * points, the interval between points is set by <Ival>. At the moment of 
 * function invocation, the array <Pnts> could already contain some points 
 * and in this case <PntsNum> has to contain its current size; if the 
 * array is empty, <Pnts> has to be set to NULL, and <PntsNum> has to be 
 * set to 0. New points are stored in the end of reallocated array <Pnts>, 
 * new size of the array is returned through <PntsNum>.
 */
static void
FillTriangleWithPoints( float *Vrtx1,   /* Vertex 1 */
                        float *Vrtx2,   /* Vertex 2 */
                        float *Vrtx3,   /* Vertex 3 */
                        float ***Pnts,  /* Array of points */
                        int *PntsNum,   /* Size of the array */
                        float Ival)     /* Interval between points */
{
    float Vec[3], Vec1[3], Vec2[3];
    float Pnt1[3], Pnt2[3];
    float Offset;
    float Param;
    float R;
    int i, j;
    
    /* Reference vectors */
    VectorSubstraction( Vec1, Vrtx2, Vrtx1);
    VectorSubstraction( Vec2, Vrtx3, Vrtx1);
    /* The length of the triangle's side */
    R = VectorNorm( Vec1);
    /* The number of points the triangle's side can be filled with */
    j = (int)(R / Ival);
    /* Fill triangle with points */
    Offset = (R - (float)(j - 1) * Ival) / 2;
    for ( i = 0; i < j; i++ )
    {
        /* Get point1 on one triangle's side and point2 on another */
        Param = R ? ((Ival * (float)i + Offset) / R) : 0;
        GetPointOnSegmentByParam( Vrtx1, Vec1, Pnt1, Param);
        GetPointOnSegmentByParam( Vrtx1, Vec2, Pnt2, Param);
        /* Fill the resulting segment with points */
        VectorSubstraction( Vec, Pnt2, Pnt1);
        FillSegmentWithPoints( Pnt1, Vec, Pnts, PntsNum, Ival);
    }
    
    return;
} /* FillTriangleWithPoints */

/**********************************************************/

/**
 * The function fills parallelepiped given by origin <Vrtx> and reference 
 * vectors <Vec1>, <Vec2> and <Vec3> with with points, the interval between 
 * points is set by <Ival>. At the moment of function invocation, the array 
 * <Pnts> could already contain some points and in this case <PntsNum> has 
 * to contain its current size; if the array is empty, <Pnts> has to be 
 * set to NULL, and <PntsNum> has to be set to 0. New points are stored in 
 * the end of reallocated array <Pnts>, new size of the array is returned 
 * through <PntsNum>.
 */
static void
FillParlpipedWithPoints( float *Vrtx,    /* Origin */
                         float *Vec1,    /* Vector 1 */
                         float *Vec2,    /* Vector 2 */
                         float *Vec3,    /* Vector 3 */
                         float ***Pnts,  /* Array of points */
                         int *PntsNum,   /* Size of the array */
                         float Ival)     /* Interval between points */
{
    float Pnt[3];
    float Param;
    float Offset;
    float R;
    int i, j;
    
    /* The length of the parallelepiped's edge */
    R = VectorNorm( Vec1);
    /* The number of points the parallelepiped's edge can be filled with */
    j = (int)(R / Ival);
    /* Fill parallelepiped with points */
    Offset = (R - (float)(j - 1) * Ival) / 2;
    for ( i = 0; i < j; i++ )
    {
        /* Get point on the parallelepiped's edge */
        Param = R ? ((Ival * (float)i + Offset) / R) : 0;
        GetPointOnSegmentByParam( Vrtx, Vec1, Pnt, Param);
        /* Fill the parallelogram with points */
        FillParlgramWithPoints( Pnt, Vec2, Vec3, Pnts, PntsNum, Ival);
    }
    
    return;
} /* FillParlpipedWithPoints */

/**********************************************************/

/**
 * The function fills parallelogram given by origin <Vrtx> and reference 
 * vectors <Vec1> and <Vec2> with points, the interval between points is 
 * set by <Ival>. At the moment of function invocation, the array <Pnts> 
 * could already contain some points and in this case <PntsNum> has to 
 * contain its current size; if the array is empty, <Pnts> has to be set 
 * to NULL, and <PntsNum> has to be set to 0. New points are stored in the 
 * end of reallocated array <Pnts>, new size of the array is returned 
 * through <PntsNum>.
 */
static void        
FillParlgramWithPoints( float *Vrtx,    /* Origin */
                        float *Vec1,    /* Vector 1 */
                        float *Vec2,    /* Vector 2 */
                        float ***Pnts,  /* Array of points */
                        int *PntsNum,   /* Size of the array */
                        float Ival)     /* Interval between points */
{
    float Pnt[3];
    float Param;
    float Offset;
    float R;
    int i, j;
    
    /* The length of the parallelogram's side */
    R = VectorNorm( Vec1);
    /* The number of points the parallelogram's side can be filled with */
    j = (int)(R / Ival);
    /* Fill parallelogram with points */
    Offset = (R - (float)(j - 1) * Ival) / 2;
    for ( i = 0; i < j; i++ )
    {
        /* Get point on the parallelogram's side */
        Param = R ? ((Ival * (float)i + Offset) / R) : 0;
        GetPointOnSegmentByParam( Vrtx, Vec1, Pnt, Param);
        /* Fill the segment with points */
        FillSegmentWithPoints( Pnt, Vec2, Pnts, PntsNum, Ival);
    }
    
    return;
} /* FillParlgramWithPoints */

/**********************************************************/

/**
 * The function fills segment given by origin <Vrtx> and reference 
 * vector <Vec> with points, the interval between points is set by 
 * <Ival>. At the moment of function invocation, the array <Pnts> 
 * could already contain some points and in this case <PntsNum> has 
 * to contain its current size; if the array is empty, <Pnts> has 
 * to be set to NULL, and <PntsNum> has to be set to 0. New points 
 * are stored in the end of reallocated array <Pnts>, new size of 
 * the array is returned through <PntsNum>.
 */
static void
FillSegmentWithPoints( float *Vrtx,    /* Origin */
                       float *Vec,     /* Vector */
                       float ***Pnts,  /* Array of points */
                       int *PntsNum,   /* Size of the array */
                       float Ival)     /* Interval between points */
{
    float Param;
    float Offset;
    float R;
    int i, j, n;
    
    /* The length of the segment */
    R = VectorNorm( Vec);
    /* Current size of the array */
    n = *PntsNum;
    /* The number of points the segment can be filled with */
    j = (int)(R / Ival);
    /* New size of the array */
    *PntsNum += j;
    /* Reallocate the array */
    *Pnts = (float **)realloc( *Pnts, *PntsNum * sizeof(float *));
    /* Fill segment with points */
    Offset = (R - (float)(j - 1) * Ival) / 2;
    for ( i = 0; i < j; i++ )
    {
        /* Allocate memory for new point */
        (*Pnts)[i + n] = (float *)malloc( Dimension * sizeof(float));
        /* Get next point on the segment by parameter and store it */
        Param = R ? ((Ival * (float)i + Offset) / R): 0;
        GetPointOnSegmentByParam( Vrtx, Vec, (*Pnts)[i + n], Param);
    }

    return;
} /* FillSegmentWithPoints */

/**********************************************************/

/**
 * It returns point <Pnt> belonging to a segment specified by 
 * origin <Vrtx> and reference vector <Vec>, the offset of 
 * point from <Vrtx> is determined by <Param> (should be [0..1]).
 */
static void
GetPointOnSegmentByParam( float *Vrtx,    /* Origin */
                          float *Vec,     /* Vector */
                          float *Pnt,     /* Point */
                          float Param)    /* Parameter */
{
    int d;
    
    for ( d = 0; d < Dimension; d++ )
        Pnt[d] = Param * Vec[d] + Vrtx[d];

    return;
} /* GetPointOnSegmentByParam */
