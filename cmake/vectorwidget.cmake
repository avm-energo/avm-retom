include(FetchContent)
FetchContent_Declare(libavm-vectorwidget
  GIT_REPOSITORY https://git.avmenergo.ru/avm-energo/libavm-vectorwidget.git
  GIT_TAG        main
  EXCLUDE_FROM_ALL)
FetchContent_MakeAvailable(libavm-vectorwidget)
