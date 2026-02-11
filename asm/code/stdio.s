%func exit {
    v0 = syscall.exit
    syscall
}

%func string_len {
    push s0
    s0 = 0
    %label loop {
        t0b = byte [a0 + s0]


        jump label return if t0b == 0
        add s0, 1
        jump label loop
        %label return {
            v0 = s0
            ret
        }
    }
    pop s0
}

%func print {
    call string_len
    push v0
    push a0
    v0 = syscall.write
    a0 = 1
    pop a1
    pop a2
    syscall
    ret
}

%func write {
    push a0
    call string_len
    push v0
    push a0
    v0 = syscall.write
    pop a1
    pop a2
    pop a0
    syscall
}

    ret
%func println {
    call print
    %local newline, 2
    byte [newline] = 10
    byte [newline + 1] = 0
    a0 = &newline
    call print
}

%func itoa {
    s0 = 0
    %label convert {
        t0 = a0
        mod t0, 10
        add t0, 48
        push t0
        add s0, 1
        div a0, 10
        jump label convert if a0 > 0
    }

    s1 = a1
    s2 = 0

    %label write {
        pop t1
        byte [s1 + s2] = t1b
        add s2, 1
        sub s0, 1
        jump label write if s0 > 0
    }
    byte [s1 + s2] = 0
    ret
}

%func and {
    jump label is_false if a0 == 0
    jump label is_false if a1 == 0
    %label is_true {
        v0 = 1
        ret
    }
    %label is_false {
        v0 = 0
        ret
    }
}

%func or {
    jump label is_true if a0 != 0
    jump label is_true if a1 != 0
    %label is_false {
        v0 = 0
        ret
    }
    %label is_true {
        v0 = 1
        ret
    }
}

%func malloc {
    s0 = a0
    v0 = syscall.mmap
    a0 = 0
    a1 = s0
    a2 = 7
    a3 = 4098
    a4 = 0
    sub a4, 1
    a5 = 0
    syscall
}

%byte errorstring_1 = "File "
%byte errorstring_2 = " could not be found\n"


%func error {
    push a0
    a0 = &errorstring_1
    call print
    pop a0
    call print
    a0 = &errorstring_2
    call print
    a0 = 43
    call exit
}

%func open {
    %local fd, 8

    push a0
    v0 = syscall.open
    a1 = 0
    a2 = 0
    syscall
    qword [fd] = v0

    a0 = v0 >= 0
    a1 = v0 <= 2

    call and
    pop a0

    call error if v0

    v0 = qword [fd]
}

%func close {
    v0 = syscall.close
    syscall
}

%func read {
    v0 = syscall.read
    syscall
}
