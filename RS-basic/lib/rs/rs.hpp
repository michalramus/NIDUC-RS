#pragma once
#include <Arduino.h>

#define MAX_POINTS 10

void multiplyPoly(double poly[], int size, double a, double result[]);
void addPoly(double a[], int sizeA, double b[], int sizeB, double res[]);
void scalePoly(double poly[], int size, double scalar, double res[]);
void lagrangeInterpolation(double x[], double y[], int n, double coeffs[]);
bool coefficientsEqual(double coeffs1[], double coeffs2[], int size, double tolerance = 0.0001);
