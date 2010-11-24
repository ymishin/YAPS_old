/**
 * Copyright (c) 2005,2010 Yury Mishin <yury.mishin@gmail.com>
 * See the file COPYING for copying permission.
 *
 * $Id$
 */

#ifndef YAPS_EOS_H
#define YAPS_EOS_H

/**********************************************************/

/* State equation's info */
struct StateEquation
{
    char  *Name;                 /* Name of the EOS */
    void (*CalcPress)( void);    /* Calculate particles' pressures */
};

/* All implemented equations of state */
extern struct StateEquation StateEquations[];

/* The size of this array */
extern int StateEquationsNum;

/**********************************************************/

#endif /* YAPS_EOS_H */
