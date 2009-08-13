/*
 * MALHEUR - Automatic Malware Analysis on Steroids
 * Copyright (c) 2009 Konrad Rieck (rieck@cs.tu-berlin.de)
 * Berlin Institute of Technology (TU Berlin).
 * --
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.  This program is distributed without any
 * warranty. See the GNU General Public License for more details. 
 */
 
#include "tests.h"
#include "mconfig.h"
#include "farray.h"
#include "fmath.h"

/* Global variables */
int verbose = 0;
config_t cfg;

/* String length */
#define STR_LENGTH              2000
/* Number of vector */
#define NUM_VECTORS             200 
/* Number of stress runs */
#define STRESS_RUNS             5

/* Test structure */
typedef struct {
    char *x;
    char *y;
    double res;
} test_t;

/* Addition test cases */
test_t test_add[] = {
    {"aa0bb0cc", "aa0bb0cc", 2.0},
    {"aa0bb0cc", "xx0bb0cc", 2.0},
    {"aa0bb0cc", "xx0yy0cc", 2.0},
    {"aa0bb0cc", "xx0yy0zz", 2.0},
    {"", "xx0yy0zz", 1},
    {"aa0bb0cc", "", 1},
    {NULL, NULL, 0}
};


/* Dot product test cases */
test_t test_dot[] = {
    {"aa0bb0cc", "aa0bb0cc", 0.3333333},
    {"aa0bb0cc", "xx0bb0cc", 0.2222222},
    {"aa0bb0cc", "xx0yy0cc", 0.1111111},
    {"aa0bb0cc", "xx0yy0zz", 0.0000000},
    {"aa", "aa", 1.000000},
    {"aa", "aa0xx", 0.500000},
    {"aa", "aa0xx0yy", 0.333333},
    {"aa", "aa0xx0yy0zz", 0.250000},
    {NULL, NULL, 0}
};

/* 
 * A simple static test for the addition of feature vectors
 */
int test_static_add() 
{
    int i, err = 0;
    fvec_t *fx, *fy, *fz;  

    test_printf("Addition of feature vectors");

    for (i = 0; test_add[i].x; i++) {
        /* Extract features */
        fx = fvec_extract(test_add[i].x, strlen(test_add[i].x), "test");
        fy = fvec_extract(test_add[i].y, strlen(test_add[i].y), "test");

        /* Add test vectors */
        fz = fvec_add(fx, fy);
        err += fabs(fvec_norm1(fz) - test_add[i].res) > 1e-7;

        fvec_destroy(fz);        
        fvec_destroy(fx);
        fvec_destroy(fy);
    }
    
    test_return(err, i);
    return err;
}

/* 
 * A simple static test for the dot-product of feature vectors
 */
int test_static_dot() 
{
    int i, err = 0;
    fvec_t *fx, *fy;

    test_printf("Dot product of feature vectors");

    for (i = 0; test_dot[i].x; i++) {
        /* Extract features */
        fx = fvec_extract(test_dot[i].x, strlen(test_dot[i].x), "test");
        fy = fvec_extract(test_dot[i].y, strlen(test_dot[i].y), "test");

        /* Compute dot product */
        double d = fvec_dot(fx, fy);
        err += fabs(d - test_dot[i].res) > 1e-6;

        fvec_destroy(fx);
        fvec_destroy(fy);
    }
    
    test_return(err, i);
    return err;
}

/* 
 * A stres test for the addition of feature vectors
 */
int test_stress_add() 
{
    int i, j, err = 0;
    fvec_t *fx, *fy, *fz;  
    char buf[STR_LENGTH + 1];

    test_printf("Stress test for addition of feature vectors");

    /* Create empty vector */
    fz = fvec_extract("aa0bb0cc", 8, "zero");
    for (i = 0; i< NUM_VECTORS; i++) {
    
        /* Create random key and string */
        for (j = 0; j < STR_LENGTH; j++)
            buf[j] = rand() % 10 + '0';
        buf[j] = 0; 
    
        /* Extract features */
        fx = fvec_extract(buf, strlen(buf), "test");
        
        /* Add fx to fz */
        fy = fvec_add(fz, fx);
        fvec_destroy(fz);

        err += fabs(fvec_norm1(fy) - 2.0) > 1e-7;
        
        /* Substract fx from fz */
        fz = fvec_sub(fy, fx);
        fvec_sparsify(fz);
        
        /* Clean up */
        fvec_destroy(fy);
        fvec_destroy(fx);
    }
    
    fvec_destroy(fz);
    test_return(err, i);
    return err;
}

/* 
 * A simple stress test for feature arrays
 */
int test_stress_dot_array() 
{
    int i, j, k, err = 0;
    fvec_t *f;
    farray_t *fa;
    char buf[STR_LENGTH + 1], label[32];

    test_printf("Stress test for dot product of feature arrays");

    for (i = 0; i < STRESS_RUNS; i++) {
        /* Create array */
        fa = farray_create("test");
        
        for (j = 0; j < NUM_VECTORS; j++) {
            for (k = 0; k < STR_LENGTH; k++)
                buf[k] = rand() % 10 + '0';
            buf[k] = 0;    
            
            /* Extract features */
            f = fvec_extract(buf, strlen(buf), "test");

            /* Get label */
            snprintf(label, 32, "label%.2d", rand() % 10);
            
            /* Add to array */
            farray_add(fa, f, label);
        }    
           
        double *d = malloc(NUM_VECTORS * NUM_VECTORS * sizeof(double));
        farray_dot(fa, fa, d);

        for (j = 0; j < fa->len ; j++) {
            double n = sqrt(d[j * fa->len + j]);
            err += fabs(fvec_norm2(fa->x[j]) - n) > 1e-7;
        }
        
        free(d);
                      
        /* Destroy features */            
        farray_destroy(fa);
    }
    
    test_return(err, NUM_VECTORS * STRESS_RUNS);
    return err;
}


/* 
 * A stres test for the addition of feature vectors
 */
int test_stress_dot() 
{
    int i, j, err = 0;
    fvec_t *fx, *fy;
    char buf[STR_LENGTH + 1];

    test_printf("Stress test for dot product of feature vectors");

    /* Create empty vector */
    for (i = 0; i< NUM_VECTORS; i++) {
    
        /* Create random key and string */
        for (j = 0; j < STR_LENGTH; j++)
            buf[j] = rand() % 10 + '0';
        buf[j] = 0; 
        fx = fvec_extract(buf, strlen(buf), "test");

        /* Create random key and string */
        for (j = 0; j < STR_LENGTH; j++)
            buf[j] = rand() % 10 + '0';
        buf[j] = 0; 
        fy = fvec_extract(buf, strlen(buf), "test");

        double nx = fvec_dot(fx, fx);
        double ny = fvec_dot(fy, fy);
        err += fabs(fvec_norm2(fx) - sqrt(nx)) > 1e-7;
        err += fabs(fvec_norm2(fy) - sqrt(ny)) > 1e-7;
        err += fabs(fvec_dot(fx, fy) > nx + ny);

        /* Clean up */
        fvec_destroy(fx);
        fvec_destroy(fy);
    }

    test_return(err, 3 * i);
    return err;
}


/* 
 * A simple stress test for feature arrays
 */
int test_stress_add_array() 
{
    int i, j, k, err = 0;
    fvec_t *f;
    farray_t *fa;
    char buf[STR_LENGTH + 1], label[32];

    test_printf("Stress test for addition of feature arrays");

    for (i = 0; i < STRESS_RUNS; i++) {
        /* Create array */
        fa = farray_create("test");
        
        for (j = 0; j < NUM_VECTORS; j++) {
            for (k = 0; k < STR_LENGTH; k++)
                buf[k] = rand() % 10 + '0';
            buf[k] = 0;    
            
            /* Extract features */
            f = fvec_extract(buf, strlen(buf), "test");

            /* Get label */
            snprintf(label, 32, "label%.2d", rand() % 10);
            
            /* Add to array */
            farray_add(fa, f, label);
        }    
           
        fvec_t *f = farray_sum(fa);
        err += fabs(fvec_norm1(f) - NUM_VECTORS) > 1e-5;
                      
        /* Destroy features */            
        fvec_destroy(f);
        farray_destroy(fa);
    }
    
    test_return(err, STRESS_RUNS);
    return err;
}


/**
 * Main function
 */
int main(int argc, char **argv)
{
    int err = FALSE;
    
    /* Create config */
    config_init(&cfg);
    config_check(&cfg);

    config_set_string(&cfg, "features.vect_embed", "cnt");    
    config_set_string(&cfg, "features.vect_norm", "l1");
    config_set_string(&cfg, "features.ngram_delim", "0");    
    config_set_int(&cfg, "features.ngram_len", 1);  
    
    err |= test_static_add(); 
    err |= test_stress_add();
    err |= test_stress_add_array();
    err |= test_static_dot(); 
    err |= test_stress_dot();
    err |= test_stress_dot_array();

    config_destroy(&cfg);
    return err;
} 

