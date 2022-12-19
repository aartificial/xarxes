project(server C)

set(CMAKE_C_STANDARD 99)

add_executable(server UEBp1-ser.c UEBp1-aUEBs.c UEBp1-tTCP.c UEBp1-tTCP.h)
add_executable(client UEBp1-cli.c UEBp1-aUEBc.c UEBp1-tTCP.c UEBp1-tTCP.h UEBp2-aDNSc.c UEBp2-aDNSc.h)