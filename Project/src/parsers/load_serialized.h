#pragma once

#include "global.h"
#include "matrix.h"
#include "shape.h"

/// Load Mitsuba's serialized file format.
TriangleMesh load_serialized(const fs::path &filename,
                             int shape_index,
                             const Matrix4x4 &to_world);
