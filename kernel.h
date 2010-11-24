/**
 * Copyright (c) 2005,2010 Yury Mishin <yury.mishin@gmail.com>
 * See the file COPYING for copying permission.
 *
 * $Id$
 */

#ifndef YAPS_KERNEL_H
#define YAPS_KERNEL_H

/**********************************************************/

/* The kernel's info */
struct Kernel
{
    char  *Name;                     /* Name of the kernel */
    void (*Init)( void);             /* Initialize the kernel */
    int  (*GetGrad)( float *Grad, 
                     float *Rij);    /* Get the kernel's gradient */
};

/* All the implemented kernels */
extern struct Kernel Kernels[];

/* The size of this array */
extern int KernelsNum;

/**********************************************************/

#endif /* YAPS_KERNEL_H */
