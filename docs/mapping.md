# Mapping algorithm notes

## Current versions

- `v1.0.0`: node-to-node nearest mapping searches every grid cell covered by the user search distance.
- `v1.1.0`: node-to-node mapping distributes each NASTRAN nodal force to the nearest ADX surface nodes with inverse-distance weights.
- `v1.2.0`: adds the first conservative surface-to-surface transfer path in `SurfaceFaceMap`.
- `v1.3.0`: improves `SurfaceFaceMap` with three integration samples per ADX triangle.
- `v1.4.0`: adaptively subdivides each ADX triangle and samples every subtriangle centroid.

## v1.2 surface transfer

`SurfaceFaceMap` maps by ADX surface triangle:

1. Build ADX surface triangles from selected ADX element sets.
2. Use each ADX triangle centroid as the integration sample point.
3. Find the nearest NASTRAN shell element centroid within the search distance.
4. Average the source NASTRAN element nodal pressure vectors.
5. Compute `face_force = pressure * adx_face_area * 1000 * ratio`.
6. Distribute the face force to the three ADX triangle vertices.

This is a centroid-sampled conservative surface-to-surface remapping foundation.
It handles both coarse-to-fine and fine-to-coarse mesh density differences better
than node nearest-neighbor transfer because the target ADX area participates in
the mapped load.

## Next step

For stricter conservation when ADX and NASTRAN surfaces have different local
areas or shifted boundaries, replace the centroid match with face-to-face overlap
or common-refinement area weights.

## v1.3 three-point triangle sampling

`SurfaceFaceMap` now samples each ADX triangle at three barycentric positions:

- `(2/3, 1/6, 1/6)`
- `(1/6, 2/3, 1/6)`
- `(1/6, 1/6, 2/3)`

Each sample owns one third of the ADX triangle area. The mapper finds the nearest
NASTRAN shell element for each sample point, converts that source pressure into a
sample force, then distributes the sample force back to the ADX triangle vertices
using the same barycentric weights.

Compared with the `v1.2.0` centroid-only sample, this reduces false matches when
one ADX triangle crosses multiple NASTRAN triangles and gives smoother nodal force
distribution on larger ADX faces.

## v1.4 adaptive subtriangle sampling

`SurfaceFaceMap` now estimates the local mesh-density difference for each ADX
surface triangle:

```text
area_ratio = adx_face_area / nearest_nastran_element_area
subdivision_count = ceil(sqrt(area_ratio))
```

The subdivision count is clamped to `2..10`. A count of `n` splits the ADX
triangle into `n * n` small triangles. Each small triangle contributes one
centroid sample with area:

```text
sample_area = adx_face_area / (n * n)
```

Each sample point independently finds its nearest NASTRAN shell element, converts
that element pressure into a sample force, and distributes the force to the ADX
triangle nodes using the sample centroid barycentric weights.

This improves cases where one large ADX triangle covers many small NASTRAN
triangles because the mapper can now sample multiple source pressure regions
inside the same target ADX triangle.
