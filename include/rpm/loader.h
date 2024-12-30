//
// Dynamic Loader Interface.
//
#ifndef RPM_LOADER_H
#define RPM_LOADER_H

#ifdef __cplusplus
extern "C" {
#endif

//
// Dynamic object structure.
//
typedef struct {
    //TODO
} dyn_object_t;

//
// Load dynamic binary.
// Return true on success.
//
bool dyn_load(dyn_object_t *dynobj, const char *filename);

//
// dyn_symbol symtab[] = { ... };
// dyn_bind(&dynobj, symtab);
// dyn_run(&dynobj, argc, argv);
// dyn_call(&dynobj, name, argc, argv);
// dyn_locate(&dynobj, name) -> pointer
//

#ifdef __cplusplus
}
#endif

#endif // RPM_LOADER_H
