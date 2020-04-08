# Tools

## Toolchain Setup

### Mac Users

Requirement: Xcode >= 8.0.

brew tap xiw/crossbuild

brew install x86_64-linux-gnu-binutils

brew install x86_64-linux-gnu-gdb

### 64-bit Linux Users

Native toolchain works.

## Building QEMU

You need QEMU >= 2.9. To build it from source code please consult [QEMU instruction manual](https://www.qemu.org/download/#source).

QEMU requires libsdl. This should not be a problem with Mac users. For 64-bit Linux users, you might need to install it.

For Ubuntu, you can do the followings:

sudo apt-get install libsdl2-dev

## Source Control
We use Git for source control. For more information about how to use Git, please consult [Git user's manual](http://www.kernel.org/pub/software/scm/git/docs/user-manual.html).

## Debugging
We use GDB as a remote debugger for xk in QEMU. To attach GDB to xk, you need to open two separate terminals. Both should be in the xk root directory. In one terminal, type "make qemu-gdb". This starts the qemu process and wait for GDB to attach. In another terminal, type "make gdb". Now the GDB process is attached to qemu.
