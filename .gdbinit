define kern
  symbol out/xk.elf
end

define sh
  symbol out/user/_sh
end

define init
  symbol out/user/_init
end

define lab1test
  symbol out/user/_lab1test
end

define lab2test
  symbol out/user/_lab2test
end

define lab3test
  symbol out/user/_lab3test
end

define lab4test_a
  symbol out/user/_lab4test_a
end

define lab4test_b
  symbol out/user/_lab4test_b
end

define initcode
  symbol out/initcode.out
end

define pass_sc
  while ($cs == 8)
    ni
  end
end

set arch i386:x86-64:intel
target remote localhost:25576
symbol out/xk.elf
b main
c
disconnect
set arch i386:x86-64
target remote localhost:25576
c
