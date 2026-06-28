# Naming Rules

This project is moving from legacy MFC-style names to a smaller modern C++ style.

## Scope

- Keep MFC framework classes and generated handlers in their current style for now.
  - Example: `CVectorMappingDlg`, `OnBnClicked...`, resource IDs.
- Apply the new rules first to domain code:
  - ADX model data
  - Nastran model data
  - surface-node mapping
  - spatial lookup/grid code

## Classes And Types

- Use `PascalCase`.
- Do not add a `C` prefix to new domain classes.
- Prefer names that describe the role, not the file format only.
  - Good: `SpatialGrid`, `SurfaceNodeMap`, `NastranModel`
  - Avoid: `CArea`, `m_vNode`

## Methods

- Use `PascalCase` for public methods while the code remains MFC-adjacent.
- Prefer verb phrases.
  - Good: `Build`, `Clear`, `FindNearestNode`, `ExportAdxForce`
  - Avoid: `MakeAreaMap`, `RemoveAll`

## Variables And Members

- New local variables use `snake_case`.
- New private members use trailing underscore.
  - Example: `nodes_`, `area_size_`
- Existing public legacy members may be renamed gradually when call sites are updated.
- Avoid Hungarian-style prefixes in new code:
  - Avoid: `m_vNode`, `m_mNodeIDtoIndex`, `p_node`
  - Prefer: `nodes_`, `node_id_to_index_`, `node`

## Ownership

- Use value containers when possible.
  - Example: `std::vector<GridCell>` instead of `std::vector<GridCell*>`
- Use `std::unique_ptr<T>` only when pointer stability or polymorphism is needed.
- Use raw pointers only for non-owning references.

## Refactoring Policy

- Rename in small vertical slices and keep each version buildable.
- Keep compatibility wrappers temporarily when a rename would otherwise touch unrelated UI code.
- Each version tag should describe the completed slice, for example `v0.2.0`.

## Versioned Slices

- `v0.2.0`: Introduced `SpatialGrid`/`SpatialGridCell` and moved surface-node storage to `std::unique_ptr`.
- `v0.3.0`: Renamed primary ADX, Nastran, and surface mapping domain classes, plus first dialog-owned model/path members.
- `v0.4.0`: Aligned physical source/header filenames with the renamed domain classes.
- `v0.5.0`: Renamed primary model, node, element, set, and force/log data members to the trailing-underscore convention.
- `v0.6.0`: Renamed domain methods to clearer verb phrases while leaving MFC event handlers in their generated style.
- `v0.7.0`: Hid model-owned containers behind accessors and split Nastran reading/force calculation into private helpers.
- `v0.8.0`: Converted Nastran element types to `enum class` and split ADX state-machine parsing into private helpers.
- `v0.9.0`: Split multi-class headers and sources so model, element, node, and face classes live in focused file pairs.
