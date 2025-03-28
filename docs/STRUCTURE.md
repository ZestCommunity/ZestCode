# Project Structure

Looking at the file structure of a project like this can feel intimidating. This document describes the structure of this project, so you know where to find stuff.

- `.github` contains files used by GitHub to perform some actions
  - `.github/ISSUE_TEMPLATE` contains the templates that are used for new [issues](https://github.com/ZestCommunity/ZestCode/issues)

- `docs` contains documentation and documentation generation config files

- `include` contains the header files
  - `include/common` headers used in various parts of the project
  - `include/pros` headers that are distributed to user projects
  - `include/rtos` headers for the scheduler (FreeRTOS)
  - `include/system` headers for low-level system functionality
    - `include/system/dev` headers for serial I/O and file management
    - `include/system/user_functions` a horrifying mess that should be destroyed
  - `include/vdml` headers for the VEX Data Management Layer (VDML), a system to ensure thread-safety when interacting with VEX devices

- `scripts` contains scripts used for building ZestCode and projects that use ZestCode

- `src` contains the source files
  - `src/common` sources defining symbols used throughout the project
  - `src/devices` implementations of VEX device abstractions. Files prefixed with `vdml_` indicate that the file makes use of the VDML
  - `src/rtos` sources, build scripts, and misc files of FreeRTOS
  - `src/system` sources for low-level system functionality
    - `src/system/dev` sources for serial I/O and file management

- `subprojects` contains wrap files for dependencies

- `tests` contains unit tests
  - `tests/examples` contains example code