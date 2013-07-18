
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <duck.h>


static void
print_node (duck_t *handle)
{
  double start, stop;
  duck_value_t res;
  memset (&res, 0, sizeof (res));

  duck_node_getinfo (handle, DUCK_NODE_I_AUDIO_POS_START, &res);
  start = res.i / 1000.0;
  duck_node_getinfo (handle, DUCK_NODE_I_AUDIO_POS_STOP, &res);
  stop = res.i / 1000.0;

  duck_node_getinfo (handle, DUCK_NODE_S_AUDIO_URI, &res);
  printf ("    |- %s (%.3f-%.3f)\n",
          res.s ? res.s : "no audio file", start, stop);
}

int
main (int argc, char **argv)
{
  duck_t *handle;
  int smilpos, nodepos;

  if (argc < 2)
    return -1;

  duck_verbosity (DUCK_MSG_VERBOSE);

  handle = duck_init (NULL);
  if (!handle)
    return -1;

  duck_load (handle, argv[1], DUCK_FORMAT_AUTO);

  for (smilpos = 1, nodepos = 1;
       !duck_walk (handle, smilpos, nodepos);
       ++smilpos)
  {
    duck_value_t res;
    memset (&res, 0, sizeof (res));

    duck_smilnode_getinfo (handle, DUCK_SMILNODE_S_HEADER, &res);
    printf (" => %s\n", res.s);
    print_node (handle);

    for (nodepos = 2; !duck_walk (handle, smilpos, nodepos); ++nodepos)
      print_node (handle);

    nodepos = 1;
  }

  duck_uninit (handle);
  return 0;
}
