/opt/homebrew/bin/cmake \
  -DCMAKE_BUILD_TYPE:STRING=Debug \
  -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE \
  -DCMAKE_C_COMPILER:FILEPATH=/opt/homebrew/opt/llvm/bin/clang \
  -DCMAKE_CXX_COMPILER:FILEPATH=/opt/homebrew/opt/llvm/bin/clang++ \
  --no-warn-unused-cli \
  -S /Users/eiri/comp8610/HW4/CodeFramework \
  -B /Users/eiri/comp8610/HW4/CodeFramework/build \
  -G "Unix Makefiles"