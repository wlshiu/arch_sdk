Pre-build
---

+ rule

    - Create your folder by component name
    - Put the header files to `include` folder
    - Put the per-builded lib files (*.a) to root of `prebuild` folder

    ```
    prebuild
    ├── [your-component-folder]
    │    └── include
    │         └── *.h
    └── lib[your-component].a
    ```
