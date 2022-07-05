#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h> //for fork()
#include <sys/wait.h> //for wait()
#include <string.h>

#include "my_ipc.h"
#include "my_ghostbuffer.h"
#include "my_cache.h"
#include "cpff_parameter.h"
#include "my_structure.h"