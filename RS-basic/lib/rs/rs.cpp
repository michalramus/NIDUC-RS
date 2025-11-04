#include <Arduino.h>
#include "rs.hpp"

// Porównanie współczynników z tolerancją
bool coefficientsEqual(double coeffs1[], double coeffs2[], int size,
                       double tolerance) {
    for (int i = 0; i < size; i++) {
        if (abs(coeffs1[i] - coeffs2[i]) > tolerance) {
            return false;
        }
    }
    return true;
}

// Multiply a polynomial by (x - a)
void multiplyPoly(double poly[], int size, double a, double result[]) {
  for (int i = 0; i <= size; i++) result[i] = 0.0;

  for (int i = 0; i < size; i++) {
    result[i]     -= poly[i] * a;
    result[i + 1] += poly[i];
  }
}

// Add two polynomials (res = a + b)
void addPoly(double a[], int sizeA, double b[], int sizeB, double res[]) {
  int maxSize = (sizeA > sizeB) ? sizeA : sizeB;
  for (int i = 0; i < maxSize; i++) res[i] = 0.0;

  for (int i = 0; i < sizeA; i++) res[i] += a[i];
  for (int i = 0; i < sizeB; i++) res[i] += b[i];
}

// Multiply a polynomial by a scalar
void scalePoly(double poly[], int size, double scalar, double res[]) {
  for (int i = 0; i < size; i++)
    res[i] = poly[i] * scalar;
}

// Compute Lagrange Interpolating Polynomial Coefficients
void lagrangeInterpolation(double x[], double y[], int n, double coeffs[]) {
  for (int i = 0; i < n; i++) coeffs[i] = 0.0;

  double Li[MAX_POINTS];
  double temp[MAX_POINTS];
  double scaled[MAX_POINTS];
  double denom;

  for (int i = 0; i < n; i++) {
    // Start with Li(x) = 1
    Li[0] = 1.0;
    int Li_size = 1;
    denom = 1.0;

    // Compute (x - xj) product for all j != i
    for (int j = 0; j < n; j++) {
      if (i != j) {
        multiplyPoly(Li, Li_size, x[j], temp);
        Li_size += 1;
        for (int k = 0; k < Li_size; k++) Li[k] = temp[k];
        denom *= (x[i] - x[j]);
      }
    }

    // Scale L_i(x) by y_i / denom
    scalePoly(Li, Li_size, y[i] / denom, scaled);

    // Add to total polynomial
    double newCoeffs[MAX_POINTS];
    addPoly(coeffs, n, scaled, Li_size, newCoeffs);
    for (int k = 0; k < n; k++) coeffs[k] = newCoeffs[k];
  }
}
