Pre-build
---

+ rule

    - Create your folder by component name
    - Put the header files to `include` folder
    - Put the per-build lib files (*.a) to root of prebuild folder

    ```
    prebuild
    ├── [your component]
    │    └── include
    │         └── *.h
    └── lib[your component].a
    ```
