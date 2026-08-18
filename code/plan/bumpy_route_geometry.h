#ifndef BUMPY_ROUTE_GEOMETRY_H
#define BUMPY_ROUTE_GEOMETRY_H

#include <math.h>
#include <stdint.h>

/*
 * Builds the vehicle-right unit vector while constraining it to the normal of
 * the route-table entry-to-exit segment.  A right-side lateral measurement is
 * therefore projected onto the route normal without changing its vehicle-frame
 * sign when the vehicle traverses the segment in reverse.
 */
static inline uint8_t BumpyRouteGeometry_GetVehicleRightUnit(float entry_x_mm,
                                                             float entry_y_mm,
                                                             float exit_x_mm,
                                                             float exit_y_mm,
                                                             float relative_yaw_deg,
                                                             float min_abs_alignment,
                                                             float *right_x,
                                                             float *right_y)
{
    const float route_dx = exit_x_mm - entry_x_mm;
    const float route_dy = exit_y_mm - entry_y_mm;
    const float route_len_sq = route_dx * route_dx + route_dy * route_dy;
    const float heading_rad = relative_yaw_deg * (3.14159265358979f / 180.0f);
    float route_inv_len;
    float route_forward_x;
    float route_forward_y;
    float alignment;
    float direction_sign;

    if ((right_x == 0) || (right_y == 0) || (route_len_sq <= 1.0e-6f))
    {
        return 0U;
    }

    route_inv_len = 1.0f / sqrtf(route_len_sq);
    route_forward_x = route_dx * route_inv_len;
    route_forward_y = route_dy * route_inv_len;
    alignment = cosf(heading_rad) * route_forward_x +
                sinf(heading_rad) * route_forward_y;

    if (fabsf(alignment) < min_abs_alignment)
    {
        return 0U;
    }

    direction_sign = (alignment >= 0.0f) ? 1.0f : -1.0f;
    *right_x = direction_sign * route_forward_y;
    *right_y = -direction_sign * route_forward_x;
    return 1U;
}

#endif
