// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#include <vespa/fastos/fastos.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vespa/vespalib/util/host_name.h>

int
main(int, char **)
{
  printf("slobrok[2]\n");
  printf("slobrok[0].connectionspec tcp/%s:18524\n", vespalib::HostName::get().c_str());
  printf("slobrok[1].connectionspec tcp/%s:18525\n", vespalib::HostName::get().c_str());

  return 0;
}
