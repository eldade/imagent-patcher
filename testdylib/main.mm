#include <cstdio>

#include "mach_override.h"
#include "mach-o/dyld.h"
#include <CoreServices/CoreServices.h>


//#include <Foundation/Foundation.h>


// Used to track the pointer to victim code's function
long *victim_func_ptr;


// A function prototype so we can call the victim function from our override
// function.
boolean_t (*victim_func)(void *entitlement, int pid) = 0;


// Our override function
boolean_t my_print_hw(void *entitlement, int pid)
{
    // Our hijacked code ...
//    NSLog(@"testlib: heloWRLD %@ %x\n", entitlement, pid);
    
    // Optionally, we can still call the original function, which is useful and
    // even necessary in some case ...
   // return (*victim_func)(entitlement, pid);
   // NSLog(@"original returned %d", result);
    return 1;
}


void install(void) __attribute__ ((constructor));
void install()
{
    printf("testlib: install\n");
    
    // Use this to discover a pointer to the function we want to hijack
    _dyld_lookup_and_bind(
                          "_IMDAuditTokenTaskHasEntitlement",
                          (void**) &victim_func_ptr,
                          NULL);
    
//    if (victim_func_ptr == NULL)
//        NSLog(@"whoa, couldn't find _IMDAuditTokenTaskHasEntitlement\n");
//    else
//        NSLog(@"found _IMDAuditTokenTaskHasEntitlement!!\n");
    
    //TODO check for bad victim_func_ptr
    
    // Assign our long pointer to our function prototype
    victim_func = (boolean_t (*)(void *entitlement, int pid))victim_func_ptr;
    
//    NSLog(@"testlib: victim_func_ptr   = %ld\n", (long)victim_func_ptr);
//    NSLog(@"testlib: victim_func       = %ld\n", (long)victim_func);
    
    // Do the override
    mach_error_t me;
    
    me = mach_override_ptr(
                           victim_func_ptr,
                           (void*)&my_print_hw,
                           (void**)&victim_func);
    
}
