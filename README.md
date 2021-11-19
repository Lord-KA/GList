# GList
This is a generalized linked list data structure

## Building
```bash
$ mkdir build
$ cd build
$ cmake .. -DCMAKE_BUILD_TYPE=Release
$ make
```

## Using in your project
Add code below to your CMakeLists.txt and include "glist.h"
```
FetchContent_Declare(
  glist
  GIT_REPOSITORY https://github.com/Lord-KA/gList.git
  GIT_TAG        release-1.X
)
if(NOT glist_POPULATED)
  FetchContent_Populate(glist)
  include_directories(${glist_SOURCE_DIR})
endif()
```
You have to pre-define `GLIST_TYPE` with macro or `typedef` before including the header

## DONE
1. Basic linked list
2. Utility ObjPool data structure
3. GraphViz dumps
4. Unit tests
5. CMake with fetching

## TODO
1. Capybara ASCII art
2. Better unit test
3. Test coverage check
4. Github CI
