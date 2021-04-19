//
// Created by lukebayes on 4/19/21.
//

#ifndef MAPLE_STITCHER_H
#define MAPLE_STITCHER_H

#include <soundio/soundio.h>


typedef struct StitcherContext {
  struct SoundIo *soundio;
  struct SoundIoDevice *to_speaker;
}StitcherContext;

StitcherContext *stitcher_new(void);
int stitcher_init(StitcherContext *c);
int stitcher_start(StitcherContext *c);
void stitcher_free(StitcherContext *c);

#endif // MAPLE_STITCHER_H
