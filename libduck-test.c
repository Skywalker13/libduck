
#include <stdlib.h>
#include <stdio.h>

#include <duck.h>


int
main (int argc, char **argv)
{
  duck_t *handle;

  if (argc < 2)
    return -1;

  duck_verbosity (DUCK_MSG_VERBOSE);

  handle = duck_init (NULL);
  if (!handle)
    return -1;

  duck_load (handle, argv[1]);

  duck_uninit (handle);
  return 0;
}
