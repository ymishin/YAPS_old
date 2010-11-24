/**
 * Copyright (c) 2005,2010 Yury Mishin <yury.mishin@gmail.com>
 * See the file COPYING for copying permission.
 *
 * $Id$
 */

#ifndef YAPS_RENDER_H
#define YAPS_RENDER_H

/**********************************************************/

/* Default size of the window */
extern int WindowWidth;
extern int WindowHeight;

/* Clipping volume (the area to render) */
extern float ClipVolume;

/**********************************************************/

/* Initialize display list(s) */
extern void InitDisplayLists    ( void);

/* Initialize GL capabilities */
extern void InitGLCapabilities  ( void);

/* GLUT callbacks */
extern void DisplayCallback     ( void);
extern void ReshapeCallback     ( int Width, 
                                  int Height);
extern void KeyboardCallback    ( unsigned char Key, 
                                  int X, int Y);
extern void IdleCallback        ( void);
extern void SpecialFuncCallback ( int Key, 
                                  int X, int Y);

/**********************************************************/

#endif /* YAPS_RENDER_H */
