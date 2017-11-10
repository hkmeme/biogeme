#ifndef DEINTEGRATOR_H
#define DEINTEGRATOR_H
 
#include "DEIntegrationConstants.h"
#include <float.h>
 
/*! Numerical integration in one dimension using the double expontial method of M. Mori. */
template<class TFunctionObject>
class DEIntegrator
{
public:
    /*! Integrate an analytic function over a finite interval. @return The value of the integral. */
    static patReal Integrate
    (
        const TFunctionObject& f,       //!< [in] integrand
        patReal a,                       //!< [in] left limit of integration
        patReal b,                       //!< [in] right limit of integration
        patReal targetAbsoluteError,     //!< [in] desired bound on error
        int& numFunctionEvaluations,    //!< [out] number of function evaluations used
        patReal& errorEstimate           //!< [out] estimated error in integration
    )
    {
        // Apply the linear change of variables x = ct + d
        // $$\int_a^b f(x) dx = c \int_{-1}^1 f( ct + d ) dt$$
        // c = (b-a)/2, d = (a+b)/2
 
        patReal c = 0.5*(b - a);
        patReal d = 0.5*(a + b);
 
        return IntegrateCore
        (
            f, 
            c, 
            d, 
            targetAbsoluteError, 
            numFunctionEvaluations, 
            errorEstimate, 
            doubleExponentialAbcissas, 
            doubleExponentialWeights
        );
    }
 
    /*! Integrate an analytic function over a finite interval. 
        This version overloaded to not require arguments passed in for 
        function evaluation counts or error estimates.
        @return The value of the integral. 
    */
    static patReal Integrate
    (
        const TFunctionObject& f,       //!< [in] integrand
        patReal a,                       //!< [in] left limit of integration
        patReal b,                       //!< [in] right limit of integration
        patReal targetAbsoluteError      //!< [in] desired bound on error
    )
    {
        int numFunctionEvaluations;
        patReal errorEstimate;
        return Integrate
        (
            f,
            a,
            b,
            targetAbsoluteError,
            numFunctionEvaluations,
            errorEstimate
        );
    }
 
 
 
private:
    // Integrate f(cx + d) with the given integration constants
    static patReal IntegrateCore
    (
        const TFunctionObject& f,
        patReal c,   // slope of change of variables
        patReal d,   // intercept of change of variables
        patReal targetAbsoluteError,
        int& numFunctionEvaluations,
        patReal& errorEstimate,
        const patReal* abcissas,
        const patReal* weights
    )
    {
        targetAbsoluteError *= c;
 
        // Offsets to where each level's integration constants start.
        // The last element is not a beginning but an end.
        int offsets[] = {1, 4, 7, 13, 25, 49, 97, 193};     
        int numLevels = sizeof(offsets)/sizeof(int) - 1;
 
        patReal newContribution = 0.0;
        patReal integral = 0.0;
        errorEstimate = DBL_MAX;
        patReal h = 1.0;
        patReal previousDelta, currentDelta = DBL_MAX;
 
        integral = f(c*abcissas[0] + d) * weights[0];
        int i;
        for (i = offsets[0]; i != offsets[1]; ++i)
            integral += weights[i]*(f(c*abcissas[i] + d) + f(-c*abcissas[i] + d));
 
        for (int level = 1; level != numLevels; ++level)
        {
            h *= 0.5;
            newContribution = 0.0;
            for (i = offsets[level]; i != offsets[level+1]; ++i)
                newContribution += weights[i]*(f(c*abcissas[i] + d) + f(-c*abcissas[i] + d));
            newContribution *= h;
 
            // difference in consecutive integral estimates
            previousDelta = currentDelta;
            currentDelta = fabs(0.5*integral - newContribution); 
            integral = 0.5*integral + newContribution;
 
            // Once convergence kicks in, error is approximately squared at each step.
            // Determine whether we're in the convergent region by looking at the trend in the error.
            if (level == 1)
                continue; // previousDelta meaningless, so cannot check convergence.
 
            // Exact comparison with zero is harmless here.  Could possibly be replaced with
            // a small positive upper limit on the size of currentDelta, but determining
            // that upper limit would be difficult.  At worse, the loop is executed more
            // times than necessary.  But no infinite loop can result since there is
            // an upper bound on the loop variable.
            if (currentDelta == 0.0) 
                break;
            patReal r = log( currentDelta )/log( previousDelta );  // previousDelta != 0 or would have been kicked out previously
 
            if (r > 1.9 && r < 2.1) 
            {
                // If convergence theory applied perfectly, r would be 2 in the convergence region.  
                // r close to 2 is good enough. We expect the difference between this integral estimate 
                // and the next one to be roughly delta^2.
                errorEstimate = currentDelta*currentDelta; 
            }
            else
            {
                // Not in the convergence region.  Assume only that error is decreasing.
                errorEstimate = currentDelta;
            }
 
            if (errorEstimate < 0.1*targetAbsoluteError)
                break;
        }
        
        numFunctionEvaluations = 2*i - 1;
        errorEstimate *= c;
        return c*integral;
    }
 
};
 
#endif // include guard
