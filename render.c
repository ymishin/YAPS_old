/**
 * Copyright (c) 2005,2010 Yury Mishin <yury.mishin@gmail.com>
 * See the file COPYING for copying permission.
 *
 * $Id$
 */

#include <stdlib.h>
#include <string.h>
#include "opengl.h"
#include "common.h"
#include "calc.h"
#include "vector.h"
#include "render.h"

/**********************************************************/

/* The number of display list to draw the obstacles */
#define OBSTACLES_LIST    1

/* The number of display list to draw the help */
#define HELP_LIST         2

/**********************************************************/

/* RGB color to draw the smoothing particles */
static float ParticleColor[3]     = { 0.0f, 0.4f, 0.6f };

/* RGB color to draw the boundary particles */
static float BParticleColor[3]    = { 0.9f, 0.9f, 0.9f };

/* RGB color to clear the background */
static float BackgroundColor[3]   = { 0.7f, 0.7f, 0.7f };

/* RGB color to draw the segments */
static float SegmentColor[3]      = { 0.0f, 0.0f, 0.0f };

/* RGB color to draw the triangles' edges */
static float TriangleLineColor[3] = { 0.0f, 0.0f, 0.0f };

/* RGBA color to fill the triangles' interiors */
static float TriangleFillColor[4] = { 0.6f, 0.6f, 0.6f, 0.8f };

/* RGB color to draw the help */
static float HelpColor[3]         = { 0.0f, 0.2f, 0.3f };

/**********************************************************/

/* Default size of the window */
int WindowWidth  = 500;
int WindowHeight = 500;

/* Clipping volume (the area to render) */
float ClipVolume;

/**********************************************************/

/* Short help which is drawn on the screen */
static char *Help[] = 
{
    "Rotation    - arrow keys",
    "Scaling     - PgUp/PgDn",
    "Start/Pause - S",
    "Exit        - Q",
};

/* The size of this array */
static int HelpSize = sizeof(Help) / sizeof(Help[0]);

/* The factor which define the height of one help's line */
static float HelpLineHeight = 50.0f;

/* The factor which define the size of characters to draw the help */
static float HelpCharSize = 0.00015f;

/**
 * Initialize display list(s).
 */
void
InitDisplayLists( void)
{
    int i, j;

    /* Initialize HELP_LIST */
    glNewList( HELP_LIST, GL_COMPILE);
    
    /* Color to draw the help */
    glColor3fv( HelpColor);
    
    glPushMatrix();
    for ( i = 0; i < HelpSize; i++ )
    {
        /* Set the origin to draw the next line */
        glLoadIdentity();
        glTranslatef( 0.0f, ClipVolume - (ClipVolume / HelpLineHeight) * 
                           (float)(i + 1), ClipVolume);
        glScalef( ClipVolume * HelpCharSize, ClipVolume * HelpCharSize, 1.0f);
        /* Draw the line with help */
        for ( j = 0; j < strlen( Help[i]); j++ )
        {
            glutStrokeCharacter( GLUT_STROKE_MONO_ROMAN, Help[i][j]);
        }
    }
    glPopMatrix();

    /* End of HELP_LIST */
    glEndList();

    /* Initialize OBSTACLES_LIST */
    glNewList( OBSTACLES_LIST, GL_COMPILE);
    
#if 0
    /* Draw boundary particles */
    glColor3fv( BParticleColor);
    for ( i = 0; i < BParticlesNumber; i++ )
    {
        glPushMatrix();
        glTranslatef( BParticles[i].Pos[0], BParticles[i].Pos[1], BParticles[i].Pos[2]);
        glutSolidSphere( 2.0f, 10, 10);
        glPopMatrix();
    }
#endif
    
    if ( Dimension == 2 )
    {
        /* Obstacles are segments */
        struct ObstacleSegment *Segments;
        Segments = (struct ObstacleSegment *)Obstacles;
        
        /* Draw segments */
        glBegin( GL_LINES);
        for ( i = 0; i < ObstaclesNumber; i++ )
        {
            glColor3fv( SegmentColor);
            glVertex3fv( Segments[i].Vrtx1);
            glVertex3fv( Segments[i].Vrtx2);
        }
        glEnd();
    }
    else if ( Dimension == 3 )
    {
        /* Obstacles are triangles */
        struct ObstacleTriangle *Triangles;
        Triangles = (struct ObstacleTriangle *)Obstacles;
        
        /* Draw triangles' edges */
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE);
        glBegin( GL_TRIANGLES);
        for ( i = 0; i < ObstaclesNumber; i++ )
        {
            glColor3fv( TriangleLineColor);
            glVertex3fv( Triangles[i].Vrtx1);
            glVertex3fv( Triangles[i].Vrtx2);
            glVertex3fv( Triangles[i].Vrtx3);
        }
        glEnd();

        /* Fill triangles' interiors */
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL);
        /* If it is transparent - turn off depth test */
        if ( TriangleFillColor[3] < 1.0f )
            glDepthMask( GL_FALSE);
        glBegin( GL_TRIANGLES);
        for ( i = 0; i < ObstaclesNumber; i++ )
        {
            glColor4fv( TriangleFillColor);
            glVertex3fv( Triangles[i].Vrtx1);
            glVertex3fv( Triangles[i].Vrtx2);
            glVertex3fv( Triangles[i].Vrtx3);
        }
        glEnd();
        /* Turn on depth test if it was turned off */
        if ( TriangleFillColor[3] < 1.0f )
            glDepthMask( GL_TRUE);
    }

    /* End of OBSTACLES_LIST */
    glEndList();
    
    return;
} /* InitDisplayLists */

/**********************************************************/

/**
 * Initialize OpenGL capabilities.
 */
void
InitGLCapabilities( void)
{
    /* Turn on antialiasing for lines */
    glEnable( GL_LINE_SMOOTH);
    glHint( GL_LINE_SMOOTH_HINT, GL_NICEST);
    
    /* Turn on Z-Buffer */
    glEnable( GL_DEPTH_TEST);
    glDepthFunc( GL_LEQUAL);

    /* Turn on blending */
    glEnable( GL_BLEND);
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    return;
} /* InitGLCapabilities */

/**********************************************************/

/**********************************************************
 *                                                        *
 *                     GLUT CALLBACKS                     *
 *                                                        *
 **********************************************************/

/* State of pause instruction */
static int Pause = 1;

/**
 * GLUT idle callback.
 */
void
IdleCallback( void)
{
    /* Check the state */
    if ( Pause == 1 )
    {
        return;
    }
    
    /* Do one calculation step */
    DoCalcStep();

    /* Redisplay */
    glutPostRedisplay();
    
    return;
} /* IdleCallBack */

/**
 * GLUT display callback.
 */
void
DisplayCallback( void)
{
    int i;
    
    /* Clear the buffers */
    glClearColor( BackgroundColor[0],
                  BackgroundColor[1],
                  BackgroundColor[2],
                  1.0f);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /* Draw the help */
    glCallList( HELP_LIST);
    
    /* Draw the particles */
    for ( i = 0; i < ParticlesNumber; i++ )
    {
        glPushMatrix();
        glTranslatef( Particles[i].Pos[0], Particles[i].Pos[1], Particles[i].Pos[2]);
        glColor3fv( ParticleColor);
        glutSolidSphere( 3.5f, 20, 20);
        glPopMatrix();
    }

    /* Draw the obstacles */
    glCallList( OBSTACLES_LIST);

    /* Swap the buffers */
    glutSwapBuffers();
    
    return;
} /* DisplayCallback */

/**
 * GLUT reshape callback.
 */
void
ReshapeCallback( int Width,   /* Window's width */
                 int Height)  /* Window's height */
{
    /* It's not allowed to change the size of the window */
    glutReshapeWindow( WindowWidth, WindowHeight);
    glViewport( 0, 0, WindowWidth, WindowHeight);
    /* Set projection matrix */
    glMatrixMode( GL_PROJECTION);
    glLoadIdentity();
    glOrtho( 0.0f, ClipVolume, 0.0f, ClipVolume, 
             -ClipVolume * 4.0f, ClipVolume * 3.0f);
    glMatrixMode( GL_MODELVIEW);
    
    return;
} /* ReshapeCallback */

/**
 * GLUT keyboard callback.
 */
void
KeyboardCallback( unsigned char Key,   /* Key's code */
                  int X,               /* Position of */
                  int Y)               /* the mouse   */
{
    switch (Key) {
      case 's':
      case 'S':
          /* 's' or 'S' - start/pause the simulation */
          Pause = Pause ? 0 : 1;
          break;
      case 'Q':
      case 'q':
          /* 'q' or 'Q' - exit the program */
          free( Particles);
          free( BParticles);
          if ( Dimension == 2 )
              free( (struct ObstacleSegment *)Obstacles);
          else if ( Dimension == 3 )
              free( (struct ObstacleTriangle *)Obstacles);
          exit( 0);
      default:
        break;
    }

    return;
} /* KeyboardCallback */

/* Scaling/rotation steps */
static float ScaleStep   = 0.1f;
static float XRotateStep = 1.0f;
static float YRotateStep = 1.0f;

/* Current scaling/rotation factors */
static float ScaleFactor   = 1.0f;
static float XRotateFactor = 0.0f;
static float YRotateFactor = 0.0f;

/**
 * GLUT special functions callback.
 */
void
SpecialFuncCallback( int Key,   /* Key's code */
                     int X,     /* Position of */
                     int Y)     /* the mouse   */
{
    switch (Key) {
      case GLUT_KEY_UP:
        XRotateFactor += XRotateStep;
        break;
      case GLUT_KEY_DOWN:
        XRotateFactor -= XRotateStep;
        break;
      case GLUT_KEY_LEFT:
        YRotateFactor += YRotateStep;
        break;
      case GLUT_KEY_RIGHT:
        YRotateFactor -= YRotateStep;
        break;
      case GLUT_KEY_PAGE_UP:
        ScaleFactor += ScaleStep;
        break;
      case GLUT_KEY_PAGE_DOWN:
        ScaleFactor -= ScaleStep;
        break;
      default:
        break;
    }
    
    glLoadIdentity();

    /* Set the center of rotation */
    glTranslatef( ClipVolume / 2.0f, ClipVolume / 2.0f, ClipVolume / 2.0f);
    /* Rotate the scene along X-axis */
    glRotatef( XRotateFactor, 1.0, 0.0, 0.0);
    /* Rotate the scene along Y-axis */
    glRotatef( YRotateFactor, 0.0, 1.0, 0.0);
    /* Scale the scene */    
    if ( ScaleFactor < ScaleStep )
        ScaleFactor = ScaleStep;
    glScalef( ScaleFactor, ScaleFactor, ScaleFactor);
    /* Restore the origin */
    glTranslatef( -ClipVolume / 2.0f, -ClipVolume / 2.0f, -ClipVolume / 2.0f);
    
    /* Redisplay the scene */
    glutPostRedisplay();

    return;
} /* SpecialFuncCallback */
