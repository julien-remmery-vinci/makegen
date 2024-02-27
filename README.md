# makegen

## QuickStart

**1. Compile**

User your favorite compiler.

```shell
gcc makegen.c
```

...or give it a prefered name:

```shell
gcc -o makegen makegen.c
```

**2. Make it accessible**

Place the generated executable in a file which path is contained in PATH env variable.

See the already added directories.
```shell
echo $PATH
```

...Or:

Add the path to your executable to the PATH env variable.

```shell
export PATH=$PATH:/directory/containing/your/executable
```

If you want to make it permanent, add the command in your ~/.bashrc file.