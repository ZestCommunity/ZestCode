# ZestCode Styleguide

> [!TIP]
> ZestCode is in the process of migrating to this new style; some older code may not yet be using this new style.

Maintaining a consistent coding style is important. It helps make the code more readable. ZestCode's style guide is largely inspired by [rustfmt](https://doc.rust-lang.org/stable/style-guide/).

## General Rules

> [!TIP]
> The [clangd VSCode extension](https://marketplace.visualstudio.com/items?itemName=llvm-vs-code-extensions.vscode-clangd) can be used to automatically format your code using the ZestCode styleguide

Some basic rules are as follows:

- use [fixed width integer types](https://en.cppreference.com/w/cpp/types/integer) instead of `int` or `unsigned int`
  - don't use `int_fast_t` or `int_smallest_t` unless you document why its necessary
- indent using 4 spaces, not tabs
- no more than 100 columns per line
- write tests for your code
- always use `enum class` over `enum`
- a `struct` should have only public member variables and no methods
- don't use `typedef`, use `using` instead
- use [C++11 attributes](https://en.cppreference.com/w/cpp/language/attributes) instead of the older GCC attribute syntax: `__attribute__((...))`
- only use macros as a last resort

For more details, see [.clang-format](../.clang-format).

## File extensions

> [!IMPORTANT]
> ZestCode is being rewritten in C++. No C files or headers may be added, but existing ones can be modified.

- C source files will have the extension `.c`
- C++ source files will have the extension `.cpp`
- C header files will have the extension `.h`
- C++ header files will have the extension `.hpp`
- Assembly source files will have the extension `.s`
- Linkerscripts will have the extension `.ld`
- Markdown documents will have the extension `.md`
- Python files will have the extension `.py`

## Naming Conventions

- **files, folders** - snake_case
- **classes, structs, enums, unions, concepts, etc** - PascalCase
- **functions** - snake_case
- **variables** - snake_case
- **member variables** - m_snake_case (The `m_` is intentional)
- **macros** - SCREAMING_SNAKE_CASE
- **compile-time constants** - SCREAMING_SNAKE_CASE
- **enum members** - snake_case

## Documentation

Use Doxygen-style comments as shown in the sections below.

### Enum-Level Comments

These should be placed immediately before the declaration of the enum.

```c++
/*
 * @brief Short description of the enum
 *
 * Extended description of the enum goes here. This should explain general usage
 * patterns for the enum.
 */
enum class MyEnum {
        member_0, ///< short description of member 0 goes here.
        member_1, ///< these can be omitted if it's painfully obvious
        member_2, ///< what each is for, or if there are just so many of
        member_3, ///< them it doesn't make practical sense to
        member_4, ///< document them all.
}
```

> [!TIP]
> In the above example, the comments describing each member of the `enum` run together and form complete sentences for effect.
> Please do not do this in your code!

### Function-Level Comments

These should be placed immediately before the function prototype they are describing in a header file.

```c++
/**
 * @brief Brief description of the function.
 *
 * An extended description of the function (if applicable).
 *
 * This function uses the following values of errno when an error state is
 * reached:
 * ERRNO_VALUE - Description of what causes this error
 *
 * @tparam T the type of the parameters
 *
 * @param parameter_name The parameter description
 * @param other_parameter_name The parameter description
 *
 * @return The description of the return value, if this is longer than one line
 * then it will wrap around under the return statement
 *
 * @code
 * // example code here
 * @endcode
 *
 * @see src/referenced_file.cpp
 */
```

### Implementation Comments

Sometimes it is necessary to explain statement or series of statements.
In this case, comment it! Prefer adding comments before the offending code,
rather than next to it.
Add as much documentation as you think is necessary, rather too much than too little.

For example:

```c++
// Program entrypoint. This is the first function that is run.
// It sets up memory, initializes libc, and then calls main
extern "C" [[gnu::section(".boot")]]
void _start() {
    // Symbols provided by the linker script
    extern uint32_t __bss_start;
    extern uint32_t __bss_end;
    extern uint32_t __sbss_start;
    extern uint32_t __sbss_end;
    // Don't try refactoring this code with stuff like std::fill or std::span.
    // It's been tried before, and it causes UB.
    // It's suspected that this is due to libc not being initialized yet.
    for (uint32_t* bss = &__bss_start; bss < &__bss_end; bss++)
        *bss = 0;
    for (uint32_t* sbss = &__sbss_start; sbss < &__sbss_end; sbss++)
        *sbss = 0;

    // Initialize libc
    __libc_init_array();

    // call the main function, which sets up the scheduler
    main();
}
```

> [!TIP]
> commenting out lines is fine during testing, but should be removed when you mark your PR as ready for review!

#### Notes to Other Developers (Or Yourself)

When writing code, it can sometimes be useful to leave notes to other developers or to yourself in the future. Examples of these include:

- `// TODO: something that should be done`
- `// HACK: used to describe a particularly nasty way of solving a problem-- could be improved, but it works for now`

While it is not strictly necessary to use these keywords in comments, they can be helpful - modern editors either highlight some of these keywords by default or have extensions that do.
This can make certain comments stand out even more when developers are "grepping" the codebase (visually or otherwise).