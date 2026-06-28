# v1.4 adaptive sampling test data

This folder contains a small synthetic mapping case for testing the v1.4
adaptive subtriangle sampler.

## Files

- `adx_large_triangle.adx`: one ADX 10-node quadratic tetrahedron. Its surface
  contains large triangles.
- `nastran_fine_triangles.nas`: a 100 x 100 square split into 8 CTRIA3 elements.
- `normal_pressure.dat`: nodal normal pressure vectors. Pressure increases
  toward the upper-right corner.
- `tangent_pressure_zero.dat`: zero tangent pressure vectors.
- `expected_usage.md`: suggested UI inputs and what to inspect.

## Purpose

The ADX surface triangle area is larger than each NASTRAN triangle area, so
v1.4 should subdivide the ADX face and sample multiple NASTRAN source regions.
