# system1

## Task

1. Fix the code so that all tests pass.
2. add a document that explains what the code does

## Constraints

- Use C++17.
- Do not remove or weaken tests.
- Keep public interfaces stable (do not rename public methods or change signatures unless truly necessary).
- You may refactor internally and add more tests.

## Build & Test

```bash
mkdir -p build
cd build
cmake ..
cmake --build .
ctest --output-on-failure
```
