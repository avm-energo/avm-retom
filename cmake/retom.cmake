include(FetchContent)
FetchContent_Declare(libavm-retom
  GIT_REPOSITORY https://git.avmenergo.ru/avm-energo/libavm-retom.git
  GIT_TAG        master
  EXCLUDE_FROM_ALL)
FetchContent_MakeAvailable(libavm-retom)
