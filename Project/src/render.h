#pragma once

#include "global.h"
#include "image.h"
#include <memory>

struct Scene;

Image3 bsdf_renderer(const Scene &scene);
