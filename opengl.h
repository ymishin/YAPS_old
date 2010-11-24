/**
 * Copyright (c) 2005,2010 Yury Mishin <yury.mishin@gmail.com>
 * See the file COPYING for copying permission.
 *
 * $Id$
 */

#ifndef YAPS_OPENGL_H
#define YAPS_OPENGL_H

/**********************************************************/

#ifndef WIN32_GL
/* X11 */
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#else
/* Win32 */
#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include "glut.h"
#endif

/**********************************************************/

#endif /* YAPS_OPENGL_H */
