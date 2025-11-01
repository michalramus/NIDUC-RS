#include <iostream>
#include <vector>
using namespace std;

// Multiply a polynomial by (x - a)
vector<double> multiplyPoly(const vector<double>& poly, double a) {
    vector<double> res(poly.size() + 1);
    for (size_t i = 0; i < poly.size(); ++i) {
        res[i] -= poly[i] * a;
        res[i + 1] += poly[i];
    }
    return res;
}

// Add two polynomials
vector<double> addPoly(const vector<double>& a, const vector<double>& b) {
    vector<double> res(max(a.size(), b.size()), 0.0);
    for (size_t i = 0; i < a.size(); ++i) res[i] += a[i];
    for (size_t i = 0; i < b.size(); ++i) res[i] += b[i];
    return res;
}

// Multiply a polynomial by a scalar
vector<double> scalePoly(const vector<double>& poly, double scalar) {
    vector<double> res(poly.size());
    for (size_t i = 0; i < poly.size(); ++i)
        res[i] = poly[i] * scalar;
    return res;
}

int main() {
    int n;
    cout << "Enter number of data points: ";
    cin >> n;

    vector<double> x(n), y(n);
    cout << "Enter x values:\n";
    for (int i = 0; i < n; ++i) cin >> x[i];
    cout << "Enter y values:\n";
    for (int i = 0; i < n; ++i) cin >> y[i];

    vector<double> coeffs(n, 0.0);  // Final polynomial coefficients

    for (int i = 0; i < n; ++i) {
        vector<double> Li = {1.0};
        double denom = 1.0;

        for (int j = 0; j < n; ++j) {
            if (i != j) {
                Li = multiplyPoly(Li, x[j]);
                denom *= (x[i] - x[j]);
                for (double &v : Li) v *= -1; // since (x - xj) = -(xj - x)
                Li[Li.size() - 1] += 0; // no effect; just clarifies structure
                Li[0] += x[j]; // Fix sign reversal
            }
        }

        // Fix sign issue properly
        Li = {1.0};
        denom = 1.0;
        for (int j = 0; j < n; ++j) {
            if (i != j) {
                Li = multiplyPoly(Li, -x[j]);
                denom *= (x[i] - x[j]);
            }
        }

        // Scale L_i(x) by y_i / denom
        Li = scalePoly(Li, y[i] / denom);

        // Add to total polynomial
        coeffs = addPoly(coeffs, Li);
    }

    cout << "\nInterpolating polynomial coefficients:\n";
    for (int i = 0; i < n; ++i)
        cout << "a[" << i << "] = " << coeffs[i] << endl;

    cout << "\nPolynomial: P(x) = ";
    for (int i = 0; i < n; ++i) {
        cout << coeffs[i];
        if (i > 0) cout << "*x^" << i;
        if (i != n - 1) cout << " + ";
    }
    cout << endl;

    return 0;
}
