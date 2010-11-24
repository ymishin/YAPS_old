/**
 * Copyright (c) 2005,2010 Yury Mishin <yury.mishin@gmail.com>
 * See the file COPYING for copying permission.
 *
 * $Id$
 */

#ifndef YAPS_CALC_H
#define YAPS_CALC_H

/**********************************************************/

/* Equation of state to calculate pressures */
extern char  EOSType[20];

/* Kernel to use in the calculations */
extern char  KernelType[20];

/* Kernel smoothing length */
extern float SmoothR;

/* Rest density */
extern float Density0;

/* Speed of sound */
extern float SOS;

/* Alpha factor to calculate viscosity */
extern float ViscAlpha;

/* Beta factor to calculate viscosity */
extern float ViscBeta;

/* Time step of integration */
extern float TimeStep;

/**********************************************************/

/* Initialize calculation module */
extern void InitCalc( void);

/* Do one calculation step */
extern void DoCalcStep( void);

/**********************************************************/

#endif /* YAPS_CALC_H */
