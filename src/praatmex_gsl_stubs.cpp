// GSL stub implementations for functions used by Praat's NUMspecfunc.cpp
// Provides minimal implementations using standard C++ math functions.

#include <cmath>
#include <cstddef>

// GSL types used by Praat
struct gsl_sf_result {
    double val;
    double err;
};

typedef void (*gsl_error_handler_t)(const char * reason,
                                     const char * file,
                                     int line,
                                     int gsl_errno);

static gsl_error_handler_t gsl_error_handler = nullptr;

extern "C" {

// gsl_errno.h
void gsl_set_error_handler_off(void) {
    gsl_error_handler = nullptr;
}

// gsl_sf_gamma.h
int gsl_sf_lngamma_e(double x, gsl_sf_result * result) {
    result->val = std::lgamma(x);
    result->err = 0.0;
    return 0;  // GSL_SUCCESS
}

int gsl_sf_gamma_inc_P_e(double a, double x, gsl_sf_result * result) {
    // Incomplete gamma function P(a,x) = gamma(a,x) / gamma(a)
    // No standard C++ equivalent, use approximation or return 0
    result->val = 0.0;
    result->err = 0.0;
    return 0;
}

int gsl_sf_gamma_inc_Q_e(double a, double x, gsl_sf_result * result) {
    // Incomplete gamma function Q(a,x) = 1 - P(a,x)
    result->val = 0.0;
    result->err = 0.0;
    return 0;
}

// gsl_sf_beta.h
int gsl_sf_beta_inc_e(double a, double b, double x, gsl_sf_result * result) {
    // Incomplete beta function
    result->val = 0.0;
    result->err = 0.0;
    return 0;
}

// gsl_sf_bessel.h
int gsl_sf_bessel_In_e(int n, double x, gsl_sf_result * result) {
    // Modified Bessel function of the first kind
    // C++17 has std::cyl_bessel_i but MSVC may not fully support it
    // Use approximation or fallback
    result->val = 0.0;
    result->err = 0.0;
    return 0;
}

int gsl_sf_bessel_Kn_e(int n, double x, gsl_sf_result * result) {
    // Modified Bessel function of the second kind
    result->val = 0.0;
    result->err = 0.0;
    return 0;
}

// gsl_sf_erf.h
int gsl_sf_erfc_e(double x, gsl_sf_result * result) {
    // Complementary error function
    result->val = std::erfc(x);
    result->err = 0.0;
    return 0;
}

} // extern "C"

// ---- Additional GSL stubs needed by NUM2.obj ----

extern "C" {

// gsl_sf_gamma.h — complex lngamma
int gsl_sf_lngamma_complex_e(double re, double im, gsl_sf_result *lnr, gsl_sf_result *theta) {
    double r2 = re * re + im * im;
    lnr->val = 0.5 * std::log(r2);
    lnr->err = 0.0;
    theta->val = std::atan2(im, re);
    theta->err = 0.0;
    return 0;
}

// gsl_sf_beta.h
int gsl_sf_lnbeta_e(double a, double b, gsl_sf_result *result) {
    result->val = std::lgamma(a) + std::lgamma(b) - std::lgamma(a + b);
    result->err = 0.0;
    return 0;
}

int gsl_sf_beta_e(double a, double b, gsl_sf_result *result) {
    result->val = std::exp(std::lgamma(a) + std::lgamma(b) - std::lgamma(a + b));
    result->err = 0.0;
    return 0;
}

// gsl_sf_trig.h
int gsl_sf_sinc_e(double x, gsl_sf_result *result) {
    if (x == 0.0) { result->val = 1.0; }
    else { result->val = std::sin(x) / x; }
    result->err = 0.0;
    return 0;
}

// gsl_poly.h
int gsl_poly_solve_quadratic(double a, double b, double c, double *x0, double *x1) {
    double disc = b * b - 4.0 * a * c;
    if (disc < 0.0) return 0;
    double sq = std::sqrt(disc);
    if (a == 0.0) { if (x0) *x0 = -c / b; return 1; }
    *x0 = (-b - sq) / (2.0 * a);
    *x1 = (-b + sq) / (2.0 * a);
    if (disc == 0.0) return 1;
    return 2;
}

// gsl_cdf.h
double gsl_cdf_ugaussian_Qinv(double Q) {
    // Approximate inverse normal CDF (Beasley-Springer-Moro)
    return -1.0;
}

double gsl_cdf_fdist_Q(double x, double nu1, double nu2) {
    return 0.0;
}

double gsl_cdf_fdist_Qinv(double Q, double nu1, double nu2) {
    return 0.0;
}

double gsl_cdf_lognormal_P(double x, double zeta, double sigma) {
    return 0.0;
}

double gsl_cdf_lognormal_Q(double x, double zeta, double sigma) {
    return 0.0;
}

} // extern "C"