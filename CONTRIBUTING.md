# Contributing

We are open to pull requests.
Before making one,
please ensure that your code:

* is tested and doesn't break any existing tests
* produces no errors or warnings
* is documented with Doxygen commands
* meets our style requirements (see below)

## Style

### Code

General rules:

* Favour safety in syntax.
  Avoid the sorts of syntactical shortcuts
  which can lead to errors like [`goto fail;`][goto-fail].
* Variable name lengths should scale by scope.
  Single-character or short names for short scopes;
  one- or two-word names for scopes
  which fit on the screen at once;
  and longer for module- and global-scope symbols.
  If you can see the entire context
  then it doesn't need to go in the name;
  conversely, if you can't see
  the entire context for the variable,
  then its name should help infer it.
* Fear not of line breaks.
  Insert line breaks before and after braces,
  between struct and enum members,
  and between function arguments
  where necessary.
* Code contributes to documentation.
  Annotate code with Doxygen commands
  and prefer to document any function subtleties
  which might be relevant to the user
  in the function header instead of
  in the body.

Technical pedantry:

* Use UTF-8 encoding for all files.
* Wrap comment and code lines at 79 characters.
  * Where possible to do so
    and still fit on the line,
    align when wrapping.
  * If line breaks must occur between function arguments,
    prefer to break between _all_ arguments,
    not just where strictly necessary
    to adhere to the line length limit.
  * Prefer to place operators
    at the beginning of new lines,
    not trailing the previous line.
* Use Unix-style line endings (`\n`).
* Include a trailing newline
  at the end of all files.
* Use 4 spaces for indentation,
  and additional spaces for alignment.
* Use C99 syntax,
  but C89/ANSI C caution.
  This means, among other things:
  * No GNU extensions.
  * Use only compile-time constant-size arrays.
  * Always use specifically-sized types from `inttypes.h`
    (e.g., `uint8_t`, `int32_t`)
    instead of platform-specific types
    (e.g., `unsigned char`, `int`)
    when dealing with data.
  * Prefer specifically-sized types elsewhere, too.
  * Use C-style (`/* ... */`) comments,
    not C++-style comments (`// ...`).
* Use UpperCamelCase type names
  and camelCase variable names.
  Use TypeName_functionName for the names
  of method-like functions,
  and camelCase the names
  of freestanding functions.
* Keep `*` and `&` type modifiers and operators
  immediately adjacent to the affected variable,
  not separated by whitespace.
* Use spaces around operators
  and after control statements (`if`/`for`/`while`),
  but not inside parentheses.
* Everything gets braces.
  No one-line `if`, `for`, `while`, etc. statements:
  put the body in braces
  so that it's obvious what it is
  and where it starts and ends.
* Braces get newlines
  (except for `do...while`).
* Generally, everything that can have a Doxygen comment
  should have a Doxygen comment
  (typedefs, macros, structs, enums, functions, etc.).
  * In cases where the declaration and definition are separate
    (e.g., a function declaration in a header
    versus its implementation),
    only the declaration need be commented.
  * Use C/C++-style Doxygen commands (`\​command`),
    not Javadoc-style commands (`@​command`).
  * Files must use the `\​file` command
    (or else no documentation gets generated!).
  * Everything except enum members
    must have at least a `\​brief` comment.
    * Enum members can just have a comment:
      brief comments are handled no differently.
  * Functions must have at least appropriate `\​param`
    and `\​return` comments.

A short example:

~~~{.c}
/*​*
 * \​brief A short description of the function.
 *
 * A longer description of the function.
 *
 * \param [in] instrument the instrument connection
 * \param [in] theseParametersAreAligned a description of the first parameter
 * \param [in] becauseTheyFitOnTheLine a description of the second parameter
 * \return a description of the return value
 */
uint32_t RBRInstrument_examplePrototype(RBRInstrument *instrument,
                                        uint32_t theseParametersAreAligned,
                                        uint32_t *becauseTheyFitOnTheLine);

uint32_t RBRInstrument_exampleOfAReallyLongFunctionName(
    RBRInstrument *instrument,
    uint32_t theseParametersAreAllWrappedAndBroken,
    uint32_t becauseTheyWouldPushPastTheLineLengthLimit,
    uint32_t evenIf,
    uint8_t *someWouldFit,
    size_t onTheSameLine)
{
    size_t i;

    for (i = 0; i < onTheSameLine; i++)
    {
        while (*someWouldFit)
        {
            someWouldFit++;
        }
    }
}
~~~

If you have [Uncrustify] installed,
you can invoke indent.sh
to make an automatic attempt
at code indentation.
The auto-indentation configuration
leans toward the conservative side;
in particular, it performs no line wrapping.

[goto-fail]: https://www.imperialviolet.org/2014/02/22/applebug.html
[ident]: https://git-scm.com/docs/gitattributes#__code_ident_code
[Uncrustify]: http://uncrustify.sourceforge.net/

### Documentation

#### Doxygen

As noted above, source code should be annotated
with Doxygen commands.
To make it easier to notice omissions,
most will result in a build error
when compiling the documentation.

#### Standalone Documentation Files

Source files for documentation external to the code
are edited with [Semantic Linefeeds].
We don't do this for comments in code,
as those will be consumed by end users from their editors
as well as in the Doxygen output.

[Semantic Linefeeds]: http://rhodesmill.org/brandon/2012/one-sentence-per-line/
