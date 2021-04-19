//
// Created by lukebayes on 4/19/21.
//

#ifndef MAPLE_STITCHER_H
#define MAPLE_STITCHER_H

typedef struct StitcherContext {
}StitcherContext;

StitcherContext *stitcher_new(void);
int stitcher_connect(StitcherContext *c);
int stitcher_start(StitcherContext *c);
void stitcher_free(StitcherContext *c);

#endif // MAPLE_STITCHER_H
