add_definitions(-Wextra -Wall -ansi -pedantic -std=c99)
add_definitions(-g -O2)
add_definitions(-D_GNU_SOURCE)

include_directories(.)

aux_source_directory(. dotSources)
aux_source_directory(config configSources)
aux_source_directory(log logSources)
aux_source_directory(parser parserSources)
aux_source_directory(prover proverSources)
aux_source_directory(str strSources)
aux_source_directory(tree treeSources)

set(commonSources
    ${dotSources}
    ${configSources}
    ${logSources}
    ${parserSources}
    ${proverSources}
    ${strSources}
    ${treeSources}
)

set(CMAKE_THREAD_PREFER_PTHREAD true)
find_package(Threads)

add_executable(contour ${commonSources})
