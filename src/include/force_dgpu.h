#ifndef BADGL_FORCE_DGPU_H
#define BADGL_FORCE_DGPU_H

#include "defines.h"

/**
 * This will force use of dedicated GPU if available on
 * both Nvidia and AMD (hopefully works cross platform idk) 
 */
BADGL_EXPORT unsigned int NvOptimusEnablement = 1;
BADGL_EXPORT int AmdPowerXpressRequestHighPerformance = 1;

#endif