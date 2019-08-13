patch file
---

# Linux

    Switch to the target dircectory

+ `diff`

    - options

    ```
    Option  Description
    -r      Recursively compare any subdirectories found
    -u      Create a diff file in the unified format
    -N      Treat absent files as empty
    ```

    - example

    ```
    $ diff -Naur dir1 dir2 > diff.patch
    ```

+ `patch`

    + options

    ```
    Option          Description
    -p0             Apply the patch to the same directory structure as when the patch was created
    -R, –reverse    Undo changes
    ```

    - example

    ```
    $ patch -p0 < diff.patch
    $ patch -R -p0 OriginalFile < diff.patch
    ```

# Git

+ `format-patch`

    ```
    $ git format-patch sha-A aha-B
    ```

+ `am`

    ```
    $ git am diff.patch
    $ git am -3 diff.patch
    ```
