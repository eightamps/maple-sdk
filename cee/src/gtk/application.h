//
// Created by lukebayes on 4/20/21.
//

#ifndef MAPLE_APPLICATION_H
#define MAPLE_APPLICATION_H

//
// Created by lukebayes on 4/15/21.
//

#ifndef FIR_APPLICATION_H
#define FIR_APPLICATION_H

#define APP_TITLE_LEN 256

typedef struct Application {
  char title[APP_TITLE_LEN];
  void *native_app;
}Application;

Application *application_new(void);
int application_run(Application *app, int argc, char *argv[]);
void application_free(Application *app);

#endif //FIR_APPLICATION_H


#endif //MAPLE_APPLICATION_H
