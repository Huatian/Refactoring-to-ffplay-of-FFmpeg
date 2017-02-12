
#include "base.h"
#include "SDL/SDL.h"

void free_picture(Frame *vp)
{
     if (vp->bmp) {
         SDL_FreeYUVOverlay(vp->bmp);
         vp->bmp = NULL;
     }
}

