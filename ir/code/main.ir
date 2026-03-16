%include "code/stdio.s"


%func main {
    s0 = a0
    s1 = a1


/*
    %local string, 100
    a0 = s0
    a1 = &string
    call itoa
    a0 = a1
    call println
*/

    %local i, 8
    qword [i] = 1

    %local file, 8
    a0 = 100
    call malloc
    qword [file] = v0

    %label loop {
        s2 = qword [i]
        mul s2, 8
        a0 = qword [s1 + s2]
        jump labelend loop if a0 == 0

        qword [file] = a0

        t0 = qword [i]
        add t0, 1
        qword [i] = t0

        jump label loop
    }


    a0 = qword [file]
    call open

    %local fd, 8
    qword [fd] = v0


    %local string, 8

    a0 = 1000
    push a0
    call malloc
    qword [string] = v0

    pop a2
    a0 = qword [fd]
    a1 = &string
    call read

    s0 = &string
    byte [s0 + v0] = 0

    call close

    // a0 = &string
    // call print

    %label exit {
        a0 = 0
        call exit
    }
}
