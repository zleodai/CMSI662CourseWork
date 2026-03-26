# Secure Stack Test Guide

This project contains secure stack implementations and test suites for C, C++, and Java.

## Layout

- `c/`
- `cpp/`
- `java/`

## Requirements

- GCC / G++ from MSYS2 MinGW64
- Java JDK 23 or newer
- PowerShell on Windows

If GCC or G++ is not available in your current shell, load your PowerShell profile first:

```powershell
. $PROFILE
```

## C

Compile the C stack and test file:

```powershell
gcc -std=c23 -Wall -Wextra -Werror .\c\SecureStack.c .\c\SecureStackTester.c -o .\c\C_Stack_Tests.exe
```

Run the C tests:

```powershell
.\c\C_Stack_Tests.exe
```

## C++

Compile the C++ stack and test file:

```powershell
g++ -std=c++20 -Wall -Wextra -Werror .\cpp\SecureStack.cpp .\cpp\SecureStackTester.cpp -o .\cpp\CPP_Stack_Tests.exe
```

Run the C++ tests:

```powershell
.\cpp\CPP_Stack_Tests.exe
```

## Java

Compile the Java stack and test file:

```powershell
javac .\java\SecureStack.java .\java\SecureStackTester.java
```

Run the Java tests directly with the JVM:

```powershell
java -cp .\java SecureStackTester
```

Run the packaged Java executable:

```powershell
.\java\JAVA_Stack_Tests\JAVA_Stack_Tests.exe
```

## Notes

- The C and C++ tests are built as native executables in their respective folders.
- The Java folder contains source files plus a packaged Windows app image.
- If you rebuild Java and want a fresh packaged executable, you must re-run the packaging step separately.
