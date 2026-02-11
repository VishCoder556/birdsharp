(module
(import "env" "js_write" (func $js_write (param i32 i32 i32)))
(memory (export "memory") 1)
(data (i32.const 1024) "Hello\n")
(global $v0 (mut i32) (i32.const 0))
(export "v0" (global $v0))
(global $s0 (mut i32) (i32.const 0))
(export "s0" (global $s0))
(global $a3 (mut i32) (i32.const 0))
(export "a3" (global $a3))
(global $a2 (mut i32) (i32.const 0))
(export "a2" (global $a2))
(global $a1 (mut i32) (i32.const 0))
(export "a1" (global $a1))
(global $a0 (mut i32) (i32.const 0))
(export "a0" (global $a0))
(global $a4 (mut i32) (i32.const 0))
(export "a4" (global $a4))
(global $a5 (mut i32) (i32.const 0))
(export "a5" (global $a5))
(global $t0 (mut i32) (i32.const 0))
(export "t0" (global $t0))
(global $t1 (mut i32) (i32.const 0))
(export "t1" (global $t1))
(global $s1 (mut i32) (i32.const 0))
(export "s1" (global $s1))
(global $t2 (mut i32) (i32.const 0))
(export "t2" (global $t2))
(func $syscall_write
	global.get $a0
	global.get $a1
	global.get $a2
	call $js_write
)
(func $string_len
	(local $a i64)
	(local $i i32)
	block $lbl._LBC0
	br $lbl._LBC0
	end
	i32.const 1
	global.set $a0
	i32.const 1024
	global.set $a1
	i32.const 6
	global.set $a2
	call $syscall_write
)
(func $exit
	(local $ret i32)
	i32.const 1
	global.set $a0
	i32.const 1024
	global.set $a1
	i32.const 6
	global.set $a2
	call $syscall_write
)
(func $open
	(local $file i64)
	(local $details i32)
	(local $creat i32)
	i32.const 1
	global.set $a0
	i32.const 1024
	global.set $a1
	i32.const 6
	global.set $a2
	call $syscall_write
)
(func $close
	(local $fd i32)
	i32.const 1
	global.set $a0
	i32.const 1024
	global.set $a1
	i32.const 6
	global.set $a2
	call $syscall_write
)
(func $read
	(local $fd i32)
	(local $data i64)
	(local $size i32)
	i32.const 1
	global.set $a0
	i32.const 1024
	global.set $a1
	i32.const 6
	global.set $a2
	call $syscall_write
)
(func $write
	(local $fd i32)
	(local $str i64)
	call $string_len
	i32.const 1
	global.set $a0
	i32.const 1024
	global.set $a1
	i32.const 6
	global.set $a2
	call $syscall_write
)
(func $print
	(local $str i64)
	call $string_len
	i32.const 1
	global.set $a0
	i32.const 1024
	global.set $a1
	i32.const 6
	global.set $a2
	call $syscall_write
)
(func $printerr
	(local $str i64)
	call $string_len
	i32.const 1
	global.set $a0
	i32.const 1024
	global.set $a1
	i32.const 6
	global.set $a2
	call $syscall_write
)
(func $malloc
	(local $size i64)
	i32.const 1
	global.set $a0
	i32.const 1024
	global.set $a1
	i32.const 6
	global.set $a2
	call $syscall_write
)
(func $int_to_string
	(local $num i32)
	(local $str i64)
	(local $temp i32)
	(local $j i64)
	(local $end i64)
	(local $begin i64)
	(local $_temp i32)
	call $malloc
	br $lbl._LBC_END_0
	block $lbl._LBC_NEXT_1
	br $lbl._LBC_END_0
	end
	block $lbl._LBC_END_0
	end
	br $lbl._LBC_END_2
	block $lbl._LBC_NEXT_3
	br $lbl._LBC_END_2
	end
	block $lbl._LBC_END_2
	end
	block $lbl._LBC4
	br $lbl._LBC4
	end
	br $lbl._LBC_END_5
	block $lbl._LBC_NEXT_6
	br $lbl._LBC_END_5
	end
	block $lbl._LBC_END_5
	end
	block $lbl._LBC7
	br $lbl._LBC7
	end
	i32.const 1
	global.set $a0
	i32.const 1024
	global.set $a1
	i32.const 6
	global.set $a2
	call $syscall_write
)
(func $strcmp
	(local $str1 i64)
	(local $str2 i64)
	(local $str1len i32)
	(local $str2len i32)
	(local $i i32)
	call $string_len
	call $string_len
	block $lbl._LBC0
	br $lbl._LBC_END_1
	block $lbl._LBC_NEXT_2
	br $lbl._LBC_END_1
	end
	block $lbl._LBC_END_1
	end
	br $lbl._LBC0
	end
	i32.const 1
	global.set $a0
	i32.const 1024
	global.set $a1
	i32.const 6
	global.set $a2
	call $syscall_write
)
(func $main
	(local $argc i64)
	(local $argv i64)
	(local $i i64)
	(local $fd i32)
	(local $str i64)
	block $lbl._LBC0
	call $print
	call $int_to_string
	call $print
	call $print
	call $print
	call $print
	call $strcmp
	call $open
	call $exit
	br $lbl._LBC_END_3
	block $lbl._LBC_NEXT_4
	br $lbl._LBC_END_3
	end
	block $lbl._LBC_END_3
	end
	call $malloc
	call $read
	call $close
	call $print
	br $lbl._LBC_END_1
	block $lbl._LBC_NEXT_2
	br $lbl._LBC_END_1
	end
	block $lbl._LBC_END_1
	end
	call $strcmp
	call $open
	call $exit
	br $lbl._LBC_END_7
	block $lbl._LBC_NEXT_8
	br $lbl._LBC_END_7
	end
	block $lbl._LBC_END_7
	end
	call $write
	call $close
	br $lbl._LBC_END_5
	block $lbl._LBC_NEXT_6
	br $lbl._LBC_END_5
	end
	block $lbl._LBC_END_5
	end
	br $lbl._LBC0
	end
	call $exit
	i32.const 1
	global.set $a0
	i32.const 1024
	global.set $a1
	i32.const 6
	global.set $a2
	call $syscall_write
)
(export "_start" (func $main))
)