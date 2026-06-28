# Suggested usage

Use these inputs in the VectorMapping UI:

- ADX file: `adx_large_triangle.adx`
- NASTRAN file: `nastran_fine_triangles.nas`
- Normal pressure file: `normal_pressure.dat`
- Tangent pressure file: `tangent_pressure_zero.dat`
- Search distance: `80`
- Mapping ratio: `1.0`
- ADX element set: `Surface_1`

Expected behavior:

- The app should read one ADX element set named `Surface_1`.
- The NASTRAN model should read 9 nodes and 8 triangular elements.
- The upper-right NASTRAN pressure region is higher, so mapped ADX force should
  not be uniform across all output nodes.
- With v1.4, large ADX faces should be sampled with adaptive subtriangles
  instead of only one or three sample points.
