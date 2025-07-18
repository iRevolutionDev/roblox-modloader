/**
 @file Spline.h

 @author Morgan McGuire, http://graphics.cs.williams.edu
 */

#ifndef G3D_SPLINE_H
#define G3D_SPLINE_H

#include "platform.h"
#include "Array.h"
#include "g3dmath.h"
#include "Matrix4.h"
#include "Vector4.h"

namespace G3D {

/** Common implementation code for all G3D::Spline template parameters */
class SplineBase {
public:

    /** Times at which control points occur.  Must have the same
        number of elements as Spline::control. */
    Array<float>            time;

    /** If cyclic, then the control points will be assumed to wrap around.
        If not cyclic, then the tangents at the ends of the spline
        point to the final control points.*/
    bool                    cyclic;

    /** For a cyclic spline, this is the time elapsed between the last
        control point and the first. If less than or equal to zero this is
        assumed to be:

        (time[0] - time[1] + .
         time[time.size() - 1] - time[time.size() - 2]) / 2.
    */
    float                   finalInterval;
    
    SplineBase() : cyclic(true), finalInterval(-1) {}

    virtual ~SplineBase() {}
    
    /** See specification for Spline::finalInterval; this handles the
      non-positive case.  Returns 0 if not cyclic. */
    float getFinalInterval() const;

    /** Returns the amount of time covered by this spline in one
        period.  For a cyclic spline, this contains the final
        interval.*/
    float duration() const;

    /** Computes the derivative spline basis from the control point version. */
    static Matrix4 computeBasis();
    
protected:

    /** Assumes that t0 <= s < tn.  called by computeIndex. */
    void computeIndexInBounds(float s, int& i, float& u) const;

public:
    
    /**
       Given a time @a s, finds @a i and 0 <= @a u < 1 such that
       @a s = time[@a i] * @a u + time[@a i + 1] * (1 - @a u).  Note that
       @a i may be outside the bounds of the time and control arrays;
       use getControl to handle wraparound and extrapolation issues.
       
       This function takes expected O(1) time for control points with
       uniform time sampled control points or for uniformly
       distributed random time samples, but may take O( log time.size() ) time
       in the worst case.

       Called from evaluate().
    */
    void computeIndex(float s, int& i, float& u) const;
};


/**
 Smooth parameteric curve implemented using a piecewise 3rd-order
 Catmull-Rom spline curve.  The spline is considered infinite and may
 either continue linearly from the specified control points or cycle
 through them.  Control points are spaced uniformly in time at unit
 intervals by default, but irregular spacing may be explicitly
 specified.

 The dimension of the spline can be set by varying the Control
 template parameter.  For a 1D function, use Spline<float>.  For a
 curve in the plane, Spline<Vector2>.  Note that <i>any</i> template
 parameter that supports operator+(Control) and operator*(float) can
 be used; you can make splines out of G3D::Vector4, G3D::Matrix3, or
 your own classes.

 To provide shortest-path interpolation, subclass G3D::Spline and
 override ensureShortestPath().  To provide normalization of
 interpolated points (e.g., projecting Quats onto the unit
 hypersphere) override correct().

 See Real Time Rendering, 2nd edition, ch 12 for a general discussion
 of splines and their properties.

 \sa G3D::UprightSpline
 */
template<typename Control>
class Spline : public SplineBase {
protected:
    /** The additive identity control point. */
    Control                 zero;

public:

    /** Control points. Must have the same number of elements as
        Spline::time.*/
    Array<Control>          control;

    Spline() {
        static Control x;
        // Hide the fact from C++ that we are using an
        // uninitialized variable here by pointer arithmetic.
        // This is ok because any type that is a legal control
        // point also supports multiplication by float.
        zero = *(&x) * 0.0f;
    }

    /** Appends a control point at a specific time that must be
        greater than that of the previous point. */
    void append(float t, const Control& c) {
        debugAssertM((time.size() == 0) || (t > time.last()), 
                     "Control points must have monotonically increasing times.");
        time.append(t);
        control.append(c);
        debugAssert(control.size() == time.size());
    }


    /** Appends control point spaced in time based on the previous
        control point, or spaced at unit intervals if this is the
        first control point. */
    void append(const Control& c) {
        switch (time.size()) {
        case 0:
            append(0, c);
            break;

        case 1:
            if (time[0] == 0) {
                append(1, c);
            } else {
                append(time[0], c);
            }
            break;

        default:
            append(2 * time[time.size() - 1] - time[time.size() - 2], c);
        }
        debugAssert(control.size() == time.size());
    }

    /** Erases all control points and times, but retains the state of 
        cyclic and finalInterval.
     */
    void clear() {
        control.clear();
        time.clear();
    }


    /** Number of control points */
    int size() const {
        debugAssert(time.size() == control.size());
        return control.size();
    }
    

    /** Returns the requested control point and time sample based on
        array index.  If the array index is out of bounds, wraps (for
        a cyclic spline) or linearly extrapolates (for a non-cyclic
        spline), assuming time intervals follow the first or last
        sample recorded.

        Calls correct() on the control point if it was extrapolated.

        Returns 0 if there are no control points.

        @sa Spline::control and Spline::time for the underlying
        control point array; Spline::computeIndex to find the index
        given a time.
    */
    void getControl(int i, float& t, Control& c) const {
        int N = control.size();
        if (N == 0) {
            c = zero;
            t = 0;
        } else if (cyclic) {
            c = control[iWrap(i, N)];

            if (i < 0) {
                // Wrapped around bottom

                // Number of times we wrapped around the cyclic array
                int wraps = (N + 1 - i) / N;                    
                int j = (i + wraps * N) % N;
                t = time[j] - wraps * duration();

            } else if (i < N) {

                t = time[i];

            } else {
                // Wrapped around top

                // Number of times we wrapped around the cyclic array
                int wraps = i / N;                    
                int j = i % N;
                t = time[j] + wraps * duration();
            }

        } else if (i < 0) {
            // Are there enough points to extrapolate?
            if (N >= 2) {
                // Step away from control point 0
                float dt = time[1] - time[0];
                
                // Extrapolate (note; i is negative)
                c = control[1] * float(i) + control[0] * float(1 - i);
                correct(c);
                t = dt * i + time[0];

            } else {
                // Just clamp
                c = control[0];

                // Only 1 time; assume 1s intervals
                t = time[0] + i;
            }

        } else if (i >= N) {
            if (N >= 2) {
                float dt = time[N - 1] - time[N - 2];

                // Extrapolate
                c = control[N - 1] * float(i - N + 2) + control[N - 2] * -float(i - N + 1);
                correct(c);
                t = time[N - 1] + dt * (i - N + 1);

            } else {
                // Return the last, clamping
                c = control.last();
                // Only 1 time; assume 1s intervals
                t = time[0] + i;
            }
        } else {
            // In bounds
            c = control[i];
            t = time[i];
        }
    }

protected:

    /** Returns a series of N control points and times, fixing
        boundary issues.  The indices may be assumed to be treated
        cyclically. */
    void getControls(int i, float* T, Control* A, int N) const {
        for (int j = 0; j < N; ++j) {
            getControl(i + j, T[j], A[j]);
        }
        ensureShortestPath(A, N);
    }

    /**
       Mutates the array of N control points that begins at \a A. It is useful to override this
       method by one that wraps the values if they are angles or quaternions
       for which "shortest path" interpolation is significant.
     */
    virtual void ensureShortestPath(Control* A, int N) const { (void)A; (void) N;}

    /** Normalize or otherwise adjust this interpolated Control. */
    virtual void correct(Control& A) const { (void)A; }

public:
   

    /**
       Return the position at time s.  The spline is defined outside
       of the time samples by extrapolation or cycling.
     */
    Control evaluate(float s) const {
        debugAssertM(control.size() == time.size(), "Corrupt spline: wrong number of control points.");

        /*
        @cite http://www.gamedev.net/reference/articles/article1497.asp 
        Derivation of basis matrix follows.
        
        Given control points with positions p[i] at times t[i], 0 <= i <= 3, find the position 
        at time t[1] <= s <= t[2].

        Let u = s - t[0]
        Let U = [u^0 u^1 u^2 u^3] = [1 u u^2 u^3]
        Let dt0 = t[0] - t[-1]
        Let dt1 = t[1] - t[0]
        Let dt2 = t[2] - t[1]
         */

        // Index of the first control point (i.e., the u = 0 point)
        int i = 0;
        // Fractional part of the time
        float u = 0;

        computeIndex(s, i, u);

        Control p[4];
        float   t[4];
        getControls(i - 1, t, p, 4);
        float dt0 = t[1] - t[0];
        float dt1 = t[2] - t[1];
        float dt2 = t[3] - t[2];

        static const Matrix4 basis = computeBasis();

        // Powers of u
        Vector4 uvec((float)(u*u*u), (float)(u*u), (float)u, 1.0f);

        // Compute the weights on each of the control points.
        const Vector4& weights = uvec * basis;
        
        // Compute the weighted sum of the neighboring control points.
        Control sum;

        const Control& p0 = p[0];
        const Control& p1 = p[1];
        const Control& p2 = p[2];
        const Control& p3 = p[3];

        // The factor of 1/2 from averaging two time intervals is 
        // already factored into the basis
        
        // tan1 = (dp0 / dt0 + dp1 / dt1) * ((dt0 + dt1) * 0.5);
        // The last term normalizes for unequal time intervals
        float x = (dt0 + dt1) * 0.5f;
        float n0 = x / dt0;
        float n1 = x / dt1;
        float n2 = x / dt2;

        const Control& dp0 = p1 + (p0*-1.0f);
        const Control& dp1 = p2 + (p1*-1.0f);
        const Control& dp2 = p3 + (p2*-1.0f);

        const Control& dp1n1 = dp1 * n1;
        const Control& tan1 = dp0 * n0 + dp1n1;
        const Control& tan2 = dp1n1 + dp2 * n2;

        sum = 
            tan1 * weights[0] +
             p1  * weights[1] +
             p2  * weights[2] +
            tan2 * weights[3]; 
            

        correct(sum);
        return sum;
    }
};
// ROBLOX
/**
 Interpolates a property according to a piecewise linear spline.  This provides
 C0 continuity but the derivatives are not smooth.  
 <P>
 Example:
 <CODE>
    const double times[] = {MIDNIGHT,               SUNRISE - HOUR,         SUNRISE,              SUNRISE + sunRiseAndSetTime / 4, SUNRISE + sunRiseAndSet
Time, SUNSET - sunRiseAndSetTime, SUNSET - sunRiseAndSetTime / 2, SUNSET,               SUNSET + HOUR/2,     DAY};
    const Color3 color[] = {Color3(0, .0, .1),      Color3(0, .0, .1),      Color3::black(),        Color3::black(),                   Color3::white() * .
25,         Color3::white() * .25,        Color3(.5, .2, .2),             Color3(.05, .05, .1),   Color3(0, .0, .1), Color3(0, .0, .1)};
    ambient = linearSpline(time, times, color, 10);
 </CODE>

  @param x         The spline is a function of x; this is the sample to choose.
  @param controlX  controlX[i], controlY[i] is a control points.  It is assumed
                   that controlX are strictly increasing.  XType must support
                   the "<" operator and a subtraction operator that returns
                   a number.
  @param controlY  YType must support multiplication and addition.
  @param numControl The number of control points.
 */
template<class XType, class YType>
YType linearSpline(double x, const XType* controlX, const YType* controlY, int numControl) {
    debugAssert(numControl >= 1);

    // Off the beginning
    if ((numControl == 1) || (x < controlX[0])) {
        return controlY[0];
    }

    for (int i = 1; i < numControl; ++i) {
        if (x < controlX[i]) {
            const double alpha = (double)(controlX[i] - x) / (controlX[i] - controlX[i - 1]);
            return controlY[i] * (1 - alpha) + controlY[i - 1] * alpha;
        }
    }

    // Off the end
    return controlY[numControl - 1];
}


template<class YType> YType cyclicCatmullRomSpline(
    double       t,
    const YType* controlY,
    int          numPoints) {

    debugAssert(numPoints >= 3);

    t = wrap(t, numPoints);

    // Find the indices of adjacent control points    
    int i = iFloor(t);

    // Compute the distance from the control point
    t = t - i;

    // Shift back one point for correct indexing
    i += numPoints - 1;

    // Pick up four control points
    const YType& P0 = controlY[(i + 0) % numPoints];
    const YType& P1 = controlY[(i + 1) % numPoints];
    const YType& P2 = controlY[(i + 2) % numPoints];
    const YType& P3 = controlY[(i + 3) % numPoints];

    return 0.5 * ((2 * P1) +
                  (-P0 + P2) * t +
                  (2*P0 - 5*P1 + 4*P2 - P3) * t*t +
                  (-P0 + 3*P1- 3*P2 + P3) * t*t*t);
}
/**
 A cubic spline with regularly spaced 
 control points.  The spline interpolates
 the control points.  The spline
 will wrap from the last point back to the first.

 The t parameter is on the range [0, controlY.size()],
 where integers correspond to control points exactly.

 @cite http://www.mvps.org/directx/articles/catmull/
*/
template<class YType> YType cyclicCatmullRomSpline(
    double       t,
    const Array<YType>&  controlY) {

    int numPoints = controlY.size();
    return cyclicCatmullRomSpline(t, controlY.getCArray(), numPoints);
}

// =============================

}

#endif
