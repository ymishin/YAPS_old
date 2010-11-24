/**
 * Copyright (c) 2005,2010 Yury Mishin <yury.mishin@gmail.com>
 * See the file COPYING for copying permission.
 *
 * $Id$
 */

#include "opengl.h"
#include "scene.h"
#include "calc.h"
#include "render.h"

/**********************************************************/

int
main( int argc, char **argv)
{
    /* Initialize GLUT */
    glutInit( &argc, argv);
    glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    
    /* Create window */
    glutInitWindowSize( WindowWidth, WindowHeight);
    glutCreateWindow( argv[0]);
    glutSetWindowTitle( "YAPS");
    
    /* Initialize scene */
    InitScene();
    
    /* Initialize calculation module */
    InitCalc();
    
    /* Initialize GL capabilities */
    InitGLCapabilities();
    
    /* Initialize display lists */
    InitDisplayLists();
    
    /* Register callbacks for current window */
    glutDisplayFunc( DisplayCallback);
    glutReshapeFunc( ReshapeCallback);
    glutKeyboardFunc( KeyboardCallback);
    glutSpecialFunc( SpecialFuncCallback);
    glutIdleFunc( IdleCallback);

    /* Go to main loop */
    glutMainLoop();
    
    return 0;
} /* main */
